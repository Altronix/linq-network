extern crate linq_netw_sys;
use linq_netw_sys::*;
use std::future::Future;
use std::option::Option;
use std::pin::Pin;
use std::sync::{Arc, Mutex};
use std::task::{Context as Ctx, Poll, Waker};

pub struct Shutdown {
    result: bool,
    waker: Option<Waker>,
}

pub struct ShutdownFuture {
    pub shutdown: Arc<Mutex<Shutdown>>,
}

impl ShutdownFuture {
    pub fn new() -> Self {
        ShutdownFuture {
            shutdown: Arc::new(Mutex::new(Shutdown {
                result: false,
                waker: None,
            })),
        }
    }
}

impl Future for ShutdownFuture {
    type Output = bool;
    fn poll(self: Pin<&mut Self>, ctx: &mut Ctx<'_>) -> Poll<Self::Output> {
        let mut shutdown = self.shutdown.lock().unwrap();
        match shutdown.result {
            true => Poll::Ready(true),
            false => {
                shutdown.waker = Some(ctx.waker().clone());
                Poll::Pending
            }
        }
    }
}

pub fn running() -> bool {
    unsafe { sys_running() }
}
