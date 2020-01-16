.. Linq Network User Guide documentation master file, created by
   sphinx-quickstart on Sun Jan 12 13:39:41 2020.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. rst-class:: align-center margin-bottom-20 
.. container::

  .. image:: _static/img/altronix_logo.png
     :scale: 150%
     :align: center

LinQ Network Documentation
==========================
   
.. image:: _static/img/login.jpg
   :width: 100%
   :align: center

About
-----

.. Note The badges won't compile when in PDF mode so comment we comment out
   when making a PDF. (Hopefully fix soon?)

.. .. image:: https://travis-ci.com/TomzBench/linq-network.svg?branch=master
..    :target: https://travis-ci.com/TomzBench/linq-network
.. .. image:: https://codecov.io/gh/TomzBench/linq-network/branch/master/graph/badge.svg
..    :target: https://codecov.io/gh/TomzBench/linq-network

The Altronix LinQ Network SDK provides libraries for interfacing with Altronix LinQ enabled products in C, C++, Rust, Go, Python and NodeJS languages.

Getting Help
------------

.. table::
   :align: left

   ========== ==================
   Sales      sales@altronix.com
   Developers thomas@altronix.com
   Chat       **TODO**
   ========== ==================


.. Note that when generating PDF we use maxdepth as 3 and when generating
   html we use maxdepth as 2, because the HTML generator doesn't seem to count
   the index.rst titles as depth, where the PDF generator does...
   (Hopefully fix soon?)

.. _ref-getting_started:

Getting Started
===============

.. toctree::
   :maxdepth: 2

   getting_started/windows.rst
   getting_started/linux.rst
   getting_started/buildroot.rst
   getting_started/configuration.rst
   getting_started/device_setup.rst
   getting_started/quickstart_c.rst
   getting_started/quickstart_cpp.rst
   getting_started/quickstart_rust.rst
   getting_started/quickstart_nodejs.rst

API Reference
=============

.. toctree::
   :maxdepth: 2

   api/api_c/api_c.rst
   api/api_http/api_http.rst

Index
=====
:ref:`genindex`
