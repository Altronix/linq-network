// -s --system Build system libraries
// -d --daemon Build with daemon

const fs = require("fs");
const cp = require("child_process");
const args = require("minimist")(process.argv.slice(2));
const env = Object.assign({}, process.env);
const logger = require("./logger");

const root = __dirname + "/../";

const jsonPackageAltronixConfig = (() => {
  let json = JSON.parse(fs.readFileSync(root + "./package.json"));
  return (
    json.linq_network || json.linq_network_js || json.altronix || undefined
  );
})();

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

// Find the prebuilt binary (Only used if native build fails)
const getPrebuilt = () =>
  process.platform === "win32"
    ? root + "./bindings/nodejs/prebuilds/win32-x64/linq-network.node"
    : root + "./bindings/nodejs/prebuilds/linux-x64/linq-network.node";

// Install prebuilt binary into a build folder
const installPrebuilt = () => {
  const prebuilt = getPrebuilt();
  if (!fs.existsSync(root + "./build")) fs.mkdirSync(root + "./build");
  logger.info(`Installing prebuilt binaries: ${prebuilt}`);
  fs.copyFileSync(prebuilt, root + "./build/linq-network.node");
};

// Attempt to build binaries using users compiler
const tryBuild = () => {
  logger.info("Attempting build with your compiler");
  logger.info(`Settings: WITH_SYSTEM_DEPENDENCIES -> ${withSystem}`);
  logger.info(`Settings: WITH_DAEMON              -> ${withDaemon}`);

  // Call cmake-js
  const result = cp.spawnSync(cmakeCmd, cmakeArgs.split(" "), {
    env,
    stdio: "inherit"
  });

  if (!result.error) {
    logger.info("Build Success!");
    process.exit(result.status);
  } else {
    logger.warn("Failed to build linq-network binding!");
    logger.warn(result.error);
    installPrebuilt();
    logger.info("Installed prebuilt binaries OK");
    process.exit(0);
  }
};

if (
  process.env.LINQ_NETWORK_USE_PREBUILT ||
  (jsonPackageAltronixConfig && jsonPackageAltronixConfig.prebuilt)
) {
  installPrebuilt();
  logger.info("Installed prebuilt binaries OK");
  process.exit(0);
} else {
  tryBuild();
}

tryBuild();
