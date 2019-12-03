// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

use std::future::Future;
use std::option::Option;
use std::pin::Pin;
use std::sync::{Arc, Mutex};
use std::task::{Context, Poll, Waker};

pub struct SimpleFutureState<T> {
    data: Option<T>,
    waker: Option<Waker>,
}

impl<T> SimpleFutureState<T> {
    pub fn resolve(&mut self, data: T) -> () {
        self.data = Some(data);
        if let Some(waker) = self.waker.take() {
            waker.wake();
        }
    }
}

pub struct SimpleFuture<T> {
    pub state: Arc<Mutex<SimpleFutureState<T>>>,
}

impl<T> SimpleFuture<T> {
    pub fn new() -> Self {
        SimpleFuture {
            state: Arc::new(Mutex::new(SimpleFutureState {
                data: None,
                waker: None,
            })),
        }
    }
}

impl<T: Clone> Future for SimpleFuture<T> {
    type Output = Result<T, Box<dyn std::error::Error>>;
    fn poll(self: Pin<&mut Self>, ctx: &mut Context<'_>) -> Poll<Self::Output> {
        let mut state = self.state.lock().unwrap();
        match &state.data {
            Some(d) => Poll::Ready(Ok(d.clone())),
            _ => {
                state.waker = Some(ctx.waker().clone());
                Poll::Pending
            }
        }
    }
}
