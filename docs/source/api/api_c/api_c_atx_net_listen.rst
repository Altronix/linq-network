.. include:: ../../roles.rst
.. _ref_api_c_atx_net_listen:

atx_net_listen
================

Synopsis
--------

.. c:function:: atx_net_socket atx_net_listen(atx_net, const char \*ep)

   This routine will listen for incoming connections (via IPC, or TCP) depending on the format string passed as the second argument.  Using the atx_net_listen and atx_net_close\_... api's you can dynamically change the port numbers you are listening to during runtime.

   .. note:: You must enable the LinQ Library Configuration **WITH_SQLITE** to use the HTTP endpoint

   :param atx_net: Main context of the LinQ Network Library required for all routines
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

   atx_net_socket s = atx_net_listen(netw, "tcp://*:33455");

   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Listening on port 33455...

   // Close socket and change the port number
   atx_net_close(netw, s);

   s = atx_net_listen(netw, "tcp://*:33456");
   if (s == LINQ_ERROR_SOCKET) {
      // Socket error ...
      return -1;
   }

   // Listening on port 33456...

   // Clean up LinQ Network
   atx_net_destroy(&netw);

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_atx_net_socket`

   2. :ref:`ref_api_c_atx_net_connect`

   3. :ref:`ref_api_c_atx_net_close`
