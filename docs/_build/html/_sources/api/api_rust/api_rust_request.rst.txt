.. _ref_api_rust_request:

Request
=======

Synopsis
--------

.. code-block:: rust

   pub enum Request<'a>;

A parameter to be passed to :ref:`ref_api_rust_arc_context_send` to designate the kind of request being sent.

Variants
--------

====================== ===========
Variant                Description
====================== ===========
Get(&'a str)           Send a GET request and associated URL
Post(&'a str, &'a str) Send a POST request and associated URL and BODY
Delete(&'a str)        Send a DELETE request and associated URL
====================== ===========

Methods
-------

None.

Examples
--------

.. rubric:: Send a POST request

.. code-block:: rust

   #![feature(rustc_private)]
   #![feature(proc_macro_hygiene)]
   #![feature(decl_macro)]
   #![feature(async_closure)]

   use futures::prelude::*;
   use futures::stream::StreamExt;
   use std::error::Error;

   #[async_attributes::main]
   async fn main() -> Result<(), Box<dyn Error>> {
      let net = linq_network::arc::Context::new();

      // Get a serial number from a connected device
      let (serial, _) = net
          .events()
          .filter_map(|e| match e {
              linq_network::Event::Heartbeat(s) => future::ready(Some(s)),
              _ => future::ready(None),
          })
          .take(1)
          .into_future()
          .await;

      // Change the SiteID
      if let Some(serial) = serial {
          let r = Request::Post("/ATX/about", "{\"siteId\":\"New Site ID\"}");
          let response = net
              .send(r, &serial)
              .await?
              .result?;
      }

   }
