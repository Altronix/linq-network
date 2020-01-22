.. _ref_api_rust_endpoint:

Endpoint
========

Synopsis
--------

.. code-block:: rust

   pub enum Endpoint;

When atx_net crate needs to listen or connect to a remote endpoint, an endpoint enum must be passed as an argument.

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

   net = atx_net::arc::Context::new();
   net.listen(atx_net::Endpoint::Tcp(33455));
