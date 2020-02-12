.. _ref_api_cpp_linq_network_poll:

poll
====

Synopsis
--------

Process LinQ Network IO. The LinQ Network Library performs non-blocking IO inside of your thread. To process LinQ Network IO you must call linq_network_poll routine inside of your main task loop. linq_network_poll will call any callbacks you have provided from the same thread context as your main application.


Parameters
----------

=========== ============
Parameter   Description
=========== ============
uint32_t ms Network IO select timeout
=========== ============

Example
-------

.. code-block:: cpp

   altronix::AtxNet net{};

   while(sys_running())
   {
     net.poll(5);
   }

.. rubric:: See Also

1. :ref:`ref_api_c_linq_network_poll`
