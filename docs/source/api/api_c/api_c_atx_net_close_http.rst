.. include:: ../../roles.rst
.. _ref_api_c_atx_net_close_http:

atx_net_close_http
======================

Synopsis
--------

.. c:function:: atx_net_socket atx_net_close_http(atx_net, atx_net_socket socket)

   This routine will shutdown an HTTP service created from atx_net_listen() call with HTTP format string.

   :param atx_net: Main context of the LinQ Network Library required for all routines
   :param atx_net_socket socket: socket handle returned from atx_net_listen

   .. note:: You must enable the LinQ Network Configuration option *WITH_SQLITE* to use the HTTP server.
