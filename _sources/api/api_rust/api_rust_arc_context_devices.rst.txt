.. _ref_api_rust_arc_context_devices:

arc::Context::devices
=====================

Synopsis
--------

.. code-block:: rust

   pub fn devices(&self) -> HashMap<String, String>;

Return a Hash Map of devices connected to the LinQ Network instance. The map is keyed by the serial number of the device. The value is the product ID of the device.

Parameters
----------

None.

Examples
--------

.. rubric:: Create JSON object of type {[key]: string, product: string}

.. code-block:: rust

   let net = atx_net::arc::Context::new();

   let mut ret = "{".to_string();
   let sz =  net.devices().len();
   let mut n = 0;
   for (k, v) in net.devices().iter() {
       let end = if n < sz - 1 { "," } else { "" };
       n = n + 1;
       ret.push_str(&format!("\"{}\":{{\"product\":\"{}\"}}{}", k, v, end));
   }
   ret.push_str("}");

   // example) ret = {"B3445ED2CDr2AC33298CXdd443": "LINQ2"}
