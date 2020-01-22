.. _ref_api_rust_arc_context_send:

arc::Context::send
==================

.. code-block:: rust

   pub fn send(&self, r: Request, sid: &str) -> SimpleFuture<Response>;

Send a request to a connected device.

Parameters
----------

========== ===========
Parameter  Description
========== ===========
r: Request A request object
sid: &str  Serial number of the device for which to receive the request
========== ===========

Examples
--------

.. rubric:: Example using executor

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
      let net = atx_net::arc::Context::new();

      // Get a serial number from a connected device
      let (serial, _) = net
          .events()
          .filter_map(|e| match e {
              net::Event::Heartbeat(s) => future::ready(Some(s)),
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

.. rubric:: See also

1. :ref:`ref_api_rust_arc_context_get`

2. :ref:`ref_api_rust_arc_context_send`

3. :ref:`ref_api_rust_request`
