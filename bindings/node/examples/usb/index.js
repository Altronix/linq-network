let netw = require("@altronix/linq").network;

// Demonstration using USB
// Note the M5 has hard coded serial number as "N/A" and vendor product
// strings as 3333, 4444 respectively (for now)

(async () => {
  let count = netw.scan();
  if (count) {
    let response = await netw.send(
      "linqm5serialnumberhere",
      "GET",
      "api/v1/network"
    );
    console.log(response);
  } else {
    console.log("No LinQ device detected");
  }
})();
