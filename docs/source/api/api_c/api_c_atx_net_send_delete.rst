.. include:: ../../roles.rst

atx_net_send_delete
===================

Synopsis
--------

.. code-block:: c
   
   // send a delete request to a device connected to us
   E_LINQ_ERROR
   atx_net_send_delete(
       const atx_net_s* linq,
       const char* serial,
       const char* path,
       atx_net_request_complete_fn fn,
       void* context)
   {
       return zmtp_device_send_delete(&linq->zmtp, serial, path, fn, context);
   } 
