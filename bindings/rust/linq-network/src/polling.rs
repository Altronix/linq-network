// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

extern crate futures;
extern crate linq_network_sys;

use crate::error::{NetworkError, NetworkErrorKind};
use crate::event;
use crate::simple_future;

use linq_network_sys::*;
use simple_future::SimpleFuture;
use std::collections::HashMap;
use std::ffi::CStr;
use std::ffi::CString;
use std::os::raw::c_char;
use std::os::raw::c_void;
use std::ptr::null_mut;
use std::sync::{Arc, Mutex};

pub use linq_network_sys::netw_socket;

pub enum Request<'a> {
    Get(&'a str),
    Post(&'a str, &'a str),
    Delete(&'a str),
}

#[derive(Clone)]
pub struct Response {
    pub result: Result<String, NetworkError>,
}

pub enum Endpoint {
    Tcp(u32),
    Ipc(&'static str),
}

impl Endpoint {
    pub fn to_str(&self) -> String {
        match self {
            Endpoint::Tcp(p) => {
                let mut ep = "tcp://*:".to_owned();
                ep.push_str(p.to_string().as_ref());
                ep
            }
            Endpoint::Ipc(s) => {
                let mut ep = "ipc://".to_owned();
                ep.push_str(s);
                ep
            }
        }
    }

    pub fn to_c_str(&self) -> CString {
        CString::new(self.to_str()).unwrap()
    }
}

pub struct Context {
    c_ctx: *mut netw_s,
    events: Arc<Mutex<event::EventStreamState>>,
    c_events: *mut c_void,
}

impl Context {
    // Create context for linq library
    // Create callback context for linq events (Event Queue)
    // Initialize Linq wrapper
    pub fn new() -> Self {
        let events = Arc::new(Mutex::new(event::EventStreamState::new()));
        let clone = Arc::clone(&events);
        let c_ctx =
            unsafe { netw_create(&event::CALLBACKS as *const _, null_mut()) };
        let c_events = Arc::into_raw(clone) as *mut c_void;
        unsafe { netw_context_set(c_ctx, c_events) };
        Context {
            c_ctx,
            events,
            c_events,
        }
    }

    // Listen for incoming linq nodes or device nodes
    // TODO - return socket handle and remove socket hashmap
    pub fn listen(&self, ep: Endpoint) -> netw_socket {
        let s = unsafe { netw_listen(self.c_ctx, ep.to_c_str().as_ptr()) };
        s
    }

    pub fn events(&self) -> event::EventStream {
        event::EventStream::new(&self.events)
    }

    // Connect to another linq node
    pub fn connect(&self, ep: Endpoint) -> netw_socket {
        let ep = ep.to_c_str().as_ptr();
        let s = unsafe { netw_connect(self.c_ctx, ep) };
        s
    }

    // Opposite of listen or connect. You do not need to close on clean up.
    pub fn close(&self, s: &netw_socket) -> &Self {
        unsafe { netw_close(self.c_ctx, *s) };
        self
    }

    // Run background tasks (handle Context IO)
    pub fn poll(&self, ms: i32) -> E_LINQ_ERROR {
        unsafe { netw_poll(self.c_ctx, ms) }
    }

    // Same as poll accept will block on socket read
    pub fn poll_blocking(&self) -> E_LINQ_ERROR {
        self.poll(-1)
    }

    pub fn get(&self, path: &str, sid: &str) -> SimpleFuture<Response> {
        self.send(Request::Get(path), sid)
    }

    pub fn post(&self, p: &str, d: &str, id: &str) -> SimpleFuture<Response> {
        self.send(Request::Post(p, d), id)
    }

    pub fn delete(&self, p: &str, id: &str) -> SimpleFuture<Response> {
        self.send(Request::Delete(p), id)
    }

    // C library accepts callback on response. We turn response into future
    pub fn send(&self, r: Request, sid: &str) -> SimpleFuture<Response> {
        let future = SimpleFuture::new();
        let clone = Arc::clone(&future.state);
        self.send_cb(r, sid, move |error, json| {
            let mut r = clone.lock().unwrap();
            let error = NetworkError::from(error);
            r.resolve(Response {
                result: if error.kind == NetworkErrorKind::Ok {
                    Ok(json.to_string())
                } else {
                    Err(error)
                },
            });
        });
        future
    }

    // We call netw_send... which accepts a callback for response.
    // We store response as heap enclosure (fn on_response)
    pub fn send_cb<F>(&self, r: Request, sid: &str, cb: F) -> ()
    where
        F: 'static + FnOnce(E_LINQ_ERROR, &str),
    {
        extern "C" fn on_response(
            cb: *mut c_void,
            serial: *const c_char,
            e: E_LINQ_ERROR,
            json: *const c_char,
        ) -> () {
            let cb: Box<Box<dyn FnOnce(E_LINQ_ERROR, &str)>> =
                unsafe { Box::from_raw(cb as *mut _) };
            let json = unsafe { CStr::from_ptr(json) };
            cb(e, json.to_str().expect("to_str() fail!"));
        }
        // Why two box's? `\_(",)_/`
        let cb: Box<Box<dyn FnOnce(E_LINQ_ERROR, &str)>> =
            Box::new(Box::new(cb));
        match r {
            Request::Get(path) => unsafe {
                netw_send(
                    self.c_ctx,
                    CString::new(sid).unwrap().as_ptr(),
                    CString::new("GET").unwrap().as_ptr(),
                    CString::new(path).unwrap().as_ptr(),
                    path.len() as u32,
                    null_mut(),
                    0,
                    Some(on_response),
                    Box::into_raw(cb) as *mut _,
                );
            },
            Request::Post(path, data) => unsafe {
                netw_send(
                    self.c_ctx,
                    CString::new(sid).unwrap().as_ptr(),
                    CString::new("POST").unwrap().as_ptr(),
                    CString::new(path).unwrap().as_ptr(),
                    path.len() as u32,
                    CString::new(data).unwrap().as_ptr(),
                    data.len() as u32,
                    Some(on_response),
                    Box::into_raw(cb) as *mut _,
                );
            },
            Request::Delete(path) => unsafe {
                netw_send(
                    self.c_ctx,
                    CString::new(sid).unwrap().as_ptr(),
                    CString::new("DELETE").unwrap().as_ptr(),
                    CString::new(path).unwrap().as_ptr(),
                    path.len() as u32,
                    null_mut(),
                    0,
                    Some(on_response),
                    Box::into_raw(cb) as *mut _,
                );
            },
        };
    }

    pub fn device_count(&self) -> u32 {
        unsafe { netw_device_count(self.c_ctx) }
    }

    pub fn devices(&self) -> HashMap<String, String> {
        let mut map = HashMap::new();
        let ctx: *mut c_void =
            &mut map as *mut HashMap<String, String> as *mut _;
        extern "C" fn foreach(
            ctx: *mut c_void,
            sid: *const c_char,
            pid: *const c_char,
        ) {
            let map = unsafe { &mut *(ctx as *mut HashMap<String, String>) };
            let sid = unsafe { CStr::from_ptr(sid).to_str().unwrap() };
            let pid = unsafe { CStr::from_ptr(pid).to_str().unwrap() };
            map.insert(sid.to_owned(), pid.to_owned());
        }
        unsafe { netw_devices_foreach(self.c_ctx, Some(foreach), ctx) };
        map
    }

    pub fn node_count(&self) -> u32 {
        unsafe { netw_node_count(self.c_ctx) }
    }

    pub fn running(&self) -> bool {
        unsafe { netw_running(self.c_ctx) }
    }
}

impl Drop for Context {
    fn drop(&mut self) {
        // Destroy c context and memory we passed to c library
        // Summon Arc pointer from raw so that it will free on drop
        use event::EventsLock;
        unsafe { netw_destroy(&mut self.c_ctx) };
        let _e: EventsLock = unsafe { Arc::from_raw(self.c_events as *mut _) };
    }
}

unsafe impl Send for Context {}
// unsafe impl Sync for Context {} // TODO evaluate if this breaks anything
