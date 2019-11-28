#![feature(proc_macro_hygiene)]
#![feature(rustc_private)]
#![feature(decl_macro)]
#![feature(async_closure)]

extern crate linq_netw;
use linq_netw::{Endpoint, Event};

use futures::prelude::*;
use futures::stream::StreamExt;

static PORT: u32 = 33455;

#[async_attributes::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let linq = linq_netw::arc::Context::new();
    linq.listen(Endpoint::Tcp(PORT));

    linq.events()
        .take_while(|e| future::ready(*e != Event::Ctrlc))
        .for_each(async move |e| {
            match e {
                Event::Heartbeat(s) => println!("[RECEIVED HEARTBEAT] {}", s),
                Event::Alert(s) => println!("[RECEIVED ALERT] {}", s),
                Event::Error(_, _) => println!("[RECEIVED ERROR]"),
                _ => (),
            };
        })
        .await;
    Ok(())
}
