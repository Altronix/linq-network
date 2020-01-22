.. _ref_api_rust_arc_context_delete:

arc::Context::delete
====================

.. code-block:: rust

   pub fn delete(&self, path: &str, data: &str, sid: &str) -> SimpleFuture<Response>;

Send a DELETE request to a connected device

Parameters
----------

========== ===========
Parameter  Description
========== ===========
path: &str URL of the request
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

      // Delete user John
      if let Some(serial) = serial {
          let response = net
              .delete("/ATX/userManagement/users/john", &serial)
              .await?
              .result?;
      }

   }

.. rubric:: See also

1. :ref:`ref_api_rust_arc_context_get`

2. :ref:`ref_api_rust_arc_context_delete`

3. :ref:`ref_api_rust_arc_context_send`
