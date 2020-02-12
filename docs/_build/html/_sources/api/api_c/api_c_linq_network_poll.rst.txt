.. include:: ../../roles.rst
.. _ref_api_c_linq_network_poll:

linq_network_poll
=================

Synopsis
--------

.. code-block:: c

   E_LINQ_ERROR linq_network_poll(linq_network_s*, uint32_t ms);

Process LinQ Network IO. The LinQ Network Library performs non-blocking IO inside of your thread. To process LinQ Network IO you must call linq_network_poll routine inside of your main task loop. linq_network_poll will call any callbacks you have provided from the same thread context as your main application.

Parameters
----------

=========== ===========
Parameter   Description
=========== ===========
linq_network_s*  Main context of the LinQ Network Library passed to all linq_network routines.
uint32_t ms Network IO Select timeout
=========== ===========

Example
--------

.. code-block:: c

   int
   main(int argc, char* argv[])
   {
      // Initialize your app
      // ...

      while(sys_running())
      {
        linq_network_poll(linq, 5);
      }

      // ...
   }

.. rst-class:: font-small
.. container::

   **See Also**

   1. sys_running
