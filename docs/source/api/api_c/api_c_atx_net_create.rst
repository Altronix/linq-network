.. include:: ../../roles.rst
.. _ref_api_c_atx_net_create:

atx_net_create
================

Synopsis
--------

.. c:function:: atx_net atx_net_create(const atx_net_callbacks *, void *)

   Initialize the LinQ Network communication library. Optionally pass in a struct with function pointers to receive notifications of network events. Also optionally pass a context that will be passed to your applications callback function during linq network events.

   :param const atx_net_callbacks*: struct of function pointers to process linq network events. 
   :param void*: User data passed to each callback function.

Examples
--------

.. rubric:: With Callbacks

.. code-block:: c

   // Declare some callbacks
   static void on_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial);
   static void on_alert(void* ctx, atx_net_alert_s* alert, atx_net_email_s* mail, device_s** d);
   static void on_heartbeat(void* ctx, const char* serial, device_s** d);
   static void on_ctrlc(void* ctx);

   atx_net_callbacks callbacks = { .err = on_error,
                                     .alert = on_alert,
                                     .hb = on_heartbeat,
                                     .ctrc = on_ctrlc };
   int main(int argc, char *argv[])
   {
     // Create some app specific thing you need!
     app_context* app = app_context_create();

     // Create some linq networking
     atx_net* netw = atx_net_create(callbacks, app);

     // Process Network IO
     while (sys_running()) atx_net_poll(netw, 5);

     // All done ...
     atx_net_destroy(&netw);
   }


.. rubric:: Without Callbacks

.. code-block:: c

   int main(int argc, char *argv[])
   {
      atx_net* netw = atx_net_create(NULL, NULL);

     // Process Network IO
     while (sys_running()) atx_net_poll(netw, 5);

      // All done ...
      atx_net_destroy(&netw);
   }

.. rubric:: See Also

:ref:`atx_net_destroy <ref_api_c_atx_net_destroy>`

:ref:`atx_net_poll<ref_api_c_atx_net_poll>`
