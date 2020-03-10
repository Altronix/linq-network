let daemon = require("@altronix/linq-network-js").daemon,
  config = {
    http: process.env.HTTP || 8000,
    https: process.env.HTTPS || 8001,
    zmtp: process.env.ZMTP || 33455,
    zmtps: process.env.ZMTPS || 33456,
    cert: process.env.TLS_CERT || "",
    key: process.env.TLS_KEY || ""
  };

console.log(
  "Starting linq-daemon-js\n" +
    "http: %s" +
    "https: %s" +
    "zmtp: %s" +
    "zmtps: %s" +
    "cert: %s" +
    "key: %s",
  config.http,
  config.https,
  config.zmtp,
  config.zmtps,
  config.cert,
  config.key
);

daemon.start(config);
