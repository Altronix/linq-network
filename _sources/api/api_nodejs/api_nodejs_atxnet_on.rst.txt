.. _ref_api_nodejs_atxnet_on:

on
==

Synopsis
--------

.. code-block:: javascript

   on(type: string, ...): self

Parameters
----------

The caller will receive specific alert data type depending on the type of alert. For example, a "heartbeat" alert will provide a serial number string. And an "alert" event will provide a Json object keyed with the alert properties. See below for the alert types.

========= ======================= ====
Event     Description             Data
========= ======================= ====
heartbeat |heartbeat_description| .. code-block:: javascript

                                     serial: string
alert     |alert_description|     .. code-block:: javascript
                                     
                                     alertData: {
                                       serial: string,
                                       who: string,
                                       what: string,
                                       when: string,
                                       where: string,
                                       message: string,
                                     }
erro      |error_description|     .. code-block:: javascript

                                     error: string
========= ======================= ====

.. |heartbeat_description| replace:: 

   A LinQ Network Device will generate a heartbeat every 5-10 seconds when connected to a LinQ Network Service.

.. |heartbeat_data| replace:: 

   *serial: string*

.. |alert_description| replace::

   A LinQ Network Device will generate asyncrounous alerts dependent on external triggers detected by the LinQ Network Device. On such a trigger the "alert" event is generated.

.. |error_description| replace::

   The LinQ Network Library will generate an alert error when an exception occurs during runtime.

Example
-------

.. rubric:: Listen to heartbeats and alerts. Print serial numbers and alerts to console.

.. code-block:: javascript

   let netw = require("@altronix/linq-network-js").default;
   
   netw.listen("tcp://*:33455");
   netw
     .on("heartbeat", sid =>
       netw
         .send(sid, "GET", "/ATX/about")
         .then(response => console.log("Response: %s", response))
         .catch(e => console.log(e))
     )
     .on("alert", alert_data => {
       console.log("ALERT from [%s]", alert_data.serial);
       console.log("ALERT who [%s]", alert_data.who);
       console.log("ALERT what [%s]", alert_data.what);
       console.log("ALERT where [%s]", alert_data.where);
       console.log("ALERT when [%s]", alert_data.when);
       console.log("ALERT mesg [%s]", alert_data.mesg);
     });
   netw.run(50);

.. rubric:: See Also

