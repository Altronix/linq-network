.. include:: ../../roles.rst

linq_network_send_delete
========================

Synopsis
--------

.. code-block:: c
   
   // send a delete request to a device connected to us
   E_LINQ_ERROR
   linq_network_send_delete(
       const linq_network_s* linq,
       const char* serial,
       const char* path,
       linq_network_request_complete_fn fn,
       void* context)
   {
       return zmtp_device_send_delete(&linq->zmtp, serial, path, fn, context);
   } 

Send a DELETE request to a device.

Parameters
----------

=============================== ===========
Parameter                       Description
=============================== ===========
linq_network_s* linq                 Main context of the LinQ Network Library passed to all linq_network routines
const char* serial              Serial number of the product for which to receive the request
const char* path                URL of the request
linq_network_request_complete_fn fn  Callback function with response from the request
void* context                   Application data passed to the callback
=============================== ===========

Examples

.. rubric:: Delete a user from the device

.. code-block:: c

   static void
   on_receive_response(
       void* context,
       E_LINQ_ERROR error,
       const char *response
       device_s** device)
   {
     if (error == LINQ_ERROR_OK) {
       printf("Received %s from the device\n", response);
     } else {
       printf("Received error %d from the device\n", error);
     }
   }

   static void
   remove_user(linq_network_s* netw)
   {
     linq_network_send_delete(
         net, 
         "B3445ED2CDr2AC33298CXdd443", 
         "/ATX/userManagement/users/john_smith", 
         on_receive_response, 
         NULL);
   }
