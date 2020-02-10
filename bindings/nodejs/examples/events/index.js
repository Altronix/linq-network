let netw = require("@altronix/atx-net-js").default;

netw.listen("tcp://*:33455");
netw.listen("http://*:8000");
netw
  .on("heartbeat", sid =>
    netw
      .send(sid, "GET", "/ATX/about")
      .then(response => console.log("Response: %s", response))
      .catch(e => console.log(e))
  )
  .on("alert", (sid, alert_data) => {
    console.log("ALERT from [%s]", sid);
    console.log("ALERT data [%s]", alert_data.mesg);
  });
netw.run(50);
