.. include:: ../../roles.rst
.. _ref_api_c_linq_network_socket:

linq_network_socket
===================

.. code-block:: c

   typedef uint32_t linq_network_socket;

A socket handle used to represent a specific connection.

Example
-------

.. code-block:: c

   linq_network_socket s = linq_network_listen(netw, "tcp://*:33455");

   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
   } else {
      // Listening on port 33455 ...

      // Close socket
      linq_network_close(netw, s);
   }

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_network_listen`

   2. linq_network_listen
