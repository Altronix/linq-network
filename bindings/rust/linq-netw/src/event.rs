// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

extern crate linq_netw_sys;

use futures::stream::Stream;
use linq_netw_sys::*;
use std::collections::VecDeque;
use std::ffi::CStr;
use std::os::raw;
use std::os::raw::c_void;
use std::pin::Pin;
use std::sync::{Arc, Mutex};
use std::task::{Context, Poll, Waker};

// All events from linq_netw match this signature
#[derive(PartialEq)]
pub enum Event {
    Heartbeat(String),
    Alert(String),
    Error(E_LINQ_ERROR, String),
    Ctrlc,
}

// Shared state for event stream
pub type EventsLock = Arc<Mutex<EventStreamState>>;

// Queue of events and waker for stream
pub struct EventStreamState {
    events: VecDeque<Event>,
    waker: Option<Waker>,
}

// Helper to create a stream state
impl EventStreamState {
    pub fn new() -> Self {
        EventStreamState {
            events: VecDeque::new(),
            waker: None,
        }
    }
}

// The event stream handle (contains pointer to shared state)
pub struct EventStream {
    state: Arc<Mutex<EventStreamState>>,
}

// Helper to create an event stream
impl EventStream {
    pub fn new(events: &Arc<Mutex<EventStreamState>>) -> Self {
        EventStream {
            state: Arc::clone(events),
        }
    }
}

// Must implement the Stream trait for our stream
impl Stream for EventStream {
    type Item = Event;
    fn poll_next(
        self: Pin<&mut Self>,
        cx: &mut Context,
    ) -> Poll<Option<Self::Item>> {
        let mut state = self.state.lock().unwrap();
        let e = state.events.pop_front();
        match e {
            Some(e) => Poll::Ready(Some(e)),
            _ => {
                state.waker = Some(cx.waker().clone());
                Poll::Pending
            }
        }
    }
}

// Populate the c_context ptr (event queue) with an additional event
fn load_event(ctx: *mut c_void, event: Event) {
    let state: EventsLock = unsafe { Arc::from_raw(ctx as *mut _) };
    {
        // Need to unlock() mutex before we forget mem
        let mut state = state.lock().unwrap();
        state.events.push_back(event);
        if let Some(waker) = state.waker.take() {
            waker.wake();
        }
    }
    // This was cast from c ptr. If we call destructor we'll get double free's
    // (We free the actual Arc ptr in Handle destructor)
    core::mem::forget(state);
}

// Callback from c library when error
extern "C" fn on_error(
    ctx: *mut raw::c_void,
    error: E_LINQ_ERROR,
    _arg3: *const raw::c_char,
    serial: *const raw::c_char,
) -> () {
    let cstr = unsafe { CStr::from_ptr(serial) };
    let cstr = cstr.to_str().expect("to_str() fail!");
    load_event(ctx, Event::Error(error, cstr.to_string()));
}

// Callback from c library when heartbeat
extern "C" fn on_heartbeat(
    ctx: *mut raw::c_void,
    serial: *const raw::c_char,
    _arg3: *mut *mut device_s,
) -> () {
    let cstr = unsafe { CStr::from_ptr(serial) };
    let cstr = cstr.to_str().expect("to_str() fail!");
    load_event(ctx, Event::Heartbeat(cstr.to_string()));
}

// Calback from c library when alert
extern "C" fn on_alert(
    ctx: *mut raw::c_void,
    _arg2: *mut linq_netw_alert_s,
    _arg3: *mut linq_netw_email_s,
    device: *mut *mut device_s,
) -> () {
    let cstr = unsafe { CStr::from_ptr(device_serial(*device)) };
    let cstr = cstr.to_str().expect("to_str() fail!");
    load_event(ctx, Event::Alert(cstr.to_string()));
}

// Calback from c library when control-c detected
extern "C" fn on_ctrlc(ctx: *mut raw::c_void) -> () {
    load_event(ctx, Event::Ctrlc);
}

pub static CALLBACKS: linq_netw_callbacks = linq_netw_callbacks {
    err: Some(on_error),
    hb: Some(on_heartbeat),
    alert: Some(on_alert),
    ctrlc: Some(on_ctrlc),
};
