.. include:: ../../roles.rst
.. _ref_api_c_linq_network_close:

linq_network_close
=============

Synopsis
--------

.. code-block:: c
    
   E_LINQ_ERROR linq_network_close(linq_network_s*, linq_network_socket);

This routine will close a connection.


Parameters
----------

============== ==========
Parameter      Description
============== ==========
linq_network        Main context of the LinQ Network Library passed to all linq_network routines.
linq_network_socket socket to close. (Returned from linq_network_listen or linq_network_connect)
============== ==========


Examples
--------

See :ref:`ref_api_c_linq_network_listen`
