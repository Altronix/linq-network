let netw = require("@altronix/linq").network;

(async () =>
  await netw
    .listen("tcp://*:33455")
    .on("new", (serial, about) => console.log(about))
    .on("heartbeat", (sid) => console.log(`HEARTBEAT [${sid}]`))
    .on("error", (error) => console.log(`ERROR [${error}]`))
    .on("ctrlc", () => console.log(`EXIT`))
    .on("alert", (alert_data) => {
      console.log("ALERT from [%s]", alert_data.serial);
      console.log("ALERT who [%s]", alert_data.who);
      console.log("ALERT what [%s]", alert_data.what);
      console.log("ALERT where [%s]", alert_data.where);
      console.log("ALERT when [%s]", alert_data.when);
      console.log("ALERT mesg [%s]", alert_data.mesg);
    })
    .run(50))();
