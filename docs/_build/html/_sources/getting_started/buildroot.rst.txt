.. include:: ../roles.rst

.. _ref-getting_started_buildroot:

Buildroot Enviorment
====================

Buildroot is used to build Embedded Linux Systems that can be cross compiled for unique hardware targets such as the Altronix LinQ Master module or the Microchip Sama5D27 SOM1 Module. Buildroot has it's own user manual and the Altronix LinQ Network User Guide is only meant as a companion guide. Users should read the Buildroot User Manual in its entirety, paying closer attention to the Section 8.5 - Building Out of tree :blue:`[1]`, Section 8.6 - Enviorment Variables :blue:`[2]`, Section 8.12 - Advanced Usage :blue:`[3]`, Section 9 - Project specific customization :blue:`[4]`, and Section III - developer guide :blue:`[5]`.

.. rst-class:: font-small
.. container::
   
   1. `Section 8.5 - Building Out of Tree <https://buildroot.org/downloads/manual/manual.html#_building_out_of_tree>`_
   
   2. `Section 8.6 - Enviorment Variables <https://buildroot.org/downloads/manual/manual.html#env-vars>`_
   
   3. `Section 8.12 - Advanced Usage <https://buildroot.org/downloads/manual/manual.html#_advanced_usage>`_
   
   4. `Section 8.6 - Project Specific Customization <https://buildroot.org/downloads/manual/manual.html#customize>`_
   
   5. `Section III - Developer Guide <https://buildroot.org/downloads/manual/manual.html#_developer_guide>`_

   6. `Buildroot User Manual PDF <https://buildroot.org/downloads/manual/manual.pdf>`_

Required Packages
-----------------

   *If you find any packages are missing, please add them to the list by submitting a PR.*

1. which

2. sed

3. make (version 3.81 or any later)

4. binutils

5. build-essential (only for Debian based systems)

6. gcc (version 4.8 or any later)

7. g++ (version 4.8 or any later)

8. bash

9. patch

10. gzip

11. bzip2

12. perl (version 5.8.7 or any later)

13. tar

14. cpio

15. unzip

16. rsync

17. file (must be in /usr/bin/file)

18. bc 

19. python (version 2.7 or any later) 

Optional Packages
------------------

1. asciidoc, version 8.6.3 or higher

2. w3m

3. python with the argparse module (automatically present in 2.7+ and 3.2+)

4. dblatex (required for the pdf manual only) 

5. graphviz to use graph-depends and <pkg>-graph-depends

6. python-matplotlib to use graph-build 

Altronix Buildroot Packages
---------------------------

Buildroot offers the BR2_EXTERNAL enviorment variable as a convenient option to store packages outside of the mainline buildroot repository.  Altronix uses this enviorment variable to extend buildroot with Altronix specific packages such as the LinQ Network library and application demos.  To get started with buildroot, clone the altronix buildroot repository and the buildroot-external-altronix submodule into your development workspace.

.. code-block:: shell

   git clone https://bitbucket.org/Altronix/buildroot-at91
   cd buildroot-at91
   git submodule update --init

You are now ready to configure and compile your embedded linux system.

Altronix Demo Configuration
---------------------------

From inside your buildroot-at91 workspace you can observe the following directory structure:

::

   buildroot-at91/
   |-- arch/
   |-- board/
   |-- boot/
   |-- configs/
   |-- dl/
   |-- docs/
   |-- package/ 
   |   |-- ...                                          <-- mainline buildroot packages
   |-- external/
   |   |-- buildroot-external-altronix/
   |   |   |-- board/
   |   |   |-- configs/
   |   |   |   |-- sama5d27_som1_ek_headless_defconfig  <-- Microchip Demo Config
   |   |   |   |-- sama5d27_som1_ek_altronix_defconfig  <-- Altronix Demo Config
   |   |   |   |-- ...
   |   |   |-- package/
   |   |   |   |-- ...                                  <-- Extra Microchip + Altronix Packages


In order to create your own Embedded Linux System you should start from a configuration that is most similar to your target hardware and software requirments.  To create. To explore the Altronix Demo Configuration, run the command from buildroot-at91 root directory:

.. code-block:: shell

   make menuconfig

You should see the following menu.

.. image:: ../_static/img/makemenu-config.jpg

From the initial menuconfig view, navigate to External -> Altronix to view the Altronix specific package options.

.. image:: ../_static/img/makemenu-config-external.jpg

Creating A Custom DefConfig
---------------------------

To create your own package configuration and save the configuration into the Altronix config folder. Run make menuconfig command. Make your changes and then select save.

.. note:: This will save your configuration to buildroot-at91/.config and therefore will not overwrite the configuration you started from.

After you have made all the changes to your Embedded Linux System, run the following command to save your new configuration.

.. code-block:: shell

   make savedefconfig BR2_DEFCONFIG=./external/buildroot-external-altronix/configs/sama5d27_som1_ek_my_new_altronix_defconfig

Flashing SD Card onto target hardware
-------------------------------------

*todo*

Adding Custom Package C/C++
-------------------------------

*todo*

Adding Custom Package Rust
------------------------------

*todo*

Adding Custom Package Go
----------------------------

*todo*
