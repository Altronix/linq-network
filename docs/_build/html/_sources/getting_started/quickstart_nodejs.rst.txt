Quick Start (NodeJS)
====================

Project Setup
-------------

```bash
npm install @altronix/linq-network-js
```

package.json

.. code-block:: javascript

   {
     // ...
     "cmake-js": {
       "runtime":"node", // or electron or nw
       "runtimeVersion": "10.9.0",
       "arch": "x64",
     }
   }

Complete Application
--------------------

.. code-block:: javascript

   let netw = require("@altronix/linq-network-js").default;
   
   netw.listen("tcp://*:33455");
   netw.listen("http://*:8000");
   netw.on("heartbeat", sid =>
     netw
       .send(sid, "GET", "/ATX/about")
       .then(response => console.log(response))
       .catch(e => console.log(e))
   );
   netw.run(50);
