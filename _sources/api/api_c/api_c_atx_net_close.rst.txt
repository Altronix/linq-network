.. include:: ../../roles.rst
.. _ref_api_c_atx_net_close:

atx_net_close
=============

Synopsis
--------

.. code-block:: c
    
   E_LINQ_ERROR atx_net_close(atx_net_s*, atx_net_socket);

This routine will close a connection.


Parameters
----------

============== ==========
Parameter      Description
============== ==========
atx_net        Main context of the LinQ Network Library passed to all atx_net routines.
atx_net_socket socket to close. (Returned from atx_net_listen or atx_net_connect)
============== ==========


Examples
--------

See :ref:`ref_api_c_atx_net_listen`
