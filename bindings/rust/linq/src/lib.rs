extern crate futures;
extern crate linq_sys;

use linq_sys::*;
use std::collections::HashMap;
use std::ffi::CStr;
use std::ffi::CString;
use std::future::Future;
use std::option::Option;
use std::os::raw;
use std::os::raw::c_char;
use std::os::raw::c_void;
use std::pin::Pin;
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

pub enum Request {
    Get(&'static str),
    Post(&'static str, &'static str),
    Delete(&'static str),
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
    type Output = Result<String, E_LINQ_ERROR>;
    fn poll(self: Pin<&mut Self>, ctx: &mut Context<'_>) -> Poll<Self::Output> {
        let mut r = self.response.lock().unwrap();
        // TODO Refactor this. We are copying the JSON twice and we shouldn't have to
        if r.error.is_some() && r.json.is_some() {
            let err = r.error.unwrap();
            let json = r.json.as_ref().unwrap().to_string();
            match r.error.unwrap() {
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

pub struct Event {
    kind: EventKind,
}

impl Event {
    pub fn on_heartbeat<F>(f: F) -> Event
    where
        F: 'static + Fn(&Linq, &str),
    {
        let kind = EventKind::Heartbeat(Box::new(f));
        Event { kind }
    }
    pub fn on_alert<F>(f: F) -> Self
    where
        F: 'static + Fn(&Linq, &str),
    {
        let kind = EventKind::Alert(Box::new(f));
        Event { kind }
    }

    pub fn on_error<F>(f: F) -> Self
    where
        F: 'static + Fn(&Linq, E_LINQ_ERROR, &str),
    {
        let kind = EventKind::Error(Box::new(f));
        Event { kind }
    }
}

pub enum EventKind {
    Heartbeat(Box<dyn Fn(&Linq, &str)>),
    Alert(Box<dyn Fn(&Linq, &str)>),
    Error(Box<dyn Fn(&Linq, E_LINQ_ERROR, &str)>),
}

pub struct Linq {
    c_ctx: *mut linq_s,
    event_handlers: std::vec::Vec<Event>,
    sockets: HashMap<String, Socket>,
}

impl Linq {
    pub fn new() -> Linq {
        Linq {
            c_ctx: unsafe {
                linq_create(&CALLBACKS as *const _, std::ptr::null_mut())
            },
            event_handlers: std::vec![],
            sockets: HashMap::new(),
        }
    }

    pub fn register(mut self, e: Event) -> Self {
        self.event_handlers.push(e);
        self
    }

    // TODO - look into std::pin or refactor so that Linq is managed in a box
    // and we return a wrapper around Linq
    pub fn pin(mut self) -> Self {
        let ctx: *mut c_void = &mut self as *mut Linq as *mut _;
        unsafe { linq_context_set(self.c_ctx, ctx) };
        self
    }

    pub fn listen(mut self, ep: Endpoint) -> Self {
        let s = unsafe { linq_listen(self.c_ctx, ep.to_c_str().as_ptr()) };
        self.sockets.insert(ep.to_str(), Socket::Server(s));
        self
    }

    pub fn connect(mut self, ep: Endpoint) -> Self {
        let s = unsafe { linq_connect(self.c_ctx, ep.to_c_str().as_ptr()) };
        self.sockets.insert(ep.to_str(), Socket::Client(s));
        self
    }

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

    pub fn poll(&self, ms: u32) -> E_LINQ_ERROR {
        unsafe { linq_poll(self.c_ctx, ms) }
    }

    pub fn send(&self, r: Request, sid: &str) -> ResponseFuture {
        let response = ResponseFuture::new();
        let clone = Arc::clone(&response.response);
        self.send_cb(r, sid, move |e, json| {
            let mut r = clone.lock().unwrap();
            r.resolve(e, json);
        });
        response
    }

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
        unsafe {
            linq_destroy(&mut self.c_ctx);
        }
    }
}

unsafe impl Send for Linq {}
unsafe impl Sync for Linq {}

extern "C" fn on_error(
    linq: *mut raw::c_void,
    error: E_LINQ_ERROR,
    _arg3: *const raw::c_char,
    serial: *const raw::c_char,
) -> () {
    let l: &mut Linq = unsafe { &mut *(linq as *mut Linq) };
    let cstr = unsafe { CStr::from_ptr(serial) };
    let cstr = cstr.to_str().expect("to_str() fail!");

    for e in l.event_handlers.iter() {
        match &e.kind {
            EventKind::Error(f) => f(l, error, cstr),
            _ => (),
        }
    }
}

extern "C" fn on_heartbeat(
    linq: *mut raw::c_void,
    serial: *const raw::c_char,
    _arg3: *mut *mut device_s,
) -> () {
    let l: &mut Linq = unsafe { &mut *(linq as *mut Linq) };
    let cstr = unsafe { CStr::from_ptr(serial) };
    let cstr = cstr.to_str().expect("to_str() fail!");

    for e in l.event_handlers.iter() {
        match &e.kind {
            EventKind::Heartbeat(f) => f(l, cstr),
            _ => (),
        }
    }
}

extern "C" fn on_alert(
    linq: *mut raw::c_void,
    _arg2: *mut linq_alert_s,
    _arg3: *mut linq_email_s,
    device: *mut *mut device_s,
) -> () {
    let l: &mut Linq = unsafe { &mut *(linq as *mut Linq) };
    let cstr = unsafe { CStr::from_ptr(device_serial(*device)) };
    let cstr = cstr.to_str().expect("to_str() fail!");
    for e in l.event_handlers.iter() {
        match &e.kind {
            EventKind::Alert(f) => f(l, cstr),
            _ => (),
        }
    }
}

static CALLBACKS: linq_callbacks = linq_callbacks {
    err: Some(on_error),
    hb: Some(on_heartbeat),
    alert: Some(on_alert),
};
