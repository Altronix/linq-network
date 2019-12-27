var binding = require("bindings")("linq-netw-js");
var linq_network = new binding.LinqNetwork();
linq_network.listen("tcp://*:33455");
process.stdin.resume();
