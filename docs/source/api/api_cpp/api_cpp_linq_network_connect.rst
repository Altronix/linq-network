.. _ref_api_cpp_linq_network_connect:

connect
=======

Synopsis
--------

Will connect for incoming connections (via IPC, or TCP) depending on the format string passed as the endpoint argument.

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
   linq_network_socket sock = net.connect("tcp://*:33455");

   // ...

   net.close(sock);

.. rubric:: See Also

1. :ref:`ref_api_c_linq_network_connect`
