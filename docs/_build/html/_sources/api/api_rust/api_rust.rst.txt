.. include:: ../../roles.rst
.. _ref-api-rust:

Rust API Reference
==================

The LinQ Network Library provides a Rust crate with idiomatic Rust API, using an async/.await API to process network events and to send and receive device requests.There is a thread safe API to process network IO in a seperate thread using the atx_net::arc module, and a lower level single threaded API using the atx_net::polling module.


Modules
-------

.. toctree::
   :maxdepth: 5

   api_rust_arc.rst
   api_rust_polling.rst
   api_rust_error.rst
