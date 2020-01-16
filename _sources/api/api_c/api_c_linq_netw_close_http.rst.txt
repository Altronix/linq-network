.. include:: ../../roles.rst
.. _ref_api_c_linq_netw_close_http:

linq_netw_close_http
======================

Synopsis
--------

.. c:function:: linq_netw_socket linq_netw_close_http(linq_netw, linq_netw_socket socket)

   This routine will shutdown an HTTP service created from linq_netw_listen() call with HTTP format string.

   :param linq_netw: Main context of the LinQ Network Library required for all routines
   :param linq_netw_socket socket: socket handle returned from linq_netw_listen

   .. note:: You must enable the LinQ Network Configuration option *WITH_SQLITE* to use the HTTP server.
