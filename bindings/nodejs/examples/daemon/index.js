let daemon = require("@altronix/linq-network-js").daemon,
  config = {
    http: process.env.HTTP || 8000,
    https: process.env.HTTPS || 8001,
    zmtp: process.env.ZMTP || 33455,
    zmtps: process.env.ZMTPS || 33456,
    cert: process.env.TLS_CERT || "",
    key: process.env.TLS_KEY || "",
    db: process.env.DATABASE || ""
  };

if (!config.key.length) console.log("TLS_KEY NOT FOUND!");
if (!config.cert.length) console.log("TLS_CERT NOT FOUND!");

daemon.start(config);
