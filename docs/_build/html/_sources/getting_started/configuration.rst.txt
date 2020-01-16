.. _ref_getting_started_configuration:

Library Configuration Options
=============================

The LinQ Network Library has compile options to customize your build for your particular enviorment and or needs.  Below is a list of configuration options.

1. **LINQ_LOG_LEVEL:STRING** *Default INFO*

   The LinQ Network Library supports traditional logging levels to control how verbose LinQ Network Library will behave while running. LINQ_LOG_LEVEL supports the following options.

   ====== ===========
   Option Description
   ====== ===========
   TRACE  Each function call will produce a time stamped output that is useful for extensive debugging
   DEBUG  Important events that warrent developer interest but not normal user concern will be reported
   WARN   Important events that are not typically expected and may represent potential issue. (IE a device failed to respond to a request)
   INFO   Normal operation behaving as expected
   ERROR  Something very unexpected has happened, but the system is able to recover and continue operating as normal
   FATAL  Something detected during runtime is preventing the system from continuing. A FATAL message will be the last message before faulting
   NONE   No output to the console will ever be generated
   ====== ===========

   .. note:: A LINQ_LOG_LEVEL of higher verbosity will also include all the logging output from the lesser verbose options

2. **LINQ_USE_SYSTEM_DEPENDENCIES:BOOL** *Default OFF*
   
   In order to support reproducable builds, the LinQ Network Library can be configured to build and compile the LinQ Network dependencies locally to your workspace.  In order to support Embedded Linux distributions, the LinQ Network Library can be configured to use system resources to resolve LinQ Network dependencies. 

   LINQ_USE_SYSTEM_DEPENDENICES has 2 options

   ======= ===================
   Option  Description
   ======= ===================
   ON       Link with dependencies located in the system installation folders (IE /usr/lib)
   OFF      Build dependencies locally
   ======= ===================

3. **LINQ_BUILD_EXAMPLES** *Default OFF*

   Any example applications will be compiled as well.  LINQ_BUILD_EXAMPLES has 2 options

   ====== ============
   Option Description
   ====== ============
   ON     All examples will be compiled
   OFF    No examples will be compiled
   ====== ============

4. **BUILD_STATIC** *Default ON*

   Build the static library for your target application.

   ====== ============
   Option Description
   ====== ============
   ON     Build a static library
   OFF    Do not build a static library
   ====== ============

5. **BUILD_SHARED** *Default ON*

   Build the shared library for your target application.

   ====== ============
   Option Description
   ====== ============
   ON     Build a shared library
   OFF    Do not build a shared library
   ====== ============

6. **LINQ_DISABLE_SQLITE** *Default OFF*

   The LinQ library can optionally be configured with a database and a HTTP server that will service external applications and users. To disable HTTP and Database support. 

   ====== ===========
   Option Description
   ====== ===========
   OFF    Database and HTTP server will be enabled
   ON     Database and HTTP server will be disabled
   ====== ===========

   .. note:: By default the HTTP server will not be made available until the caller initializes the HTTP server.


7. **ENABLE_TESTING** *Default OFF*

   The LinQ Library has extensive testing driven by ctest and cmocka test framework.  Enable testing will compile the test applications needed by the ctest program.

   ====== ===========
   Option Description
   ====== ===========
   OFF    Do not build tests
   On     Build tests
   ====== ===========
