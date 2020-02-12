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
       let net = linq_network::arc::Context::new();
       net.listen(linq_network::Endpoint::Tcp(33455));

       let (serial, _) = net
           .events()
           .filter_map(|e| match e {
               linq_network::Event::Heartbeat(s) => future::ready(Some(s)),
               _ => future::ready(None),
           })
           .take(1)
           .into_future()
           .await;
   }
