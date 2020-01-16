.. include:: ../../roles.rst
.. _ref_api_c_linq_netw_listen:

linq_netw_listen
================

Synopsis
--------

.. c:function:: linq_netw_socket linq_netw_listen(linq_netw, const char \*ep)

   This routine will listen for incoming connections (via IPC, or TCP) depending on the format string passed as the second argument.  Using the linq_netw_listen and linq_netw_close\_... api's you can dynamically change the port numbers you are listening to during runtime.

   .. note:: You must enable the LinQ Library Configuration **WITH_SQLITE** to use the HTTP endpoint

   :param linq_netw: Main context of the LinQ Network Library required for all routines
   :param const char\* ep: String describing the endpoint for the LinQ Network Libary should listen for incoming connections.


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

   linq_netw_socket s = linq_netw_listen(netw, "tcp://*:33455");

   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Listening on port 33455...

   // Close socket and change the port number
   linq_netw_close_router(netw, s);

   s = linq_netw_listen(netw, "tcp://*:33456");
   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Listening on port 33456...

   // Clean up LinQ Network
   linq_netw_destroy(&netw);

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_netw_socket`

   2. :ref:`ref_api_c_linq_netw_connect`

   3. linq_netw_close_router

   4. linq_netw_close_dealer

   5. linq_netw_close_http
