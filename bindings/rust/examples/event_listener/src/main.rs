#![feature(proc_macro_hygiene)]
#![feature(decl_macro)]
#![feature(async_closure)]

extern crate linq_netw;
use linq_netw::{Endpoint, Event};

use futures::executor::block_on;
use futures::prelude::*;
use futures::stream::StreamExt;

static PORT: u32 = 33455;

fn main() {
    let linq = linq_netw::arc::Context::new();
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

    block_on(events);
}
