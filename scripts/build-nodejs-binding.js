// -s --system Build system libraries
// -d --daemon Build with daemon

const cp = require("child_process");
const args = require("minimist")(process.argv.slice(2));
const env = Object.assign({}, process.env);
const logger = require("./logger");

// Helper for parsing enviorment variable
const isTrue = opt =>
  opt === "ON" ||
  opt === "on" ||
  opt === "On" ||
  opt === "TRUE" ||
  opt === "true" ||
  opt === "True" ||
  opt === "YES" ||
  opt === "yes" ||
  opt === "Yes" ||
  opt === 1;

// Build command string from user argument linqd option
const linqd = opt => `--CDBUILD_LINQD=${opt ? "ON" : "OFF"}`;

// Build command string from user argument system option
const system = opt =>
  opt
    ? `--CDUSE_SYSTEM_ZMQ=ON --CDUSE_SYSTEM_JSMN_WEB_TOKENS=ON`
    : `--CDUSE_SYSTEM_ZMQ=OFF --CDUSE_SYSTEM_JSMN_WEB_TOKENS=OFF`;

// Parse user options for linqd
const withDaemon =
  isTrue(process.env.LINQ_NETWORK_WITH_DAEMON) ||
  args._.indexOf("linqd") >= 0 ||
  !!args.d;

// Parse user options for system
const withSystem =
  isTrue(process.env.LINQ_NETWORK_USE_SYSTEM_DEPENDENCIES) ||
  args._.indexOf("system") >= 0 ||
  !!args.s;

// Generate cmake-js argument
const cmakeCmd = process.platform === "win32" ? `cmake-js.cmd` : `cmake-js`;
const cmakeArgs =
  `${system(withSystem)} ${linqd(withDaemon)} ` +
  `--CDCMAKE_INSTALL_PREFIX=./ --CDBUILD_SHARED=OFF --CDWITH_NODEJS_BINDING ` +
  `--CDCMAKE_BUILD_TYPE=Release build --target=install`;

// Call cmake-js with args
logger.log(
  "info",
  "Attempting to use your compiler to build linq-network and dependencies"
);
logger.info(`WITH_SYSTEM_DEPENDENCIES: ${withSystem}`);
logger.info(`WITH_DAEMON             : ${withDaemon}`);
const result = cp.spawnSync(cmakeCmd, cmakeArgs.split(" "), {
  env,
  stdio: "inherit"
});
if (result.status === 0) {
  logger.info("Binding built success!");
} else {
  logger.warn("Failed to build LINQ_NETWORK_JS Binding!!!");
  logger.warn(JSON.stringify(result));
  logger.info("Using pre-built binaries");
  logger.fatal("TODO");
  process.exit(result.status);
}
