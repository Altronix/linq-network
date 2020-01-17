Quick Start (Rust)
==================

The LinQ Network Library provides a Rust crate with idiomatic Rust API, using an async/.await API to process network events and to send and receive device requests.

Project Setup
-------------

To get started with the Rust binding, add the linq-netw crate to your Cargo.toml file.

*Cargo.toml*

.. code-block:: toml

   [package]
   name = "example-project"
   version = "0.0.1"
   edition = "2018"
   
   # See more keys and their definitions at https://doc.rust-lang.org/cargo/reference/manifest.html
   
   [dependencies]
   linq-netw = "0.0.1"

Configuration Options
---------------------

.. attention:: TODO

Single-Threaded
---------------

LinQ Network can share the same process as your application, or provides a thread safe wrapper to use in multi-threaded applications. To get started with a single-threaded project, initialize LinQ Network using the *Polling* Context.

.. code-block:: rust

   fn main() {
       let linq = atx_net::polling::Context::new();
       linq.listen(Endpoint::Tcp(PORT));

Multi-Threaded Async
--------------------

LinQ Network can share the same process as your application, or provides a thread safe wrapper to use in multi-threaded applications. To get started with a multi-threaded project, initialize LinQ Network using the *Arc* Context.

.. code-block:: rust

   #[async_attributes::main]
   async fn main() -> Result<(), Box<dyn std::error::Error>> {
       let linq = atx_net::arc::Context::new();
       linq.listen(Endpoint::Tcp(PORT));

Event Stream
------------

The LinQ Network Rust binding uses a stream API to process incoming events. Use the events() method to get a handle to the event stream. Then use std future methods to process the incoming stream.

*Example - Listen for a single heartbeat and get the serial number*

.. code-block:: rust

   let (serial, _) = linq
       .events()
       .filter_map(|e| match e {
           linq::netw::Event::Heartbeat(s) => future::ready(Some(s)),
           _ => future::ready(None),
       })
       .take(1)
       .into_future()
       .await;

Complete Example
----------------

.. code-block:: rust


   #![feature(proc_macro_hygiene)]
   #![feature(decl_macro)]
   #![feature(async_closure)]
   
   extern crate atx_net;
   use atx_net::{Endpoint, Event};
   
   use futures::executor::block_on;
   use futures::future::join;
   use futures::prelude::*;
   use futures::stream::StreamExt;
   use std::time::Duration;
   
   static PORT: u32 = 33455;
   
   fn main() {
       let linq = atx_net::polling::Context::new();
       linq.listen(Endpoint::Tcp(PORT));
   
       let events = linq
           .events()
           .take_while(|e| future::ready(*e != Event::Ctrlc))
           .for_each(async move |e| {
               match e {
                   Event::Heartbeat(_serial) => (),
                   Event::Alert(_serial) => (),
                   Event::Error(_, _) => (),
                   _ => (),
               };
           });
   
       let linq_poller = async_std::task::spawn(async move {
           while atx_net::running() {
               futures_timer::Delay::new(Duration::from_millis(50)).await;
               linq.poll(0);
           }
       });
   
       block_on(join(events, linq_poller));
   }

Complete Example (Async)
------------------------


.. code-block:: rust

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
