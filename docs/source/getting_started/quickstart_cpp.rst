Quick Start (C++)
=================

TODO Describe configure C++ App with CMake


Project Setup
-------------

.. code-block:: cmake

   add_executable(server main.c)
   target_link_libraries(server linq-netw)
   install(TARGETS server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


Complete Application
--------------------

TODO Describe Source Code

.. code-block:: cpp

   #include "altronix/linq_netw.hpp"
   
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
