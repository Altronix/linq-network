linq_netw_create
----------------

.. c:function:: linq_netw_create(const linq_netw_callbacks*, void*);

   Initialize the LinQ Network communication library. Optionally pass in a struct with function pointers to receive notifications of network events. Also optionally pass a context that will be passed to your applications callback function during linq network events.

   :param const linq_netw_callbacks*: struct of function pointers to process linq network events. 
   :param void*: User data passed to each callback function.

**Example with callbacks**

.. code-block:: c

   // Declare some callbacks
   static void on_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial);
   static void on_alert(void* ctx, linq_netw_alert_s* alert, linq_netw_email_s* mail, device_s** d);
   static void on_heartbeat(void* ctx, const char* serial, device_s** d);
   static void on_ctrlc(void* ctx);

   linq_netw_callbacks callbacks = { .err = on_error,
                                     .alert = on_alert,
                                     .hb = on_heartbeat,
                                     .ctrc = on_ctrlc };
   int main(int argc, char *argv[])
   {
     // Create some app specific thing you need!
     app_context* app = app_context_create();

     // Create some linq networking
     linq_netw* netw = linq_netw_create(callbacks, app);

     // Process Network IO
     while (sys_running()) linq_netw_poll(netw, 5);

     // All done ...
     linq_netw_destroy(&netw);
   }

**Example with out callbacks**

.. code-block:: c

   int main(int argc, char *argv[])
   {
      linq_netw* netw = linq_netw_create(NULL, NULL);

     // Process Network IO
     while (sys_running()) linq_netw_poll(netw, 5);

      // All done ...
      linq_netw_destroy(&netw);
   }

*See also*

:ref:`linq_netw_destroy <ref_api_c_linq_netw_destroy>`

:ref:`linq_netw_poll<ref_api_c_linq_netw_poll>`
