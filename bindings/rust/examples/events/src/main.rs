// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#![feature(proc_macro_hygiene)]
#![feature(rustc_private)]
#![feature(decl_macro)]
#![feature(async_closure)]

extern crate atx_net;
use atx_net::{Endpoint, Event};

use futures::prelude::*;
use futures::stream::StreamExt;

static PORT: u32 = 33455;

#[async_attributes::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let linq = atx_net::arc::Context::new();
    linq.listen(Endpoint::Tcp(PORT));

    linq.events()
        .take_while(|e| future::ready(*e != Event::Ctrlc))
        .for_each(async move |e| {
            match e {
                Event::Heartbeat(_serial) => (),
                Event::Alert(_serial) => (),
                Event::Error(_, _) => (),
                _ => (),
            };
        })
        .await;
    Ok(())
}
