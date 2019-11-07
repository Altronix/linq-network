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
    Heartbeat(Box<dyn Fn(&LinqContext, &str)>),
    Alert(Box<dyn Fn(&LinqContext, linq_sys::E_LINQ_ERROR, &str)>),
    Error(Box<dyn Fn(&LinqContext, &str)>),
}

pub struct LinqContext {
    pub c_ctx: *mut linq_sys::linq_s,
    pub on_heartbeat: Option<Box<dyn Fn(&LinqContext, &str)>>,
    pub on_error: Option<Box<dyn Fn(&LinqContext, linq_sys::E_LINQ_ERROR, &str)>>,
    pub on_alert: Option<Box<dyn Fn(&LinqContext, &str)>>,
}

impl LinqContext {
    pub fn new() -> LinqContext {
        LinqContext {
            c_ctx: unsafe { linq_create(&CALLBACKS as *const _, std::ptr::null_mut()) },
            on_heartbeat: None,
            on_alert: None,
            on_error: None,
        }
    }

    pub fn listen(&self, s: &str) -> linq_sys::linq_socket {
        let cstr = std::ffi::CString::new(s).unwrap();
        let ep = cstr.as_ptr();
        unsafe { linq_sys::linq_listen(self.c_ctx, ep) }
    }

    pub fn connect(&self, s: &str) -> linq_sys::linq_socket {
        let cstr = std::ffi::CString::new(s).unwrap();
        let ep = cstr.as_ptr();
        unsafe { linq_sys::linq_connect(self.c_ctx, ep) }
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
    pub fn device_count(&self) -> &LinqContext {
        unsafe {
            linq_sys::linq_device_count(self.c_ctx);
        }
        self
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

extern "C" fn on_error(
    linq: *mut raw::c_void,
    error: linq_sys::E_LINQ_ERROR,
    _arg3: *const raw::c_char,
    serial: *const raw::c_char,
) -> () {
    let l: &mut LinqContext = unsafe { &mut *(linq as *mut LinqContext) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(serial) };
    l.on_error.as_ref().unwrap()(l, error, cstr.to_str().expect("to_str() fail!"));
}

extern "C" fn on_heartbeat(
    linq: *mut raw::c_void,
    serial: *const raw::c_char,
    _arg3: *mut *mut linq_sys::device_s,
) -> () {
    let l: &mut LinqContext = unsafe { &mut *(linq as *mut LinqContext) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(serial) };
    l.on_heartbeat.as_ref().unwrap()(l, cstr.to_str().expect("to_str() fail!"));
}

extern "C" fn on_alert(
    linq: *mut raw::c_void,
    _arg2: *mut linq_sys::linq_alert_s,
    _arg3: *mut linq_sys::linq_email_s,
    device: *mut *mut linq_sys::device_s,
) -> () {
    let l: &mut LinqContext = unsafe { &mut *(linq as *mut LinqContext) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(linq_sys::device_serial(*device)) };
    l.on_alert.as_ref().unwrap()(l, cstr.to_str().expect("to_str() fail!"));
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
