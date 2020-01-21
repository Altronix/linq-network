.. _ref_api_rust_arc_context_listen:

listen
======

Synopsis
--------

.. code-block:: rust

   pub fn listen(&self, ep: Endpoint) -> atx_net_socket;

This routine will listen for incoming connections (via IPC, or TCP) depending on the endpoint argument.  Using the listen and close api you can dynamically change the port numbers you are listening to during runtime.

Parameters
----------

========= ===========
Parameter Description
========= ===========
Endpoint  Endpoint location for the LinQ Network Library to listen too.
========= ===========

Examples
--------

.. rubric:: listen on port 33455

.. code-block:: rust

   let net = atx_net::arc::Context::new();
   net.listen(atx_net::Endpoint::Tcp(33455));

.. rubric:: listen on IPC "channel" on the current working directory

.. code-block:: rust

   let net = atx_net::arc::Context::new();
   net.listen(atx_net::Endpoint::Ipc("local"));

.. rubric:: See also

1. :ref:`ref_api_rust_endpoint`
