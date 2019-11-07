extern crate linq_sys;

use linq_sys::linq_callbacks;
use linq_sys::linq_create;
use std::os::raw;
use std::os::raw::c_char;
use std::os::raw::c_void;

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
        F: 'static + Fn(&LinqContext, &str),
    {
        let kind = EventKind::Heartbeat(Box::new(f));
        Event { kind }
    }
    pub fn on_alert<F>(f: F) -> Self
    where
        F: 'static + Fn(&LinqContext, &str),
    {
        let kind = EventKind::Alert(Box::new(f));
        Event { kind }
    }

    pub fn on_error<F>(f: F) -> Self
    where
        F: 'static + Fn(&LinqContext, linq_sys::E_LINQ_ERROR, &str),
    {
        let kind = EventKind::Error(Box::new(f));
        Event { kind }
    }
}

pub enum EventKind {
    Heartbeat(Box<dyn Fn(&LinqContext, &str)>),
    Alert(Box<dyn Fn(&LinqContext, &str)>),
    Error(Box<dyn Fn(&LinqContext, linq_sys::E_LINQ_ERROR, &str)>),
}

pub struct LinqContext {
    pub c_ctx: *mut linq_sys::linq_s,
    event_handlers: std::vec::Vec<Event>,
}

impl LinqContext {
    pub fn new() -> LinqContext {
        LinqContext {
            c_ctx: unsafe { linq_create(&CALLBACKS as *const _, std::ptr::null_mut()) },
            event_handlers: std::vec![],
        }
    }

    pub fn register(&mut self, e: Event) -> &mut Self {
        self.event_handlers.push(e);
        self
    }

    pub fn listen(&self, ep: &Endpoint) -> linq_sys::linq_socket {
        unsafe { linq_sys::linq_listen(self.c_ctx, ep.to_c_str().as_ptr()) }
    }

    pub fn connect(&self, ep: &Endpoint) -> linq_sys::linq_socket {
        unsafe { linq_sys::linq_connect(self.c_ctx, ep.to_c_str().as_ptr()) }
    }

    pub fn shutdown(&self, s: linq_sys::linq_socket) -> () {
        unsafe {
            linq_sys::linq_shutdown(self.c_ctx, s);
        }
    }

    pub fn disconnect(&self, s: linq_sys::linq_socket) -> () {
        unsafe {
            linq_sys::linq_shutdown(self.c_ctx, s);
        }
    }

    pub fn poll(&self, ms: u32) -> linq_sys::E_LINQ_ERROR {
        unsafe { linq_sys::linq_poll(self.c_ctx, ms) }
    }

    pub fn send<F>(&self, r: Request, sid: &str, cb: F) -> &LinqContext
    where
        F: 'static + Fn(linq_sys::E_LINQ_ERROR, &str),
    {
        use std::ffi::CString;
        let cb: Box<Box<dyn Fn(linq_sys::E_LINQ_ERROR, &str)>> = Box::new(Box::new(cb));
        match r {
            Request::Get(path) => unsafe {
                linq_sys::linq_device_send_get(
                    self.c_ctx,
                    CString::new(sid).unwrap().as_ptr(),
                    CString::new(path).unwrap().as_ptr(),
                    Some(on_response),
                    Box::into_raw(cb) as *mut _,
                );
            },
            Request::Post(path, data) => unsafe {
                linq_sys::linq_device_send_post(
                    self.c_ctx,
                    CString::new(sid).unwrap().as_ptr(),
                    CString::new(path).unwrap().as_ptr(),
                    CString::new(data).unwrap().as_ptr(),
                    Some(on_response),
                    Box::into_raw(cb) as *mut _,
                );
            },
            Request::Delete(path) => unsafe {
                linq_sys::linq_device_send_delete(
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
        unsafe { linq_sys::linq_device_count(self.c_ctx) }
    }

    pub fn node_count(&self) -> &LinqContext {
        unsafe {
            linq_sys::linq_nodes_count(self.c_ctx);
        }
        self
    }
}

impl Drop for LinqContext {
    fn drop(&mut self) {
        unsafe {
            linq_sys::linq_destroy(&mut self.c_ctx);
        }
    }
}

unsafe impl Send for LinqContext {}
unsafe impl Sync for LinqContext {}

extern "C" fn on_error(
    linq: *mut raw::c_void,
    error: linq_sys::E_LINQ_ERROR,
    _arg3: *const raw::c_char,
    serial: *const raw::c_char,
) -> () {
    let l: &mut LinqContext = unsafe { &mut *(linq as *mut LinqContext) };
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
    _arg3: *mut *mut linq_sys::device_s,
) -> () {
    let l: &mut LinqContext = unsafe { &mut *(linq as *mut LinqContext) };
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
    _arg2: *mut linq_sys::linq_alert_s,
    _arg3: *mut linq_sys::linq_email_s,
    device: *mut *mut linq_sys::device_s,
) -> () {
    let l: &mut LinqContext = unsafe { &mut *(linq as *mut LinqContext) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(linq_sys::device_serial(*device)) };
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
    e: linq_sys::E_LINQ_ERROR,
    json: *const c_char,
    _d: *mut *mut linq_sys::device_s,
) -> () {
    let cb: Box<Box<dyn Fn(linq_sys::E_LINQ_ERROR, &str)>> = unsafe { Box::from_raw(cb as *mut _) };
    let json = unsafe { std::ffi::CStr::from_ptr(json) };
    cb(e, json.to_str().expect("to_str() fail!"));
    drop(cb);
}

static CALLBACKS: linq_callbacks = linq_callbacks {
    err: Some(on_error),
    hb: Some(on_heartbeat),
    alert: Some(on_alert),
};
