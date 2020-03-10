let fs = require("fs"),
  daemon = require("@altronix/linq-network-js").daemon,
  config = {
    zmtp: parseInt(process.env.ZMTP) || 33455,
    zmtps: parseInt(process.env.ZMTPS) || 33456,
    http: parseInt(process.env.HTTP) || 33485,
    https: parseInt(process.env.HTTPS) || 33486,
    cert: "./unsafe-cert.pem",
    key: "./unsafe-key.pem",
    db: "./db.sqlite"
  };

if (!config.key.length) console.log("TLS_KEY NOT FOUND!");
if (!config.cert.length) console.log("TLS_CERT NOT FOUND!");

daemon.start(config);
