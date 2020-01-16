Reading entire event history
============================

.. http:get:: /ATX/alerts/all

   Return the entire log history from the peristant storage of the device. Note the JSON properties are a minimal set of alert meta data.  For additinal data referencing the alert you can map the alert name with the alert type to reference relavent alert meta-data.

   **Example request**:

   .. sourcecode:: http

      GET /ATX/alerts/all HTTP/1.0
      Host: 192.168.168.168
      Accept: application/json, text/javascript

   **Example response**:

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      {
         "log": [
           { "who": "system", "what": "log reset", "when": 0 },
           { "who": "admin", "what": "Power Supply ID", "when": 1519057709, "name": "psOff" },
           { "who": "admin", "what": "Power Supply ID", "when": 1519057713, "name": "psOn" },
           { "who": "admin", "what": "Power Supply ID", "when": 1519826227, "name": "psOff" },
           { "who": "admin", "what": "Power Supply ID", "when": 1519826485, "name": "psOn" },
           { "who": "system", "what": "test", "when": 1519840284, "name": "tmr" },
           { "who": "system", "what": "test", "when": 1519840292, "name": "tmr" },
           { "who": "system", "what": "test", "when": 1519840356, "name": "tmr" },
           { "who": "system", "what": "test", "when": 1519846949, "name": "tmr" },
           { "who": "system", "what": "test", "when": 1519846963, "name": "tmr" },
           { "who": "system", "what": "test", "when": 1519846967, "name": "tmr" },
           { "who": "system", "what": "test", "when": 1519846972, "name": "tmr" },
           { "who": "system", "what": "test", "when": 1519847044, "name": "tmr" },
           { "who": "system", "what": "test", "when": 1519847047, "name": "tmr" },
           { "who": "system", "what": "test", "when": 1519847067, "name": "tmr" },
           { "who": "system", "what": "test", "when": 1519847071, "name": "tmr" },
           { "who": "system", "what": "test", "when": 1519847075, "name": "battFail" },
           { "who": "system", "what": "test", "when": 1519847080, "name": "ilowR" },
           { "who": "system", "what": "test", "when": 1519847152, "name": "psOn" },
           { "who": "system", "what": "test", "when": 1519847160, "name": "psOff" },
           { "who": "system", "what": "test", "when": 1519847174, "name": "rOn" },
           { "who": "admin", "what": "Power Supply ID", "when": 1519847194, "name": "psOff" },
           { "who": "admin", "what": "Power Supply ID", "when": 1519847247, "name": "psOn" },
           { "who": "admin", "what": "Power Supply ID", "when": 1520445407, "name": "psOff" },
           { "who": "admin", "what": "Power Supply ID", "when": 1520445413, "name": "psOn" }
         ]
      }


   :>json string who: The name of the user who caused the reason for the alert
   :>json string what: The name of the port or power supply that is cause for alert
   :>json number when: The unix time stamp of when the alert was generated
   :>json string name: The alert [key] property used to map alert types to a human readable message
