.. _ref_api_rust_polling_context_new:

polling::Context::new
=====================

Synopsis
--------

.. code-block:: rust
   
   pub fn new() -> Self;

Create a new instance of an AtxNet object. The struct return contains methods for interacting with the LinQ Network library.

Parameters
----------

None.

Examples
--------

.. code-block:: rust

   fn main() {
     let net = atx_net::polling::Context::new();

     // ...
   }
