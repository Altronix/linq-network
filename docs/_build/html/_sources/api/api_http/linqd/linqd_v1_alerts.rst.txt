/api/v1/alerts
==============

.. http:get:: /api/v1/alerts

   Return an array of alerts. You can sort and or filter alerts based on a sort or search key query parameters. You can limit result with the count query parameter. You can loop through alerts starting at a specified index with the start query parameter.

   **Example request:**

   .. sourcecode:: http

      GET /api/v1/alerts?start=0&count=10%search=LINQ2 HTTP/1.0
      Host: 192.168.0.22
      Accept: application/json, text/javascript
      Content-Type: application/json

   **Example response:**

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      [
         {
            "alert_id": "UXAse3A22ddASx",
            "device_id": "AsrxFq_Gb39asADB92SsriSqQmf33PoMzzerRf_afe=",
            "who": "Admin",
            "what": "Port 0",
            "when": 1580601600,
            "siteId": "Netway8BT - Office",
            "mesg": "Door closed"
         },
         {
            "alert_id": "UXB33Affrs_344d23",
            "device_id": "4B_j7lZYk8UByAjU7vjyDB1Uam17wSzri5QMN11q_G8=",
            "who": "Admin",
            "what": "Port 0",
            "when": 1580601600,
            "siteId": "LINQ2 - Hallway",
            "mesg": "Power Supply Trouble"
         }
      ]

   :query string search: (optional) Return only keys that match this search
   :query number start: (optional) return alerts starting at a specific index
   :query number count: (optional) return a specified amount of devices
   :query string sort: (optional) sort result by a specific key
   :>json string alert_id: Unique identity of the alert
   :>json string device_id: Unique identity (serial number) of the device responsible for the alert
   :>json string who: Account holder responsible for the reason for alert
   :>json string what: Secondary ID describing associated with the alert
   :>json string when: unix time stamp of when the alert occured
   :>json string siteId: Location of the device responsible for the alert
   :>json string mesg: Human readable string describing the alert
