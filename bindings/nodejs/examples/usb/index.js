let usbh = require("@altronix/linq-network").usbh;

// Demonstration using USB
// Note the M5 has hard coded serial number as "N/A" and vendor product
// strings as 3333, 4444 respectively (for now)

(async () => {
  let count = usbh.scan(3333, 4444);
  if (count) {
    let response = await usbh.send("N/A", "GET", "api/v1/network");
    console.log(response);
  } else {
    console.log("No LinQ device detected");
  }
})();
