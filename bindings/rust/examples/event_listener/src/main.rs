#![feature(proc_macro_hygiene)]
#![feature(decl_macro)]
#![feature(async_closure)]

extern crate linq_netw;
use futures::executor::block_on;
use futures::stream::StreamExt;
use linq_netw::{Context, Endpoint, Event};
use std::thread;
use std::time::Duration;

static PORT: u32 = 33455;

fn main() {
    let linq = Context::new();
    linq.listen(Endpoint::Tcp(PORT));

    let events = linq.events().for_each(async move |e| {
        match e {
            Event::Heartbeat(s) => println!("[RECEIVED HEARTBEAT] {}", s),
            Event::Alert(s) => println!("[RECEIVED ALERT] {}", s),
            Event::Error(_, _) => println!("[RECEIVED ERROR]"),
        };
    });

    let t = std::thread::spawn(move || {
        while linq_netw::running() {
            thread::sleep(Duration::from_millis(50));
            linq.poll(1);
        }
    });

    block_on(events);
    t.join().unwrap();
}
