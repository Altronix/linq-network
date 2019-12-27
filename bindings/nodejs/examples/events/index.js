let LinqNetwork = require("linq-network-js").LinqNetwork;
let netw = new LinqNetwork();

netw.listen("tcp://*:33455");
netw.on("heartbeat", sid => console.log(sid));
netw.run(50);
