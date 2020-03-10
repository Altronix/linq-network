let fs = require("fs"),
  daemon = require("@altronix/linq-network-js").daemon,
  config = {
    http: parseInt(process.env.HTTP) || 8000,
    https: parseInt(process.env.HTTPS) || 8001,
    zmtp: parseInt(process.env.ZMTP) || 33455,
    zmtps: parseInt(process.env.ZMTPS) || 33456,
    cert: "./unsafe-cert.pem",
    key: "./unsafe-key.pem",
    db: "./db.sqlite"
  };

if (!config.key.length) console.log("TLS_KEY NOT FOUND!");
if (!config.cert.length) console.log("TLS_CERT NOT FOUND!");

daemon.start(config);
