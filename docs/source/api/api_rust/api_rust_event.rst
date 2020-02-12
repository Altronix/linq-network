.. _ref_api_rust_event:

Event
=====

Synopsis
--------

.. code-block:: rust

   #[derive(PartialEq)]
   pub enum Event;

The linq_network stream API returns a stream of Events. The event is represented as an enum variant which is bound to event specific data.

Variants
--------

=========================== ===========
Variant                     Description
=========================== ===========
Heartbeat(String)           A heartbeat event and assosiated serial ID accountable for the heartbeat
Alert(String)               TODO
Error(E_LINQ_ERROR, String) A error event, and assosicated error code. If a device is associated with the error, then the serial number of the device is also passed to the enum.
Ctrlc                       A Control-C event, or a shutdown signal, has occured.
=========================== ===========

Methods
-------

None.

Examples
--------

.. rubric:: listen for events and process events as they occur

.. code-block:: rust

   #[async_attributes::main]
   async fn main() -> Result<(), Box<dyn std::error::Error>> {
       let net = linq_network::arc::Context::new();
       net.listen(Endpoint::Tcp(PORT));
   
       net.events()
           .take_while(|e| future::ready(*e != Event::Ctrlc))
           .for_each(async move |e| {
               match e {
                   Event::Heartbeat(serial) => process_heartbeat(serial),
                   Event::Alert(serial) => process_alert(serial),
                   Event::Error(error, serial) => process_error(error, serial),
                   _ => (),
               };
           })
           .await;
       Ok(())
   }
