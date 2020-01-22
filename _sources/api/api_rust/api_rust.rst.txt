.. include:: ../../roles.rst
.. _ref-api-rust:

Rust API Reference
==================

The LinQ Network Library provides a Rust crate with idiomatic Rust API, using an async/.await API to process network events and to send and receive device requests.


Modules
-------

:ref:`ref_api_rust_arc` - Thread safe context

:ref:`ref_api_rust_error` - Error types

Enums
-----

:ref:`ref_api_rust_event` - Event types

:ref:`ref_api_rust_endpoint` - Listen / Connect function parameter

:ref:`ref_api_rust_request` - Send / Get / Post / Delete function parameter

Reference
---------

.. toctree::
   :maxdepth: 1
   
   api_rust_arc.rst
   api_rust_arc_context.rst
   api_rust_arc_context_new.rst
   api_rust_arc_context_events.rst
   api_rust_arc_context_listen.rst
   api_rust_arc_context_connect.rst
   api_rust_arc_context_close.rst
   api_rust_arc_context_get.rst
   api_rust_arc_context_post.rst
   api_rust_arc_context_delete.rst
   api_rust_arc_context_send.rst
   api_rust_arc_context_node_count.rst
   api_rust_arc_context_device_count.rst
   api_rust_arc_context_devices.rst
   api_rust_error.rst
   api_rust_event.rst
   api_rust_endpoint.rst
   api_rust_request.rst
