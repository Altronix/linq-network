// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#![feature(proc_macro_hygiene)]
#![feature(decl_macro)]
#![feature(async_closure)]

extern crate linq_netw;
use linq_netw::{Endpoint, Event};

use futures::executor::block_on;
use futures::future::join;
use futures::prelude::*;
use futures::stream::StreamExt;
use std::time::Duration;

static PORT: u32 = 33455;

fn main() {
    let linq = linq_netw::polling::Context::new();
    linq.listen(Endpoint::Tcp(PORT));

    let events = linq
        .events()
        .take_while(|e| future::ready(*e != Event::Ctrlc))
        .for_each(async move |e| {
            match e {
                Event::Heartbeat(s) => println!("[RECEIVED HEARTBEAT] {}", s),
                Event::Alert(s) => println!("[RECEIVED ALERT] {}", s),
                Event::Error(_, _) => println!("[RECEIVED ERROR]"),
                _ => (),
            };
        });

    let linq_poller = async_std::task::spawn(async move {
        while linq_netw::running() {
            futures_timer::Delay::new(Duration::from_millis(50)).await;
            linq.poll(0);
        }
    });

    block_on(join(events, linq_poller));
}
