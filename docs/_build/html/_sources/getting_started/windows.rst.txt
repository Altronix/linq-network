.. _ref-getting_started_windows:

Windows
=======

Setting up your development enviorment for the Windows OS is used to build LinQ Networking Applications that run on Windows 10 operating system. The Windows Development enviorment does not support running the test framework and is only supported for application development. The Windows Development enviorment may be used to build embedded linux distributions but this is not supported. For Embedded Linux target installations, please refer to setting up your Linux host machine.

Required Packages
-----------------

The following packages are required to get your Windows OS up and running.

1. `Git <https://git-scm.com/download/win>`_

2. `Docker Community <https://docs.docker.com/docker-for-windows/install/>`_

   .. note:: Configure your docker to run Linux Containers per the Windows Docker documentation

   .. warning:: If docker does not support your Windows Installation well, try and use `Docker Toolbox <https://docs.docker.com/toolbox/toolbox_install_windows>`_

3. `Microsoft Visual Studio 2019 (Community Edition) <https://visualstudio.microsoft.com/vs/community>`_

4. `LLVM <https://releases.llvm.org/download.html>`_

5. `CMake <https://cmake.org/download>`_

6. `Rust <https://rustup.rs>`_ (Recommended for building command line utilities and Rust bindings)

Installation
------------

The following terminal commands give example for how to build and install the linq-network library to the linq-network/build-windows/install directory.

.. code-block:: bash

   git clone https://bitbucket.org/Altronix/linq-network
   git submodule update --init
   cd linq-network
   cmake -Bbuild-windows -S . -DCMAKE_ISNTALL_PREFIX=./build-windows/install
   cmake --build ./build-windows --target install --config MinSizeRel

.. warning:: 
   Windows does not support the CMAKE_BUILD_TYPE flag during configuration step.  To configure the BUILD_TYPE use the --config switch during the compilation step.

.. note:: For a list of configuration options, use cmake -LH command.  See a description of the configuration parameters here: :ref:`ref_getting_started_configuration`
