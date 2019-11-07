// pub mod linq;

pub mod linq;
pub use linq::*;
use std::collections::HashMap;
use std::sync::{Arc, Mutex};

pub enum Socket {
    Server(linq_sys::linq_socket),
    Client(linq_sys::linq_socket),
}

pub fn running() -> bool {
    unsafe { linq_sys::sys_running() }
}

pub fn init() -> LinqConnection {
    LinqConnection::new()
}

pub fn task(linq: Arc<Mutex<LinqConnection>>) -> () {
    while running() {
        // TODO we are locking to long.
        let linq = linq.lock().unwrap();
        linq.poll(200);
    }
}

pub struct LinqConnection {
    pub ctx: Box<LinqContext>,
    sockets: HashMap<String, Socket>,
}

impl LinqConnection {
    pub fn new() -> LinqConnection {
        let mut ctx = Box::new(LinqContext::new());
        unsafe {
            linq_sys::linq_context_set(ctx.as_ref().c_ctx, &mut *ctx as *mut LinqContext as *mut _)
        };
        LinqConnection {
            ctx,
            sockets: HashMap::new(),
        }
    }

    pub fn start(self) -> () {
        while unsafe { linq_sys::sys_running() } {
            self.poll(200);
        }
    }

    pub fn listen(mut self, ep: Endpoint) -> Self {
        let socket = self.ctx.listen(&ep);
        self.sockets.insert(ep.to_str(), Socket::Server(socket));
        self
    }

    pub fn connect(mut self, ep: Endpoint) -> Self {
        let socket = self.ctx.connect(&ep);
        self.sockets.insert(ep.to_str(), Socket::Client(socket));
        self
    }

    pub fn shutdown(&mut self, s: &str) -> &mut LinqConnection {
        match self.sockets.get(s).unwrap() {
            Socket::Server(s) => self.ctx.as_ref().shutdown(*s),
            Socket::Client(s) => self.ctx.as_ref().disconnect(*s),
        }
        self
    }

    pub fn poll(&self, ms: u32) -> linq_sys::E_LINQ_ERROR {
        self.ctx.as_ref().poll(ms)
    }

    pub fn register(mut self, e: Event) -> Self {
        self.ctx.register(e);
        self
    }
}