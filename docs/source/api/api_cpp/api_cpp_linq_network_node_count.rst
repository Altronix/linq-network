.. _ref_api_cpp_linq_network_node_count:

node_count
==========

Synopsis
--------

Return the number of nodes connected to your LinQ Network context.

Parameters
----------

None.

Example
-------

.. code-block:: cpp

   altronix::AtxNet net{};

   std::cout << "There are " << net.node_count() << "nodes connected to LinQ Network\n";

.. rubric:: See Also

1. :ref:`ref_api_c_linq_network_node_count`
