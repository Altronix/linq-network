.. _ref_api_rust_arc_context_connect:

arc::Context::connect
=====================

Synopsis
--------

.. code-block:: rust

   pub fn connect(&self, ep: Endpoint) -> linq_network_socket;

This routine will connect to a remote instance of linq_network (via IPC, or TCP) depending on the endpoint argument.

Parameters
----------

========= ===========
Parameter Description
========= ===========
Endpoint  Endpoint location for the LinQ Network Library to connect too.
========= ===========

Examples
--------

.. rubric:: connect to port 33455

.. code-block:: rust

   let net = linq_network::arc::Context::new();
   net.connect(linq_network::Endpoint::Tcp(33455));

.. rubric:: connect to IPC "channel" on the current working directory

.. code-block:: rust

   let net = linq_network::arc::Context::new();
   net.connect(linq_network::Endpoint::Ipc("local"));

.. rubric:: See also

1. :ref:`ref_api_rust_endpoint`
