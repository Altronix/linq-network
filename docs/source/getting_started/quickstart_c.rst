Quick Start (C)
===============

TODO describe configuring C App With CMake

Project Setup
-------------

CMakeLists.txt

.. code-block:: cmake

   add_executable(server main.c)
   target_link_libraries(server linq-netw)
   install(TARGETS server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

Complete Application
--------------------

TODO Describe source code

.. code-block:: c

   // Include the Altronix Header
   #include "altronix/linq_netw.h"
   
   // Declare your application callbacks
   static void on_error(void* ctx, E_LINQ_ERROR e, const char *sid, device_s**d);
   static void on_alert(void* ctx, linq_netw_alert_s* alert, linq_netw_email_s* mail);
   static void on_heartbeat(void* ctx, const char *serial, device_s** d);
   linq_netw_callbacks callbacks = {.err = on_error, .alert = on_alert, .hb = on_heartbeat};
   
   int
   main(int argc, char *argv[])
   {
     ((void)argc); // Not used
     ((void)argv); // Not used
   
     int err = -1;
     linq_netw_socket s;
   
     // Create a linq-network context
     linq_netw_s *network = linq_netw_create(&callbacks, NULL);
     if(!server) return err;
   
     // Listen for incoming device connections...
     s = linq_netw_listen(server, "tcp://*:33455");
   
     // Process Socket IO every 5ms
     while (sys_running())
     {
       linq_netw_poll(server, 5);
     }
   
     // Free linq-network context
     linq_netw_destroy(&server);
   }
   
   static void 
   on_error(void* ctx, E_LINQ_ERROR e, const char *sid, device_s**d)
   {
     // 
   }
   
   static void 
   on_alert(void* ctx, linq_netw_alert_s* alert, linq_netw_email_s* mail)
   {
     //
   }
   
   static void 
   on_heartbeat(void* ctx, const char *serial, device_s** d)
   {
     //
   }
   
