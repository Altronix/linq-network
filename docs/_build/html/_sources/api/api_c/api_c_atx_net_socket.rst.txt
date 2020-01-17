.. include:: ../../roles.rst
.. _ref_api_c_atx_net_socket:

atx_net_socket
================

.. c:type:: atx_net_socket

   A socket handle used to represent a specific connection.

Members
-------

The atx_net_socket is a simple integer used to LinQ Network to index the specific connection.

Example
-------

.. code-block:: c

   atx_net_socket s = atx_net_listen(netw, "tcp://*:33455");

   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
   } else {
      // Listening on port 33455 ...

      // Close socket
      atx_net_close_router(netw, s);
   }

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_atx_net_listen`

   2. atx_net_listen
