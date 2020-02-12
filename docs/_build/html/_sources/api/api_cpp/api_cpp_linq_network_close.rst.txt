.. _ref_api_cpp_linq_network_close:

close
=====

Synopsis
--------

This routine will close a connection.

Parameters
----------

=================== ============
Parameter           Description
=================== ============
linq_network_socket socket to close. (Returned from linq_network_listen or linq_network_connect)
=================== ============

Example
-------

.. code-block:: cpp

   altronix::AtxNet net{};
   linq_network_socket sock = net.listen("tcp://*:33455");

   // ...

   net.close(sock);

.. rubric:: See Also

1. :ref:`ref_api_c_linq_network_listen`

2. :ref:`ref_api_c_linq_network_connect`
