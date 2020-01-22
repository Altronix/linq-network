.. _ref_api_rust_arc_context_get:

arc::Context::get
=================

Synopsis
--------

.. code-block:: rust

   pub fn get(&self, path: &str, sid: &str) -> SimpleFuture<Response>;

Send a GET request to a connected device

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

      // Request some about data
      if let Some(serial) = serial {
          let response = net
              .get("/ATX/about", &serial)
              .await?
              .result?;
      }

   }

.. rubric:: Example route based on rocket.rs

.. code-block:: rust

   #[post("/proxy/<id>/about", format = "json")]
   async fn proxy_route(net: State<'_, AtxNet>, id:String) -> Result<content::Json<String>, Status> {
      match net.get("/ATX/about", &id).await {
         Ok(response) => match response.result {
            Ok(v) => Ok(content::Json(v)),
            Err(e) => Err(Status::new(e.to_http(), "")),
         },
         Err(_) => Err(Status::new(400,"")),
      }
   }
