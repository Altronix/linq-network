extern crate futures;
extern crate linq_sys;

use futures::stream::Stream;
use linq_sys::*;
use std::collections::HashMap;
use std::collections::VecDeque;
use std::ffi::CStr;
use std::ffi::CString;
use std::future::Future;
use std::option::Option;
use std::os::raw;
use std::os::raw::c_char;
use std::os::raw::c_void;
use std::pin::Pin;
use std::ptr::null_mut;
use std::result::Result;
use std::sync::{Arc, Mutex};
use std::task::{Context, Poll, Waker};

pub fn running() -> bool {
    unsafe { sys_running() }
}

pub enum Socket {
    Server(linq_socket),
    Client(linq_socket),
}

pub enum Request<'a> {
    Get(&'a str),
    Post(&'a str, &'a str),
    Delete(&'a str),
}

pub struct Response {
    error: Option<E_LINQ_ERROR>,
    json: Option<String>,
    waker: Option<Waker>,
}

impl Response {
    fn resolve(&mut self, e: E_LINQ_ERROR, json: &str) {
        self.error = Some(e);
        self.json = Some(json.to_string());
        if let Some(waker) = self.waker.take() {
            waker.wake();
        }
    }
}

pub struct ResponseFuture {
    response: Arc<Mutex<Response>>,
}

impl ResponseFuture {
    fn new() -> Self {
        ResponseFuture {
            response: Arc::new(Mutex::new(Response {
                error: None,
                json: None,
                waker: None,
            })),
        }
    }
}

impl Future for ResponseFuture {
    //   Output = Result<&'a str, E_LINQ_ERROR> TODO - figure out how
    type Output = Result<String, E_LINQ_ERROR>;
    fn poll(self: Pin<&mut Self>, ctx: &mut Context<'_>) -> Poll<Self::Output> {
        let mut r = self.response.lock().unwrap();
        if r.error.is_some() && r.json.is_some() {
            let err = r.error.unwrap();
            let json = r.json.as_ref().unwrap().to_string();
            match err {
                E_LINQ_ERROR_LINQ_ERROR_OK => Poll::Ready(Ok(json)),
                _ => Poll::Ready(Err(err)),
            }
        } else {
            r.waker = Some(ctx.waker().clone());
            Poll::Pending
        }
    }
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

type EventsLock = Arc<Mutex<EventStreamState>>;
pub enum Event {
    Heartbeat(String),
    Alert(String),
    Error(E_LINQ_ERROR, String),
}

pub struct EventStreamState {
    events: VecDeque<Event>,
    waker: Option<Waker>,
}

impl EventStreamState {
    pub fn new() -> Self {
        EventStreamState {
            events: VecDeque::new(),
            waker: None,
        }
    }
}

pub struct EventStream {
    state: Arc<Mutex<EventStreamState>>,
}

impl EventStream {
    pub fn new(events: &Arc<Mutex<EventStreamState>>) -> Self {
        EventStream {
            state: Arc::clone(events),
        }
    }
}

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

pub struct Linq {
    c_ctx: *mut linq_s,
    events: Arc<Mutex<EventStreamState>>,
    c_events: *mut c_void,
    sockets: HashMap<String, Socket>,
}

impl Linq {
    // Create context for linq library
    // Create callback context for linq events (Event Queue)
    // Initialize Linq wrapper
    pub fn new() -> Self {
        let events = Arc::new(Mutex::new(EventStreamState::new()));
        let clone = Arc::clone(&events);
        let c_ctx = unsafe { linq_create(&CALLBACKS as *const _, null_mut()) };
        let c_events = Arc::into_raw(clone) as *mut c_void;
        unsafe { linq_context_set(c_ctx, c_events) };
        Linq {
            c_ctx,
            events,
            c_events,
            sockets: HashMap::new(),
        }
    }

    pub fn stream(ep: Endpoint) -> (Self, EventStream) {
        let linq = Linq::new().listen(ep);
        let es = EventStream::new(&linq.events);
        (linq, es)
    }

    // Listen for incoming linq nodes or device nodes
    pub fn listen(mut self, ep: Endpoint) -> Self {
        let s = unsafe { linq_listen(self.c_ctx, ep.to_c_str().as_ptr()) };
        self.sockets.insert(ep.to_str(), Socket::Server(s));
        self
    }

    // Connect to another linq node
    pub fn connect(mut self, ep: Endpoint) -> Self {
        let s = unsafe { linq_connect(self.c_ctx, ep.to_c_str().as_ptr()) };
        self.sockets.insert(ep.to_str(), Socket::Client(s));
        self
    }

    // Opposite of listen or connect. You do not need to shutdown on clean up.
    // You only need to shutdown if you want to close connections
    pub fn shutdown(self, s: &str) -> Self {
        match self.sockets.get(s).unwrap() {
            Socket::Server(s) => unsafe {
                linq_shutdown(self.c_ctx, *s);
            },
            Socket::Client(s) => unsafe {
                linq_disconnect(self.c_ctx, *s);
            },
        };
        self
    }

    // Run background tasks (handle Linq IO)
    pub fn poll(&self, ms: u32) -> E_LINQ_ERROR {
        unsafe { linq_poll(self.c_ctx, ms) }
    }

    // Linq C library accepts callback on response. We turn response into future
    pub fn send(&self, r: Request, sid: &str) -> ResponseFuture {
        let response = ResponseFuture::new();
        let clone = Arc::clone(&response.response);
        self.send_cb(r, sid, move |e, json| {
            let mut r = clone.lock().unwrap();
            r.resolve(e, json);
        });
        response
    }

    // We call linq_device_send... which accepts a callback for response. We
    // store response as heap enclosure (fn on_response)
    pub fn send_cb<F>(&self, r: Request, sid: &str, cb: F) -> ()
    where
        F: 'static + FnOnce(E_LINQ_ERROR, &str),
    {
        extern "C" fn on_response(
            cb: *mut c_void,
            e: E_LINQ_ERROR,
            json: *const c_char,
            _d: *mut *mut device_s,
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
                linq_device_send_get(
                    self.c_ctx,
                    CString::new(sid).unwrap().as_ptr(),
                    CString::new(path).unwrap().as_ptr(),
                    Some(on_response),
                    Box::into_raw(cb) as *mut _,
                );
            },
            Request::Post(path, data) => unsafe {
                linq_device_send_post(
                    self.c_ctx,
                    CString::new(sid).unwrap().as_ptr(),
                    CString::new(path).unwrap().as_ptr(),
                    CString::new(data).unwrap().as_ptr(),
                    Some(on_response),
                    Box::into_raw(cb) as *mut _,
                );
            },
            Request::Delete(path) => unsafe {
                linq_device_send_delete(
                    self.c_ctx,
                    CString::new(sid).unwrap().as_ptr(),
                    CString::new(path).unwrap().as_ptr(),
                    Some(on_response),
                    Box::into_raw(cb) as *mut _,
                );
            },
        };
    }

    pub fn device_count(&self) -> u32 {
        unsafe { linq_device_count(self.c_ctx) }
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
        unsafe { linq_devices_foreach(self.c_ctx, Some(foreach), ctx) };
        map
    }

    pub fn node_count(&self) -> &Linq {
        unsafe {
            linq_nodes_count(self.c_ctx);
        }
        self
    }
}

impl Drop for Linq {
    fn drop(&mut self) {
        // Destroy c context and memory we passed to c library
        // Summon Arc pointer from raw so that it will free on drop
        unsafe { linq_destroy(&mut self.c_ctx) };
        let _e: EventsLock = unsafe { Arc::from_raw(self.c_events as *mut _) };
    }
}

unsafe impl Send for Linq {}
unsafe impl Sync for Linq {}

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
    // (We free the actual Arc ptr in Linq destructor)
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
    _arg2: *mut linq_alert_s,
    _arg3: *mut linq_email_s,
    device: *mut *mut device_s,
) -> () {
    let cstr = unsafe { CStr::from_ptr(device_serial(*device)) };
    let cstr = cstr.to_str().expect("to_str() fail!");
    load_event(ctx, Event::Alert(cstr.to_string()));
}

static CALLBACKS: linq_callbacks = linq_callbacks {
    err: Some(on_error),
    hb: Some(on_heartbeat),
    alert: Some(on_alert),
};
