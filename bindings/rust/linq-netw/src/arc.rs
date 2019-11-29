use crate::event;
use crate::polling;
use crate::simple_future::SimpleFuture;
use polling::{Endpoint, Request, Response, Socket};
use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::time::Duration;

pub struct Context {
    context: Arc<Mutex<polling::Context>>,
    thread: Option<std::thread::JoinHandle<()>>,
}

impl Context {
    pub fn new() -> Self {
        let context = Arc::new(Mutex::new(polling::Context::new()));
        let clone = Arc::clone(&context);
        let thread = Some(std::thread::spawn(move || {
            while polling::running() {
                std::thread::sleep(Duration::from_millis(50));
                clone.lock().unwrap().poll(0);
            }
        }));
        Context { context, thread }
    }

    pub fn listen(&self, ep: Endpoint) -> Socket {
        self.context.lock().unwrap().listen(ep)
    }

    pub fn events(&self) -> event::EventStream {
        self.context.lock().unwrap().events()
    }

    pub fn connect(&self, ep: Endpoint) -> Socket {
        self.context.lock().unwrap().connect(ep)
    }

    pub fn close(&self, s: &Socket) -> &Self {
        self.context.lock().unwrap().close(s);
        self
    }

    pub fn send(&self, r: Request, sid: &str) -> SimpleFuture<Response> {
        self.context.lock().unwrap().send(r, sid)
    }

    pub fn device_count(&self) -> u32 {
        self.context.lock().unwrap().device_count()
    }

    pub fn devices(&self) -> HashMap<String, String> {
        self.context.lock().unwrap().devices()
    }

    pub fn node_count(&self) -> u32 {
        self.context.lock().unwrap().node_count()
    }
}

impl Drop for Context {
    fn drop(&mut self) {
        self.thread.take().unwrap().join().unwrap();
    }
}

impl AsRef<Context> for Context {
    fn as_ref(&self) -> &Self {
        self
    }
}
