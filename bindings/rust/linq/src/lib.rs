// pub mod linq;

pub mod linq;
pub use linq::*;

enum Socket {
    Server(linq_sys::linq_socket),
    Client(linq_sys::linq_socket),
}

pub fn running() -> bool {
    unsafe { linq_sys::sys_running() }
}

pub fn init() -> LinqHandle {
    LinqHandle::new()
}

pub struct LinqHandle {
    ctx: Box<LinqContext>,
    sockets: std::collections::HashMap<String, Socket>,
}

impl LinqHandle {
    pub fn new() -> LinqHandle {
        let mut ctx = Box::new(LinqContext::new());
        unsafe {
            linq_sys::linq_context_set(ctx.as_ref().c_ctx, &mut *ctx as *mut LinqContext as *mut _)
        };
        LinqHandle {
            ctx,
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
        let socket = self.ctx.as_ref().listen(s);
        self.sockets.insert(s.to_string(), Socket::Server(socket));
        self
    }

    pub fn connect(&mut self, s: &str) -> &mut LinqHandle {
        let socket = self.ctx.as_ref().connect(s);
        self.sockets.insert(s.to_string(), Socket::Client(socket));
        self
    }

    pub fn shutdown(&mut self, s: &str) -> &mut LinqHandle {
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
