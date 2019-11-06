extern crate linq_sys;

use linq_sys::linq_callbacks;
use linq_sys::linq_create;
use std::option::Option;
use std::os::raw;
use std::os::raw::c_char;
use std::os::raw::c_void;

pub enum Request {
    Get(&'static str),
    Post(&'static str, &'static str),
    Delete(&'static str),
}

pub enum Alert {
    Heartbeat(Box<dyn Fn(&LinqHandle, &str)>),
    Alert(Box<dyn Fn(&LinqHandle, linq_sys::E_LINQ_ERROR, &str)>),
    Error(Box<dyn Fn(&LinqHandle, &str)>),
}

enum Socket {
    Server(linq_sys::linq_socket),
    Client(linq_sys::linq_socket),
}

struct LinqContext {
    ctx: *mut linq_sys::linq_s,
    on_heartbeat: Option<Box<dyn Fn(&LinqHandle, &str)>>,
    on_error: Option<Box<dyn Fn(&LinqHandle, linq_sys::E_LINQ_ERROR, &str)>>,
    on_alert: Option<Box<dyn Fn(&LinqHandle, &str)>>,
}

pub struct LinqHandle {
    ctx: Box<LinqContext>,
    sockets: std::collections::HashMap<String, Socket>,
}

impl LinqHandle {
    pub fn new() -> LinqHandle {
        let c_ctx = unsafe { linq_create(&CALLBACKS as *const _, std::ptr::null_mut()) };
        LinqHandle {
            ctx: Box::new(LinqContext {
                ctx: c_ctx,
                on_heartbeat: None,
                on_alert: None,
                on_error: None,
            }),
            sockets: std::collections::HashMap::new(),
        }
    }

    pub fn start(self) -> () {
        while unsafe { linq_sys::sys_running() } {
            self.poll(200);
        }
    }

    pub fn listen(self, port: u32) -> Self {
        self.listen_tcp(port)
    }

    pub fn listen_tcp(self, port: u32) -> Self {
        let mut ep = "tcp://*:".to_owned();
        ep.push_str(port.to_string().as_ref());
        self.listen_ep(ep.as_ref())
    }

    pub fn listen_ep(mut self, s: &str) -> Self {
        let cstr = std::ffi::CString::new(s).unwrap();
        let ep = cstr.as_ptr();
        let socket = unsafe { linq_sys::linq_listen(self.ctx.as_ref().ctx, ep) };
        self.sockets.insert(s.to_string(), Socket::Server(socket));
        self
    }

    pub fn connect(&mut self, s: &str) -> &mut LinqHandle {
        let cstr = std::ffi::CString::new(s).unwrap();
        let ep = cstr.as_ptr();
        let socket = unsafe { linq_sys::linq_connect(self.ctx.as_ref().ctx, ep) };
        self.sockets.insert(s.to_string(), Socket::Client(socket));
        self
    }

    pub fn shutdown(&mut self, s: &str) -> &mut LinqHandle {
        match self.sockets.get(s).unwrap() {
            Socket::Server(s) => unsafe {
                linq_sys::linq_shutdown(self.ctx.as_ref().ctx, *s);
            },
            Socket::Client(s) => unsafe {
                linq_sys::linq_shutdown(self.ctx.as_ref().ctx, *s);
            },
        }
        self
    }

    pub fn poll(&self, ms: u32) -> linq_sys::E_LINQ_ERROR {
        unsafe { linq_sys::linq_poll(self.ctx.as_ref().ctx, ms) }
    }

    pub fn context_set<T>(self, ctx: &mut T) -> Self {
        let data: *mut c_void = ctx as *mut T as *mut c_void;
        unsafe { linq_sys::linq_context_set(self.ctx.as_ref().ctx, data) };
        self
    }

    pub fn on_heartbeat<F>(mut self, f: F) -> Self
    where
        F: 'static + Fn(&LinqHandle, &str),
    {
        self.ctx.as_mut().on_heartbeat = Some(Box::new(f));
        self
    }

    pub fn on_alert<F>(mut self, f: F) -> Self
    where
        F: 'static + Fn(&LinqHandle, &str),
    {
        self.ctx.as_mut().on_alert = Some(Box::new(f));
        self
    }

    pub fn on_error<F>(mut self, f: F) -> Self
    where
        F: 'static + Fn(&LinqHandle, linq_sys::E_LINQ_ERROR, &str),
    {
        self.ctx.as_mut().on_error = Some(Box::new(f));
        self
    }

    pub fn send<F>(&self, r: Request, sid: &str, cb: F) -> &LinqHandle
    where
        F: 'static + Fn(linq_sys::E_LINQ_ERROR, &str),
    {
        let cb: Box<Box<dyn Fn(linq_sys::E_LINQ_ERROR, &str)>> = Box::new(Box::new(cb));
        match r {
            Request::Get(path) => unsafe {
                linq_sys::linq_device_send_get(
                    self.ctx.as_ref().ctx,
                    std::ffi::CString::new(sid).unwrap().as_ptr(),
                    std::ffi::CString::new(path).unwrap().as_ptr(),
                    Some(on_response),
                    Box::into_raw(cb) as *mut _,
                );
            },
            Request::Post(path, data) => unsafe {
                linq_sys::linq_device_send_post(
                    self.ctx.as_ref().ctx,
                    std::ffi::CString::new(sid).unwrap().as_ptr(),
                    std::ffi::CString::new(path).unwrap().as_ptr(),
                    std::ffi::CString::new(data).unwrap().as_ptr(),
                    Some(on_response),
                    Box::into_raw(cb) as *mut _,
                );
            },
            Request::Delete(path) => unsafe {
                linq_sys::linq_device_send_delete(
                    self.ctx.as_ref().ctx,
                    std::ffi::CString::new(sid).unwrap().as_ptr(),
                    std::ffi::CString::new(path).unwrap().as_ptr(),
                    Some(on_response),
                    Box::into_raw(cb) as *mut _,
                );
            },
        }
        self
    }
    pub fn device_count(&self) -> &LinqHandle {
        unsafe {
            linq_sys::linq_device_count(self.ctx.as_ref().ctx);
        }
        self
    }

    pub fn node_count(&self) -> &LinqHandle {
        unsafe {
            linq_sys::linq_nodes_count(self.ctx.as_ref().ctx);
        }
        self
    }
}

impl Drop for LinqHandle {
    fn drop(&mut self) {
        unsafe {
            linq_sys::linq_destroy(&mut self.ctx.as_mut().ctx);
        }
    }
}

unsafe impl Send for LinqHandle {}

extern "C" fn on_error(
    ctx: *mut raw::c_void,
    error: linq_sys::E_LINQ_ERROR,
    _arg3: *const raw::c_char,
    serial: *const raw::c_char,
) -> () {
    let l: &mut LinqHandle = unsafe { &mut *(ctx as *mut LinqHandle) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(serial) };
    l.ctx.as_ref().on_error.as_ref().unwrap()(l, error, cstr.to_str().expect("to_str() fail!"));
}

extern "C" fn on_heartbeat(
    ctx: *mut raw::c_void,
    serial: *const raw::c_char,
    _arg3: *mut *mut linq_sys::device_s,
) -> () {
    let l: &mut LinqHandle = unsafe { &mut *(ctx as *mut LinqHandle) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(serial) };
    l.ctx.as_ref().on_heartbeat.as_ref().unwrap()(l, cstr.to_str().expect("to_str() fail!"));
}

extern "C" fn on_alert(
    ctx: *mut raw::c_void,
    _arg2: *mut linq_sys::linq_alert_s,
    _arg3: *mut linq_sys::linq_email_s,
    device: *mut *mut linq_sys::device_s,
) -> () {
    let l: &mut LinqHandle = unsafe { &mut *(ctx as *mut LinqHandle) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(linq_sys::device_serial(*device)) };
    l.ctx.as_ref().on_alert.as_ref().unwrap()(l, cstr.to_str().expect("to_str() fail!"));
}

extern "C" fn on_response(
    ctx: *mut c_void,
    e: linq_sys::E_LINQ_ERROR,
    json: *const c_char,
    _d: *mut *mut linq_sys::device_s,
) -> () {
    let cb: Box<Box<dyn Fn(linq_sys::E_LINQ_ERROR, &str)>> =
        unsafe { Box::from_raw(ctx as *mut _) };
    let json = unsafe { std::ffi::CStr::from_ptr(json) };
    cb(e, json.to_str().expect("to_str() fail!"));
    drop(cb);
}

static CALLBACKS: linq_callbacks = linq_callbacks {
    err: Some(on_error),
    hb: Some(on_heartbeat),
    alert: Some(on_alert),
};

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn allocators() {
        let _l = LinqHandle::new();
    }
}
