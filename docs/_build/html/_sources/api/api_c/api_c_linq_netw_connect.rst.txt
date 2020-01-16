.. include:: ../../roles.rst
.. _ref_api_c_linq_netw_connect:

linq_netw_connect
=================

Synopsis
--------

.. c:function:: linq_netw_socket linq_netw_connect(linq_netw, const char \*ep)

   This routine will connect to a remote endpoint via IPC, or TCP, depending on the format string passed as the second argument.  Using the linq_netw_connect and linq_netw_close\_... api's you can dynamically change the port numbers you are connected to during runtime.

   :param linq_netw: Main context of the LinQ Network Library required for all routines
   :param const char\* ep: String describing the endpoint for the LinQ Network Libary should connect to.


Examples
--------

.. rubric:: Example Endpoints

======== ============== ===========
Endpoint Example        Description
======== ============== ===========
TCP      tcp://\*:33455 Connect to TCP port 33455
IPC      ipc://local    Connect to a local IPC socket on the host system
======== ============== ===========

.. rubric:: Connect to other instances of LinQ Network 

.. code-block:: c

   linq_netw_socket s = linq_netw_connect(netw, "tcp://*:33455");

   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Connected to port 33455

   // Close socket and connect to a different port number
   linq_netw_close_dealer(netw, s);

   s = linq_netw_connect(netw, "tcp://*:33456");
   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Connected to port 33456

   // Clean up LinQ Network
   linq_netw_destroy(&netw);

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_netw_socket`

   2. :ref:`ref_api_c_linq_netw_listen`

   3. :ref:`ref_api_c_linq_netw_close_router`

   4. :ref:`ref_api_c_linq_netw_close_dealer`

   5. :ref:`ref_api_c_linq_netw_close_http`

