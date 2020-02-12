/api/v1/devices
===============

.. http:get:: /api/v1/devices

   Return a JSON object keyed with serial numbers of known LinQ devices.

   **Example request:**

   .. sourcecode:: http

      GET /api/v1/devices?start=0&count=2 HTTP/1.0
      Host: 192.168.0.22
      Accept: application/json, text/javascript
      Content-Type: application/json

   **Example response:**

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      {
        "4B_j7lZYk8UByAjU7vjyDB1Uam17wSzri5QMN11q_G8=": {
          "siteId": "LinQ2-Office",
          "product": "LINQ2",
          "prjVersion": "2.01.32",
          "mqxVersion": "4.2.0",
          "atxVersion": "2.02.01",
          "mac": "CC:7F:39:86:E4:DE",
          "sid": "4B_j7lZYk8UByAjU7vjyDB1Uam17wSzri5QMN11q_G8=",
          "lastSeen": 1580601600
        },
        "AsrxFq_Gb39asADB92SsriSqQmf33PoMzzerRf_afe=": {
          "siteId": "Netway8BT-Conference",
          "product": "Netway8BT",
          "prjVersion": "2.01.32",
          "mqxVersion": "4.2.0",
          "atxVersion": "2.02.01",
          "mac": "CC:7F:39:86:E4:DD",
          "sid": "AsrxFq_Gb39asADB92SsriSqQmf33PoMzzerRf_afe=",
          "lastSeen": 1580601600
        }
      }

   :query string serial: (optional) return only data for a single serial number device
   :query number start: (optional) return devices starting at a specific index
   :query number count: (optional) return a specified amount of devices
   :query string sort: (optional) sort result by a specific key
   :>json string siteId: Location of the device
   :>json string product: Product type key
   :>json string prjVersion: Project Version
   :>json string atxVersion: Common Version
   :>json string mqxVersion: OS Version
   :>json string mac: MAC address
   :>json string sid: Serial Number
   :>json number lastSeen: The last time LinQ Network has seen the device in unix
