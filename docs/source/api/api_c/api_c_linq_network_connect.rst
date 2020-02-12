.. include:: ../../roles.rst
.. _ref_api_c_linq_network_connect:

linq_network_connect
====================

Synopsis
--------

.. code-block:: c

   linq_network_socket linq_network_connect(linq_network_s*, const char* ep);

This routine will connect to a remote endpoint via IPC, or TCP, depending on the format string passed as the second argument.  Using the linq_network_connect and linq_network_close\_... api's you can dynamically change the port numbers you are connected to during runtime.


Parameters
----------

============== ===========
Parameter      Description
============== ===========
linq_network        Main context of the LinQ Network Library passed to all linq_network routines.
const char* ep String describing the endpoint the LinQ Network Library should connect to.
============== ===========


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

   linq_network_socket s = linq_network_connect(netw, "tcp://*:33455");

   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Connected to port 33455

   // Close socket and connect to a different port number
   linq_network_close_dealer(netw, s);

   s = linq_network_connect(netw, "tcp://*:33456");
   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Connected to port 33456

   // Clean up LinQ Network
   linq_network_destroy(&netw);

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_network_socket`

   2. :ref:`ref_api_c_linq_network_listen`

   3. :ref:`ref_api_c_linq_network_close`
