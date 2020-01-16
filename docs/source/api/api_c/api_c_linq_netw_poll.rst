.. include:: ../../roles.rst
.. _ref_api_c_linq_netw_poll:

linq_netw_poll
==============

Synopsis
--------

.. c:function:: E_LINQ_ERROR linq_netw_poll(linq_netw *netw, uint32_t ms)

   Process LinQ Network IO. The LinQ Network Library performs non-blocking IO inside of your thread. To process LinQ Network IO you must call linq_netw_poll routine inside of your main task loop. linq_netw_poll will call any callbacks you have provided from the same thread context as your main application.

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
        linq_netw_poll(linq, 5);
      }

      // ...
   }

.. rst-class:: font-small
.. container::

   **See Also**

   1. sys_running
