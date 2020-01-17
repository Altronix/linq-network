Quick Start (NodeJS)
====================

The LinQ Network NodeJS binding uses cmake-js to compile the native module into your project. cmake-js supports nodejs runtime, electron runtime and nw runtime enviorments. To select your runtime enviorment, include a cmake-js option inside your package.json file.

Project Setup
-------------

.. code-block:: bash

   npm install @altronix/atx-net-js

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

Event Emitter
-------------

The LinQ Network NodeJS binding uses the event emitter to notify your application of incoming LinQ Network events.

*Event emitter keys*

========= ===========
Event     Description
========= ===========
heartbeat A device has sent a heartbeat
error     The library detected a runtime error
ctrlc     The LinQ Network is shutting down
alert     A device has sent an alert
========= ===========

Complete Application
--------------------

*Listen for incoming heartbeats and request the ABOUT data*

.. code-block:: javascript

   let netw = require("@altronix/atx-net-js").default;
   
   netw.listen("tcp://*:33455");
   netw.listen("http://*:8000");
   netw.on("heartbeat", sid =>
     netw
       .send(sid, "GET", "/ATX/about")
       .then(response => console.log(response))
       .catch(e => console.log(e))
   );
   netw.run(50);
