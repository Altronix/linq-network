function forceGC() {
  if (global.gc) {
    global.gc();
  } else {
    console.warn("No GC hook! Start node with --expose-gc");
  }
}
var binding = require("bindings")("linq-netw-js");
var linq_network = new binding.LinqNetwork();
console.log(linq_network.version());
linq_network = null;
forceGC();
