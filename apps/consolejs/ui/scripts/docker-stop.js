let utils = require("./utils"),
  yargs = require("yargs").argv;

(async () => {
  let name = yargs.name || "atxmon";
  let child = utils.stopDockerContainer(name);
  await child;
})();
