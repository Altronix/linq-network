use std::future::Future;
use std::option::Option;
use std::pin::Pin;
use std::sync::{Arc, Mutex};
use std::task::{Context, Poll, Waker};

pub struct Resolve<T> {
    data: Option<T>,
    waker: Option<Waker>,
}

impl<T> Resolve<T> {
    pub fn resolve(&mut self, data: T) -> () {
        self.data = Some(data);
        if let Some(waker) = self.waker.take() {
            waker.wake();
        }
    }
}

pub struct ResolveFuture<T> {
    pub resolve: Arc<Mutex<Resolve<T>>>,
}

impl<T> ResolveFuture<T> {
    pub fn new() -> Self {
        ResolveFuture {
            resolve: Arc::new(Mutex::new(Resolve {
                data: None,
                waker: None,
            })),
        }
    }
}

impl<T: Copy> Future for ResolveFuture<T> {
    type Output = Result<T, Box<dyn std::error::Error>>;
    fn poll(self: Pin<&mut Self>, ctx: &mut Context<'_>) -> Poll<Self::Output> {
        let mut resolve = self.resolve.lock().unwrap();
        match resolve.data {
            Some(d) => Poll::Ready(Ok(d)),
            _ => {
                resolve.waker = Some(ctx.waker().clone());
                Poll::Pending
            }
        }
    }
}
