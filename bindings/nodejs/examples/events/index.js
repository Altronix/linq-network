let LinqNetwork = require("@altronix/linq-network-js").LinqNetwork;
let netw = new LinqNetwork();

netw.listen("tcp://*:33455");
netw.listen("http://*:8000");
netw.on("heartbeat", sid =>
  netw
    .send(sid, "GET", "/ATX/about")
    .then(response => console.log(response))
    .catch(e => console.log(e))
);
netw.run(50);
