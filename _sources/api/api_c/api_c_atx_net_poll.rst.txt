.. include:: ../../roles.rst
.. _ref_api_c_atx_net_poll:

atx_net_poll
==============

Synopsis
--------

.. code-block:: c

   E_LINQ_ERROR atx_net_poll(atx_net_s*, uint32_t ms);

Process LinQ Network IO. The LinQ Network Library performs non-blocking IO inside of your thread. To process LinQ Network IO you must call atx_net_poll routine inside of your main task loop. atx_net_poll will call any callbacks you have provided from the same thread context as your main application.

Parameters
----------

=========== ===========
Parameter   Description
=========== ===========
atx_net_s*  Main context of the LinQ Network Library passed to all atx_net routines.
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
        atx_net_poll(linq, 5);
      }

      // ...
   }

.. rst-class:: font-small
.. container::

   **See Also**

   1. sys_running
