.. _ref_api_rust_arc_context_device_count:

arc::Context::device_count
==========================

Synopsis
--------

.. code-block:: rust
   
   pub fn device_count(&self) -> u32;

Return the number of devices connected to your LinQ Network context.

Parameters
----------

None.

Examples
--------

.. rubric:: Example printing number of devices connected to the terminal.

.. code-block:: rust

   let count = net.device_count();
