.. _ref_api_rust_atx_net:

atx-net
=======

The LinQ Network Library provides a Rust crate with idiomatic Rust API, using an async/.await API to process network events and to send and receive device requests.There is a thread safe API to process network IO in a seperate thread using the atx_net::arc module, and a lower level single threaded API using the atx_net::polling module.


Modules
-------

.. code-block:: rust

   pub mod polling;
   pub mod arc;
   pub mod error;


.. rubric:: polling

Single threaded context

.. rubric:: polling

Thread safe context

.. rubric:: error

Error types
