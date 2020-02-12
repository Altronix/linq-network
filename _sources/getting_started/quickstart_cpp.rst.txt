Quick Start (C++)
=================

Project Setup
-------------

This example project setup uses CMake to build your project.  In order to use system dependencies you must first install linq-network and it's dependencies into your system.  For example on how to compile and install linq-network and it's dependencies, see :ref:`ref_getting_started_configuration`

CMakeLists.txt with system depenencies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*CMakeLists.txt*

.. code-block:: cmake

   add_executable(server main.cpp)
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
   add_executable(server main.cpp)
   target_link_libraries(server linq-netw)
   install(TARGETS server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

*Build application*

.. code-block:: shell

   mkdir build
   cd build
   cmake -DCMAKE_INSTALL_PREFIX=./install ..
   make
   make install

Process Network Events *(Optional)*
-----------------------------------

The LinQ Network C++ binding uses a lambda style API to listen for incoming network events.

Alert
~~~~~

The "Alert Callback" is called from the LinQ Network library when a LinQ Enabled device generates an alert. An optional application context, :ref:`alert struct <ref_api_c_linq_network_alert>`, and a :ref:`email struct <ref_api_c_linq_network_email>` are passed into your application alert callback function.

.. code-block:: cpp 

   std::function<void(linq_network_alert_s*, linq_network_email_s*, Device&)>;

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_network_alert`

   2. :ref:`ref_api_c_linq_network_email`


Heartbeat
~~~~~~~~~

The "Heartbeat Callback" is called from the LinQ Network library when a LinQ Enabled device sends a heartbeat. An optional application context, and the device serial number are passed into your application heartbeat callback function.

.. code-block:: cpp

   std::function<void(const char*, Device&)>;

Error
~~~~~

The "Error Callback" is called from the LinQ Network library when the LinQ Network library detects a runtime error. An optional application context, and an :ref:`ref_api_c_linq_network_error` are passed into your application error callback function.

.. code-block:: cpp

    std::function<void(E_LINQ_ERROR, const char*, const char*)> error_;

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_network_error`

Ctrlc
~~~~~

The "Ctrlc Callback" is called from the LinQ Network library when the LinQ Network library is about to shutdown. An optional application context is passed into your application ctrlc callback function.

.. code-block:: cpp

   std::function<void()> ctrlc_;

Process Network IO
------------------

The LinQ Network library performs non-blocking IO inside of your thread. To process LinQ Network IO you must call the :ref:`ref_api_c_linq_network_poll` routine from inside of your main loop.  linq_network_poll() will call any callbacks you have provided from the same thread context as your main application.

.. code-block:: c

   int
   main(int argc, char* argv[])
   {
      // Initialize your app
      altronix::linq linq{};

      while(sys_running())
      {
        linq.poll(5);
      }

      // ...
   }

.. note:: For multi threaded support, use one of the higher level language bindings that provide thread safe wrappers, or create your own IPC scheme per your applications needs.

.. rst-class:: font-small
.. container::

   **See Also**

   1. :ref:`ref_api_c_linq_network_poll`

   2. sys_running


Complete Application
--------------------

.. code-block:: cpp

   #include "altronix/linq_network.hpp"
   
   int
   main(int argc, char *argv[])
   {
     altronix::Linq netw{};
   
     netw.on_error([this](E_LINQ_ERROR error, const char *serial, const char *err) {
         // Received an error
       }).on_heartbeat([this](const char *serial, Device& device) {
         // Received a heartbeat
       }).on_alert([this](linq_alert_s* alert, linq_email_s* email, Device& device) {
         // Received an alert
       });
   
     while (true)
     {
       netw.poll(5);
     }
   }
