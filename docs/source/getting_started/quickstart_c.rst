Quick Start (C)
===============

Project Setup
-------------

This example project setup uses CMake to build your project.  In order to use system dependencies you must first install linq-network and it's dependencies into your system.  For example on how to compile and install linq-network and it's dependencies, see :ref:`ref_getting_started_configuration`

CMakeLists.txt with system depenencies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*CMakeLists.txt*

.. code-block:: cmake

   add_executable(server main.c)
   target_link_libraries(server linq-netw)
   install(TARGETS server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

*Build application*

.. code-block:: shell

   mkdir build
   cd build
   cmake -DLINQ_USE_SYSTEM_DEPENDENCIES:BOOL=ON ..
   make
   make install


CMakeLists.txt for reproducible builds
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*Download dependencies*

.. code-block:: shell

   git submodule add https://bitbucket.org/Altronix/linq-network external/linq-network
   git submodule add https://github.com/zeromq/libzmq external/libzmq
   git submodule add https://github.com/zeromq/czmq external/czmq

*CMakeLists.txt*

.. code-block:: cmake

   add_subdirectory(external/linq-network)
   add_subdirectory(external/libzmq)
   add_subdirectory(external/czmq)
   add_executable(server main.c)
   target_link_libraries(server linq-netw)
   install(TARGETS server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

*Build application*

.. code-block:: shell

   mkdir build
   cd build
   cmake -DCMAKE_INSTALL_PREFIX=./install ..
   make
   make install

Create LinQ Network Callbacks *(Optional)*
-------------------------------------------

To initialize the LinQ Network library you may optionally pass in function pointers to callbacks that will be called during LinQ Network Events.


Alert Callback
~~~~~~~~~~~~~~

The "Alert Callback" is called from the LinQ Network library when a LinQ Enabled device generates an alert. An optional application context, :ref:`alert struct <ref_api_c_linq_netw_alert>`, and a :ref:`email struct <ref_api_c_linq_netw_email>` are passed into your application alert callback function.

.. code-block:: c

   static void
   on_alert(
       void* ctx,
       linq_netw_alert_s* alert,
       linq_netw_email_s* mail,
       device_s** d)
   {
     // ...
   }
   

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_netw_callbacks`

   2. :ref:`ref_api_c_linq_netw_alert`

   4. :ref:`ref_api_c_linq_netw_email`


Heartbeat Callback
~~~~~~~~~~~~~~~~~~

The "Heartbeat Callback" is called from the LinQ Network library when a LinQ Enabled device sends a heartbeat. An optional application context, and the device serial number are passed into your application heartbeat callback function.


.. code-block:: c

   static void
   on_heartbeat(void* ctx, const char* serial, device_s** d)
   {
      // ...
   }

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_netw_callbacks`

Error Callback
~~~~~~~~~~~~~~

The "Error Callback" is called from the LinQ Network library when the LinQ Network library detects a runtime error. An optional application context, and an :ref:`ref_api_c_linq_netw_error` are passed into your application error callback function.

.. code-block:: c

   static void
   on_error(void* ctx, E_LINQ_ERROR e, const char* what, const char* serial)
   {
     // ...
   }

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_netw_callbacks`

   2. :ref:`ref_api_c_linq_netw_error`

CTRLC Callback
~~~~~~~~~~~~~~

The "Ctrlc Callback" is called from the LinQ Network library when the LinQ Network library is about to shutdown. An optional application context is passed into your application ctrlc callback function.

.. code-block:: c

   static void
   on_ctrlc(void* ctx)
   {
     // ...
   }

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_netw_callbacks`

Process Network IO
------------------

Complete Application
--------------------

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
     //  ...
   }
   
   static void 
   on_alert(void* ctx, linq_netw_alert_s* alert, linq_netw_email_s* mail)
   {
     // ...
   }
   
   static void 
   on_heartbeat(void* ctx, const char *serial, device_s** d)
   {
     // ...
   }
   
