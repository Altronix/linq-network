// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

use crate::event;
use crate::polling;
use crate::simple_future::SimpleFuture;
use polling::{atx_net_socket, Endpoint, Request, Response};
use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::time::Duration;

pub struct ThreadContext {
    context: polling::Context,
    shutdown: bool,
}

pub struct Context {
    mutex: Arc<Mutex<ThreadContext>>,
    thread: Option<std::thread::JoinHandle<()>>,
}

impl Context {
    pub fn new() -> Self {
        let mutex = Arc::new(Mutex::new(ThreadContext {
            context: polling::Context::new(),
            shutdown: false,
        }));
        let clone = Arc::clone(&mutex);
        let thread = Some(std::thread::spawn(move || loop {
            std::thread::sleep(Duration::from_millis(50));
            let clone = clone.lock().unwrap();
            clone.context.poll(0);
            if clone.shutdown {
                break;
            };
        }));
        Context { mutex, thread }
    }

    pub fn listen(&self, ep: Endpoint) -> atx_net_socket {
        self.mutex.lock().unwrap().context.listen(ep)
    }

    pub fn events(&self) -> event::EventStream {
        self.mutex.lock().unwrap().context.events()
    }

    pub fn connect(&self, ep: Endpoint) -> atx_net_socket {
        self.mutex.lock().unwrap().context.connect(ep)
    }

    pub fn close(&self, s: &atx_net_socket) -> &Self {
        self.mutex.lock().unwrap().context.close(s);
        self
    }

    pub fn get(&self, path: &str, sid: &str) -> SimpleFuture<Response> {
        self.mutex.lock().unwrap().context.get(path, sid)
    }

    pub fn post(&self, p: &str, d: &str, id: &str) -> SimpleFuture<Response> {
        self.mutex.lock().unwrap().context.post(p, d, id)
    }

    pub fn delete(&self, p: &str, id: &str) -> SimpleFuture<Response> {
        self.mutex.lock().unwrap().context.delete(p, id)
    }

    pub fn send(&self, r: Request, sid: &str) -> SimpleFuture<Response> {
        self.mutex.lock().unwrap().context.send(r, sid)
    }

    pub fn device_count(&self) -> u32 {
        self.mutex.lock().unwrap().context.device_count()
    }

    pub fn devices(&self) -> HashMap<String, String> {
        self.mutex.lock().unwrap().context.devices()
    }

    pub fn node_count(&self) -> u32 {
        self.mutex.lock().unwrap().context.node_count()
    }

    fn shutdown(&self) {
        self.mutex.lock().unwrap().shutdown = true;
    }
}

impl Drop for Context {
    fn drop(&mut self) {
        self.shutdown();
        self.thread.take().unwrap().join().unwrap();
    }
}

impl AsRef<Context> for Context {
    fn as_ref(&self) -> &Self {
        self
    }
}
