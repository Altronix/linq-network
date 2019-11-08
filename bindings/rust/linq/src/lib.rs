extern crate linq_sys;

use linq_sys::*;
use std::collections::HashMap;
use std::os::raw;
use std::os::raw::c_char;
use std::os::raw::c_void;

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

    pub fn to_c_str(&self) -> std::ffi::CString {
        std::ffi::CString::new(self.to_str()).unwrap()
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
    pub c_ctx: *mut linq_s,
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
        unsafe {
            linq_context_set(self.c_ctx, &mut self as *mut Linq as *mut _)
        };
        self.event_handlers.push(e);
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

    pub fn send<F>(&self, r: Request, sid: &str, cb: F) -> &Linq
    where
        F: 'static + Fn(E_LINQ_ERROR, &str),
    {
        use std::ffi::CString;
        let cb: Box<Box<dyn Fn(E_LINQ_ERROR, &str)>> = Box::new(Box::new(cb));
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
        }
        self
    }
    pub fn device_count(&self) -> u32 {
        unsafe { linq_device_count(self.c_ctx) }
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
    let cstr = unsafe { std::ffi::CStr::from_ptr(serial) };
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
    let cstr = unsafe { std::ffi::CStr::from_ptr(serial) };
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
    let cstr = unsafe { std::ffi::CStr::from_ptr(device_serial(*device)) };
    let cstr = cstr.to_str().expect("to_str() fail!");
    for e in l.event_handlers.iter() {
        match &e.kind {
            EventKind::Alert(f) => f(l, cstr),
            _ => (),
        }
    }
}

extern "C" fn on_response(
    cb: *mut c_void,
    e: E_LINQ_ERROR,
    json: *const c_char,
    _d: *mut *mut device_s,
) -> () {
    let cb: Box<Box<dyn Fn(E_LINQ_ERROR, &str)>> =
        unsafe { Box::from_raw(cb as *mut _) };
    let json = unsafe { std::ffi::CStr::from_ptr(json) };
    cb(e, json.to_str().expect("to_str() fail!"));
    drop(cb);
}

static CALLBACKS: linq_callbacks = linq_callbacks {
    err: Some(on_error),
    hb: Some(on_heartbeat),
    alert: Some(on_alert),
};
