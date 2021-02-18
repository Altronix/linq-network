// -s --system Build system libraries
// -d --daemon Build with daemon

const fs = require("fs");
let path = require("path");
const cp = require("child_process");
const args = require("minimist")(process.argv.slice(2));
const env = Object.assign({}, process.env);
const logger = require("./logger");

const root = __dirname + "/../";

// Helper for parsing enviorment variable
const isTrue = (opt) =>
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
const linqdOpt = (opt) => `${opt ? "--CDBUILD_LINQD=ON" : ""}`;
const usbhOpt = (opt) => `${opt ? "--CDBUILD_USBH=ON" : ""}`;
const disablePassOpt = (opt) => `${opt ? "--CDDISABLE_PASSWORD=ON" : ""}`;
const systemOpt = (opt) => (opt ? "" : `--CDBUILD_DEPENDENCIES=ON`);
const logOpt = (opt) => (opt ? `--CDLOG_LEVEL=${opt}` : ``);
const debugOpt = (opt) => `--CDCMAKE_BUILD_TYPE=${opt ? "Debug" : "Release"}`;

// Parse user options for linqd
const withLinqd = (json) =>
  isTrue(process.env.LINQ_NETWORK_WITH_LINQD) ||
  args._.indexOf("linqd") >= 0 ||
  (json && json.linqd) ||
  !!args.d;

const withLog = (json) =>
  process.env.LINQ_NETWORK_LOG_LEVEL || (json && json.log) || false;

const withUsbh = (json) =>
  isTrue(process.env.LINQ_NETWORK_WITH_USBH) ||
  args._.indexOf("usbh") >= 0 ||
  (json && json.usbh) ||
  false;

// Parse user options for system
const withSystem = (json) =>
  isTrue(process.env.LINQ_NETWORK_USE_SYSTEM_DEPENDENCIES) ||
  args._.indexOf("system") >= 0 ||
  !!args.s;

const withDisablePass = (json) =>
  isTrue(process.env.LINQ_NETWORK_DISABLE_PASSWORD) ||
  (json && json.disablePassword) ||
  false;

const withDebug = (json) =>
  isTrue(process.env.LINQ_NETWORK_DEBUG) || (json && json.debug) || false;

// Generate cmake-js argument
const cmakeCmd = process.platform === "win32" ? `cmake-js.cmd` : `cmake-js`;
const cmakeArgs = (json) =>
  `${systemOpt(withSystem(json))} ` +
  `${linqdOpt(withLinqd(json))} ` +
  `${usbhOpt(withUsbh(json))} ` +
  `${disablePassOpt(withDisablePass(json))} ` +
  `${logOpt(withLog(json))} ` +
  `${debugOpt(withDebug(json))} ` +
  `--CDCMAKE_INSTALL_PREFIX=./ --CDBUILD_SHARED=OFF ` +
  `--CDBUILD_APPS=OFF ` +
  `--CDWITH_NODEJS_BINDING ` +
  `build --target=install`;

// Find the prebuilt binary (Only used if native build fails)
const getPrebuilt = () =>
  process.platform === "win32"
    ? root + "./bindings/nodejs/prebuilds/win32-x64/linq.node"
    : root + "./bindings/nodejs/prebuilds/linux-x64/linq.node";

// Install prebuilt binary into a build folder
const installPrebuilt = () => {
  const prebuilt = getPrebuilt();
  if (!fs.existsSync(root + "./build")) fs.mkdirSync(root + "./build");
  logger.info(`Installing prebuilt binaries: ${prebuilt}`);
  fs.copyFileSync(prebuilt, root + "./build/linq.node");
};

// Attempt to build binaries using users compiler
const tryBuild = async (json) => {
  logger.info("Attempting build with your compiler");
  logger.info(`Settings: WITH_SYSTEM_DEPENDENCIES -> ${withSystem(json)}`);
  logger.info(`Settings: WITH_LINQD               -> ${withLinqd(json)}`);
  logger.info(`Settings: WITH_USBH                -> ${withUsbh(json)}`);
  logger.info(`Settings: DISABLE_PASSWORD         -> ${withDisablePass(json)}`);
  logger.info(`Settings: LOG_LEVEL                -> ${withLog(json)}`);
  logger.info(`Settings: LINQ_NETWORK_DEBUG       -> ${withDebug(json)}`);

  // Call cmake-js
  const result = cp.spawnSync(cmakeCmd, cmakeArgs(json).split(" "), {
    env,
    stdio: "inherit",
    shell: true
  });

  if (!result.status) {
    logger.info("Build Success!");
    logger.debug(JSON.stringify(result));
    process.exit(result.status);
  } else {
    logger.warn("Failed to build linq binding!");
    logger.warn(JSON.stringify(result));
    installPrebuilt();
    logger.info("Installed prebuilt binaries OK");
    process.exit(0);
  }
};

(async () => {
  const filename =
      (require.main && require.main.filename) ||
      (process.mainModule && process.mainModule.filename),
    start = path.join(path.dirname(filename), "../.."),
    count = 10;
  let json = await (async function seek(start, count) {
    try {
      let file = await fs.promises.readFile(path.join(start, "package.json"));
      return JSON.parse(file);
    } catch {
      return count ? seek(path.join(start, ".."), --count) : undefined;
    }
  })(start, count);
  if (json && (json = json["linq"])) logger.info("Loading JSON config");
  if (process.env.LINQ_NETWORK_USE_PREBUILT || (json && json.prebuilt)) {
    installPrebuilt();
    logger.info("Installed prebuilt binaries OK");
    process.exit(0);
  } else {
    tryBuild(json);
  }
})();
