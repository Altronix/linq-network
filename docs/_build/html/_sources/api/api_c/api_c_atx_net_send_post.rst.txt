.. include:: ../../roles.rst

atx_net_send_post
=================

Synopsis
--------

.. code-block:: c

   E_LINQ_ERROR
   atx_net_send_post(
       const atx_net_s* linq,
       const char* serial,
       const char* path,
       const char* json,
       atx_net_request_complete_fn fn,
       void* context)
   {
       return zmtp_device_send_post(&linq->zmtp, serial, path, json, fn, context);
   }

Send a POST request to a device.

Paremters
---------

=============================== ===========
Parameter                       Description
=============================== ===========
atx_net_s* linq                 Main context of the LinQ Network Library passed to all atx_net routines
const char* serial              Serial number of the product for which to receive the request
const char* path                URL of the request
const char* json                Body of the request
atx_net_request_complete_fn fn  Callback function with response from the request
void* context                   Application data passed to the callback
=============================== ===========

Examples
--------

.. rubric:: Make a request to change the Site ID of the device

.. code-block:: c

   static void
   on_receive_response(void* context, E_LINQ_ERROR error, const char *response)
   {
     if (error == LINQ_ERROR_OK) {
       printf("Received %s from the device\n", response);
     } else {
       printf("Received error %d from the device\n", error);
     }
   }

   static void
   update_site_id(atx_net_s* net)
   {
     atx_net_send_post(
         net, 
         "B3445ED2CDr2AC33298CXdd443", 
         "/ATX/about", 
         "{\"siteId\": \"New Site ID\"}",
         on_receive_response, 
         NULL);
   }
