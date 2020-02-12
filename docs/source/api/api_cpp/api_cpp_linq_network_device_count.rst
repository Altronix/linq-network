.. _ref_api_cpp_linq_network_device_count:

device_count
============

Synopsis
--------

Return the number of devices connected to your LinQ Network context.

Parameters
----------

None.

Example
-------

.. code-block:: cpp

   altronix::AtxNet net{};

   std::cout << "There are " << net.device_count() << "devices connected to LinQ Network\n";

.. rubric:: See Also

1. :ref:`ref_api_c_linq_network_device_count`
