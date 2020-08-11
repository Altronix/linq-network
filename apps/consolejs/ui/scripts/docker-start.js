// Note that Dockerfile exposts 33455 33456 8000 and 8001
// -p ${httpPort}:8000 -p ${httpsPort}:8001
// -p ${zmtpPort}:33455 -p ${zmtpsPort}:33456

let utils = require("./utils"),
  yargs = require("yargs").argv;

(async () => {
  let config = { ...yargs };
  let child = utils.startAtxmonContainer(config);
  await child;
})();
