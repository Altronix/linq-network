.. _ref_api_cpp_atx_net_listen:

listen
======

Synopsis
--------

Will listen for incoming connections (via IPC, or TCP) depending on the format string passed as the endpoint argument.

Parameters
----------

============== ============
Parameter      Description
============== ============
const char* ep String describing the endpoint the LinQ Network Library should connect to.
============== ============

Example
-------

.. code-block:: cpp

   altronix::AtxNet net{};
   atx_net_socket sock = net.listen("tcp://*:33455");

   // ...

   net.close(sock);

.. rubric:: See Also

1. :ref:`ref_api_c_atx_net_listen`
