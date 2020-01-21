.. _ref_api_rust_arc_context_events:

arc::Context::events
====================

Synopsis
--------

.. code-block:: rust

   pub fn events(&self) -> event::EventStream;

This method returns a stream that is compatibile with futures::stream::Stream crate.

Parameters
----------

None.

Examples
--------

.. rubric:: Wait for incoming heartbeat and get the serial number

.. code-block:: rust

   #[async_attributes::main]
   async fn main() -> Result<(), Box<dyn Error>> {
       let net = atx_net::arc::Context::new();
       net.listen(atx_net::Endpoint::Tcp(33455));

       let (serial, _) = net
           .events()
           .filter_map(|e| match e {
               atx_net::Event::Heartbeat(s) => future::ready(Some(s)),
               _ => future::ready(None),
           })
           .take(1)
           .into_future()
           .await;
   }
