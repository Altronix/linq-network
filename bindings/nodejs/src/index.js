var binding = require("bindings")("linq-netw-js");
var linq_network = new binding.LinqNetwork();
console.log(linq_network.version());
process.stdin.resume();
