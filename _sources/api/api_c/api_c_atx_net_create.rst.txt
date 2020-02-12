.. include:: ../../roles.rst
.. _ref_api_c_linq_network_create:

linq_network_create
================

Synopsis
--------


.. code-block:: c

   linq_network_s* linq_network_create(const linq_network_callbacks*, void*);

Initialize the LinQ Network communication library. Optionally pass in a struct with function pointers to receive notifications of network events. Also optionally pass a context that will be passed to your applications callback function during linq network events.

Parameters
----------

======================== ===========
Parameter                Description
======================== ===========
const linq_network_callbacks* Struct of function pointers to process events
void*                    User data passed to each callback function to process events.
======================== ===========

Examples
--------

.. rubric:: With Callbacks

.. code-block:: c

   // Declare some callbacks
   static void on_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial);
   static void on_alert(void* ctx, linq_network_alert_s* alert, linq_network_email_s* mail, device_s** d);
   static void on_heartbeat(void* ctx, const char* serial, device_s** d);
   static void on_ctrlc(void* ctx);

   linq_network_callbacks callbacks = { .err = on_error,
                                     .alert = on_alert,
                                     .hb = on_heartbeat,
                                     .ctrc = on_ctrlc };
   int main(int argc, char *argv[])
   {
     // Create some app specific thing you need!
     app_context* app = app_context_create();

     // Create some linq networking
     linq_network* netw = linq_network_create(callbacks, app);

     // Process Network IO
     while (sys_running()) linq_network_poll(netw, 5);

     // All done ...
     linq_network_destroy(&netw);
   }


.. rubric:: Without Callbacks

.. code-block:: c

   int main(int argc, char *argv[])
   {
      linq_network* netw = linq_network_create(NULL, NULL);

     // Process Network IO
     while (sys_running()) linq_network_poll(netw, 5);

      // All done ...
      linq_network_destroy(&netw);
   }

.. rubric:: See Also

:ref:`linq_network_destroy <ref_api_c_linq_network_destroy>`

:ref:`linq_network_poll<ref_api_c_linq_network_poll>`
