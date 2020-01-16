.. include:: ../../roles.rst
.. _ref_api_c_linq_netw_socket:

linq_netw_socket
================

.. c:type:: linq_netw_socket

   A socket handle used to represent a specific connection.

Members
-------

The linq_netw_socket is a simple integer used to LinQ Network to index the specific connection.

Example
-------

.. code-block:: c

   linq_netw_socket s = linq_netw_listen(netw, "tcp://*:33455");

   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
   } else {
      // Do something with socket
      // ...

      // Close socket
      linq_netw_close_router(netw, s);
   }

.. rst-class:: font-small
.. container::

   **See Also**

   1. linq_netw_connect

   2. linq_netw_listen
