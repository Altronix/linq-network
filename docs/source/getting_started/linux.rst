.. _ref_getting_started_linux:

Linux
=====

Setting up your development enviorment for the Linux OS is used to build LinQ Networking Applications that run Linx Host machines as well as Embedded Linux Target systems. The Linux Host enviorment is also used to run the LinQ Network test framework.

Required Packages
-----------------

The following packages are required to get your Linux OS up and running.

   *If you find any packages are missing, please add them to the list by submitting a PR*

1. Git

2. clang and LLVM

3. gcc

4. lcov

5. `Docker Community <https://docs.docker.com/install>`_ (Recommended for development using hardware containers)

6. `Rust <https://rustup.rs>`_ (Recommended for building  the command line utilities and Rust bindings)

7. `NodeJS + NPM <https://nodejs.org>`_ (Recommended for building the NodeJS bindings)

8. `Go <https://golang.org>`_ (Recommended for building the Go bindings)

9. `Python <https://python.org>`_ (Recommended for building the Python bindings)

Installation
------------

The following terminal commands show how to build and install the linq-network library to the linq-network/build-linux/install directory.

.. code-block:: shell

   git clone https://bitbucket.org/Altronix/linq-network
   cd linq-network
   git submodule update --init
   mkdir build-linux
   cd build-linux
   cmake -DCMAKE_INSTALL_PREFIX=./install -DENABLE_TESTING={ON|OFF} -DCMAKE_BUILD_TYPE=MinSizeRel ..
   make
   make install

.. note:: For a list of configuration options, use cmake -LH command.  See a description of the configuration parameters here: :ref:`ref_getting_started_configuration`
