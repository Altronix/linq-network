.. include:: ../../roles.rst
.. _ref_api_c_linq_network_listen:

linq_network_listen
===================

Synopsis
--------

.. code-block:: c

   linq_network_socket linq_network_listen(linq_network_s* net, const char *ep);


This routine will listen for incoming connections (via IPC, or TCP) depending on the format string passed as the second argument.  Using the linq_network_listen and linq_network_close\_... api's you can dynamically change the port numbers you are listening to during runtime.

.. note:: You must enable the LinQ Library Configuration **WITH_SQLITE** to use the HTTP endpoint

Parameters
----------

============== ===========
Parameter      Description
============== ===========
linq_network_s      Main context of the LinQ Network Library passed to all linq_network routines.
const char* ep String describing the endpoint the LinQ Network Library should connect to.
============== ===========


Examples
--------

.. rubric:: Example Endpoints

======== ============== ===========
Endpoint Example        Description
======== ============== ===========
TCP      tcp://\*:33455 Listen incoming connections on the network port 33455
HTTP     http://\*:8080 Listen for incoming HTTP connections on port 8080
IPC      ipc://local    Listen for incoming connections on IPC "local" of the current working directory of the running application.
======== ============== ===========

.. rubric:: Listen for incoming connections

.. code-block:: c

   linq_network_socket s = linq_network_listen(netw, "tcp://*:33455");

   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Listening on port 33455...

   // Close socket and change the port number
   linq_network_close(netw, s);

   s = linq_network_listen(netw, "tcp://*:33456");
   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Listening on port 33456...

   // Clean up LinQ Network
   linq_network_destroy(&netw);

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_network_socket`

   2. :ref:`ref_api_c_linq_network_connect`

   3. :ref:`ref_api_c_linq_network_close`
