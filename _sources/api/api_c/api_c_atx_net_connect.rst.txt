.. include:: ../../roles.rst
.. _ref_api_c_atx_net_connect:

atx_net_connect
=================

Synopsis
--------

.. c:function:: atx_net_socket atx_net_connect(atx_net, const char \*ep)

   This routine will connect to a remote endpoint via IPC, or TCP, depending on the format string passed as the second argument.  Using the atx_net_connect and atx_net_close\_... api's you can dynamically change the port numbers you are connected to during runtime.

   :param atx_net: Main context of the LinQ Network Library required for all routines
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

   atx_net_socket s = atx_net_connect(netw, "tcp://*:33455");

   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Connected to port 33455

   // Close socket and connect to a different port number
   atx_net_close_dealer(netw, s);

   s = atx_net_connect(netw, "tcp://*:33456");
   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Connected to port 33456

   // Clean up LinQ Network
   atx_net_destroy(&netw);

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_atx_net_socket`

   2. :ref:`ref_api_c_atx_net_listen`

   3. :ref:`ref_api_c_atx_net_close_router`

   4. :ref:`ref_api_c_atx_net_close_dealer`

   5. :ref:`ref_api_c_atx_net_close_http`

