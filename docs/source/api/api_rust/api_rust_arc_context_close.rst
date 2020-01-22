.. _ref_api_rust_arc_context_close:

arc::Context::close
===================

Synopsis
--------

.. code-block:: rust

   pub fn close(&self, s: &atx_net_socket) -> &Self;

This routine will close a connection created by :ref:`ref_api_rust_arc_context_listen` and :ref:`ref_api_rust_arc_context_connect`

Parameters
----------

=============== ===========
Parameter       Description
=============== ===========
&atx_net_socket socket handle representing the connection to close (returned from :ref:`ref_api_rust_arc_context_listen` and :ref:`ref_api_rust_arc_context_connect`
=============== ===========

Examples
--------

.. rubric:: close connection on port 33456

.. code-block:: rust

   let net = atx_net::arc::Context::new();
   let socka = net.listen(atx_net::Endpoint::Tcp(33455));
   let sockb = net.listen(atx_net::Endpoint::Tcp(33456));

   // ...

   net.close(&sockb);

.. rubric:: See also

1. :ref:`ref_api_rust_arc_context_listen`

2. :ref:`ref_api_rust_arc_context_connect`
