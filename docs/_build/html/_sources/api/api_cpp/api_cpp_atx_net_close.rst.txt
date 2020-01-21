.. _ref_api_cpp_atx_net_close:

close
=====

Synopsis
--------

This routine will close a connection.

Parameters
----------

============== ============
Parameter      Description
============== ============
atx_net_socket socket to close. (Returned from atx_net_listen or atx_net_connect)
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

2. :ref:`ref_api_c_atx_net_connect`
