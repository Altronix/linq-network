.. _ref_api_rust_endpoint:

Endpoint
========

Synopsis
--------

.. code-block:: rust

   pub enum Endpoint;

When linq_network crate needs to listen or connect to a remote endpoint, an endpoint enum must be passed as an argument.

Variants
--------

================= ===========
Variant           Description
================= ===========
Tcp(u32)          A TCP endpoint and a port number
Ipc(&'static str) A IPC endpoint bound to a IPC file on the host OS
================= ===========

Methods
-------

None.

Examples
--------

.. rubric:: listen on endpoint 33455

.. code-block:: rust

   net = linq_network::arc::Context::new();
   net.listen(linq_network::Endpoint::Tcp(33455));
