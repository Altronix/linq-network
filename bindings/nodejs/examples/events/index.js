let LinqNetwork = require("linq-network-js").LinqNetwork;
let netw = new LinqNetwork();
netw.listen("tcp://*:33455");

process.stdin.resume();
