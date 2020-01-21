.. _ref_api_rust_polling_context_connect:

polling::Context::connect
=========================

Synopsis
--------

.. code-block:: rust

   pub fn connect(&self, ep: Endpoint) -> atx_net_socket;

This routine will connect to a remote instance of atx_net (via IPC, or TCP) depending on the endpoint argument.

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

   let net = atx_net::polling::Context::new();
   net.connect(atx_net::Endpoint::Tcp(33455));

.. rubric:: connect to IPC "channel" on the current working directory

.. code-block:: rust

   let net = atx_net::polling::Context::new();
   net.connect(atx_net::Endpoint::Ipc("local"));

.. rubric:: See also

1. :ref:`ref_api_rust_endpoint`
