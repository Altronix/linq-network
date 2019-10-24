extern crate linq_sys;

use linq_sys::linq_callbacks;
use linq_sys::linq_create;
use std::os::raw;

pub type HeartbeatFunction = fn(&mut Linq, sid: &str);
pub type ErrorFunction = fn(&mut Linq, linq_sys::E_LINQ_ERROR, sid: &str);
pub type AlertFunction = fn(&mut Linq, sid: &str);

pub struct Linq {
    ctx: *mut linq_sys::linq_s,
    on_heartbeat: std::option::Option<HeartbeatFunction>,
    on_error: std::option::Option<ErrorFunction>,
    on_alert: std::option::Option<AlertFunction>,
}

impl Linq {
    pub fn new() -> Linq {
        use std::os::raw::c_void;
        unsafe {
            let ctx = linq_create(&CALLBACKS as *const _, std::ptr::null_mut());
            let mut l = Linq {
                ctx,
                on_heartbeat: None,
                on_alert: None,
                on_error: None,
            };
            let data: *mut c_void = &mut l as *mut Linq as *mut c_void;
            linq_sys::linq_context_set(l.ctx, data);
            l
        }
    }

    pub fn listen(&self, s: &str) -> linq_sys::linq_socket {
        let cstr = std::ffi::CString::new(s).unwrap();
        let ep = cstr.as_ptr();
        unsafe {
            let socket = linq_sys::linq_listen(self.ctx, ep);
            socket
        }
    }

    pub fn connect(&self, s: &str) -> linq_sys::linq_socket {
        let cstr = std::ffi::CString::new(s).unwrap();
        let ep = cstr.as_ptr();
        unsafe {
            let socket = linq_sys::linq_connect(self.ctx, ep);
            socket
        }
    }

    pub fn shutdown(&self, socket: linq_sys::linq_socket) -> linq_sys::E_LINQ_ERROR {
        unsafe {
            let e = linq_sys::linq_shutdown(self.ctx, socket);
            e
        }
    }

    pub fn disconnect(&self, socket: linq_sys::linq_socket) -> linq_sys::E_LINQ_ERROR {
        unsafe {
            let e = linq_sys::linq_disconnect(self.ctx, socket);
            e
        }
    }

    pub fn poll(&self) -> linq_sys::E_LINQ_ERROR {
        unsafe {
            let e = linq_sys::linq_poll(self.ctx);
            e
        }
    }

    // TODO
    // pub fn device(&self, s: &str) -> std::option::Option<Device> {}

    pub fn device_count(&self) -> &Linq {
        unsafe {
            linq_sys::linq_device_count(self.ctx);
        }
        self
    }

    pub fn node_count(&self) -> &Linq {
        unsafe {
            linq_sys::linq_nodes_count(self.ctx);
        }
        self
    }
}

impl Drop for Linq {
    fn drop(&mut self) {
        unsafe {
            linq_sys::linq_destroy(&mut self.ctx);
        }
    }
}

extern "C" fn on_error(
    ctx: *mut raw::c_void,
    error: linq_sys::E_LINQ_ERROR,
    _arg3: *const raw::c_char,
    serial: *const raw::c_char,
) -> () {
    let l: &mut Linq = unsafe { &mut *(ctx as *mut Linq) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(serial) };
    l.on_error.unwrap()(l, error, cstr.to_str().expect("to_str() fail!"));
}

extern "C" fn on_heartbeat(
    ctx: *mut raw::c_void,
    serial: *const raw::c_char,
    _arg3: *mut *mut linq_sys::device_s,
) -> () {
    let l: &mut Linq = unsafe { &mut *(ctx as *mut Linq) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(serial) };
    l.on_heartbeat.unwrap()(l, cstr.to_str().expect("to_str() fail!"));
}

extern "C" fn on_alert(
    ctx: *mut raw::c_void,
    _arg2: *mut linq_sys::linq_alert_s,
    _arg3: *mut linq_sys::linq_email_s,
    device: *mut *mut linq_sys::device_s,
) -> () {
    let l: &mut Linq = unsafe { &mut *(ctx as *mut Linq) };
    let cstr = unsafe { std::ffi::CStr::from_ptr(linq_sys::device_serial(*device)) };
    l.on_alert.unwrap()(l, cstr.to_str().expect("to_str() fail!"));
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
        let _l = Linq::new();
    }
}
