.. include:: ../../roles.rst
.. _ref-api-rust:

Rust API Reference
==================

The LinQ Network Library provides a Rust crate with idiomatic Rust API, using an async/.await API to process network events and to send and receive device requests.There is a thread safe API to process network IO in a seperate thread using the atx_net::arc module, and a lower level single threaded API using the atx_net::polling module.


Modules
-------

:ref:`ref_api_rust_polling` - Lower level API

:ref:`ref_api_rust_arc` - Thread safe context

:ref:`ref_api_rust_error` - Error types

.. toctree::
   :maxdepth: 5
   :hidden:

   api_rust_arc.rst
   api_rust_polling.rst
   api_rust_error.rst

Enums
-----

:ref:`ref_api_rust_event` - Event types

:ref:`ref_api_rust_endpoint` - Listen / Connect function parameter

:ref:`ref_api_rust_request` - Send / Get / Post / Delete function parameter

.. toctree::
   :maxdepth: 5
   :hidden:

   api_rust_event.rst
   api_rust_endpoint.rst
   api_rust_request.rst

.. :ref:`polling::Context::new <ref_api_rust_polling_context_new>`
.. ---------------------------------------------------------------
.. 
.. :ref:`polling::Context::events <ref_api_rust_polling_context_events>`
.. ---------------------------------------------------------------------
.. 
.. :ref:`polling::Context::listen <ref_api_rust_polling_context_listen>`
.. ---------------------------------------------------------------------
.. 
.. :ref:`polling::Context::connect <ref_api_rust_polling_context_connect>`
.. -----------------------------------------------------------------------
.. 
.. :ref:`polling::Context::close <ref_api_rust_polling_context_close>`
.. -------------------------------------------------------------------
.. 
.. :ref:`polling::Context::get <ref_api_rust_polling_context_get>`
.. ---------------------------------------------------------------
.. 
.. :ref:`polling::Context::post <ref_api_rust_polling_context_post>`
.. -----------------------------------------------------------------
.. 
.. :ref:`polling::Context::delete <ref_api_rust_polling_context_delete>`
.. ---------------------------------------------------------------------
.. 
.. :ref:`polling::Context::send <ref_api_rust_polling_context_send>`
.. -----------------------------------------------------------------
.. 
.. :ref:`polling::Context::node_count <ref_api_rust_polling_context_node_count>`
.. -----------------------------------------------------------------------------
.. 
.. :ref:`polling::Context::device_count <ref_api_rust_polling_context_device_count>`
.. ---------------------------------------------------------------------------------
.. 
.. :ref:`polling::Context::devices <ref_api_rust_polling_context_devices>`
.. -----------------------------------------------------------------------
.. 
.. :ref:`arc::Context::new <ref_api_rust_arc_context_new>`
.. -------------------------------------------------------
.. 
.. :ref:`arc::Context::events <ref_api_rust_arc_context_events>`
.. -------------------------------------------------------------
.. 
.. :ref:`arc::Context::listen <ref_api_rust_arc_context_listen>`
.. -------------------------------------------------------------
.. 
.. :ref:`arc::Context::connect <ref_api_rust_arc_context_connect>`
.. ---------------------------------------------------------------
.. 
.. :ref:`arc::Context::close <ref_api_rust_arc_context_close>`
.. -----------------------------------------------------------
.. 
.. :ref:`arc::Context::get <ref_api_rust_arc_context_get>`
.. -------------------------------------------------------
.. 
.. :ref:`arc::Context::post <ref_api_rust_arc_context_post>`
.. ---------------------------------------------------------
.. 
.. :ref:`arc::Context::delete <ref_api_rust_arc_context_delete>`
.. -------------------------------------------------------------
.. 
.. :ref:`arc::Context::send <ref_api_rust_arc_context_send>`
.. ---------------------------------------------------------
.. 
.. :ref:`arc::Context::node_count <ref_api_rust_arc_context_node_count>`
.. ---------------------------------------------------------------------
.. 
.. :ref:`arc::Context::device_count <ref_api_rust_arc_context_device_count>`
.. -------------------------------------------------------------------------
.. 
.. :ref:`arc::Context::devices <ref_api_rust_arc_context_devices>`
.. ---------------------------------------------------------------
