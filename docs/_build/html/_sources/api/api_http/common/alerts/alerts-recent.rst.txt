Most recent alerts
==================

.. http:get:: /ATX/alerts/recent

   Return a minimal set of the most recent alerts (Max 5)

   **Example request**:

   .. sourcecode:: http

      GET /ATX/alerts/recent HTTP/1.0
      Host: 192.168.168.168
      Accept: application/json, text/javascript

   **Example response**:

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      {
         "recent": [{
           "who": "system",
           "what": "Power Supply ID",
           "when": 1520445079,
           "name": "ihigh",
           "mesg": "Over current",
           "siteId": "Demo Laptop"
         }, {
           "who": "system",
           "what": "Power Supply ID",
           "when": 1520445079,
           "name": "ihigh",
           "mesg": "Over current",
           "siteId": "Demo Laptop"
         }, {
           "who": "admin",
           "what": "Power Supply ID",
           "when": 1520445407,
           "name": "psOff",
           "mesg": "Power supply off",
           "siteId": "Demo Laptop"
         }, {
           "who": "admin",
           "what": "Power Supply ID",
           "when": 1520445413,
           "name": "psOn",
           "mesg": "Power supply on",
           "siteId": "Demo Laptop"
         }]
      }


   :>json string who: The name of the user who caused the reason for the alert
   :>json string what: The name of the port or power supply that is cause for alert
   :>json number when: The unix time stamp of when the alert was generated
   :>json string name: The alert [key] property used to map alert types to a human readable message
   :>json string mesg: A human readable message describing the alert
   :>json string siteId: The location of where the device that caused the alert
