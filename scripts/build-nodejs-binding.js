// -s --system Build system libraries
// -d --daemon Build with daemon

// Note cannot import from node_modules because it doesn't exist yet
const fs = require("fs");
const path = require("path");
const cp = require("child_process");
const { sanitizeEnv, sanitizePath } = require("./sanitize");
const __env = sanitizeEnv(process.env);
const logger = require("./logger");

const root = __dirname + "/../";

const spawnEnv = {
  env: __env,
  stdio: "inherit",
  shell: process.platform === "win32",
};

// Helper for parsing enviorment variable
function isTrue(opt) {
  return (
    opt === "ON" ||
    opt === "on" ||
    opt === "On" ||
    opt === "TRUE" ||
    opt === "true" ||
    opt === "True" ||
    opt === "YES" ||
    opt === "yes" ||
    opt === "Yes" ||
    opt === 1
  );
}

// Generate cmake arg for the USB config
function cmakeArgUsbh(json) {
  return isTrue(__env.LINQ_WITH_USBH) || (json && json.usbh) || false
    ? "BUILD_USBH=ON"
    : "BUILD_USBH=OFF";
}

// Generate CMake arg for the disable password config
function cmakeArgDisablePassword(json) {
  return isTrue(__env.LINQ_DISABLE_PASSWORD) ||
    (json && json.disablePassword) ||
    false
    ? "DISALBE_PASSWORD=ON"
    : "DISABLE_PASSWORD=OFF";
}

// Generate cmake arg for the log level config
function cmakeArgLogLevel(json) {
  const level = __env.LINQ_LOG_LEVEL || (json && json.log) || "INFO";
  return `LOG_LEVEL=${level}`;
}

// Generate CMake arg for the debug config
function cmakeArgDebug(json) {
  return isTrue(__env.LINQ_DEBUG) || (json && json.debug) || false
    ? "CMAKE_BUILD_TYPE=Debug"
    : "CMAKE_BUILD_TYPE=Release";
}

// Generate entire cmake command
function cmakeArgs({ sourceDir, buildDir, installDir, config }) {
  return [
    `-S${sanitizePath(sourceDir)}`,
    `-B${sanitizePath(buildDir)}`,
    `-D${cmakeArgUsbh(config)}`,
    `-D${cmakeArgDisablePassword(config)}`,
    `-D${cmakeArgLogLevel(config)}`,
    `-D${cmakeArgDebug(config)}`,
    `-DBUILD_DEPENDENCIES=ON`,
    `-DCMAKE_INSTALL_PREFIX=${sanitizePath(installDir)}`,
    `-DBUILD_SHARED=OFF`,
    `-DBUILD_APPS=OFF`,
  ];
}

// Install prebuilt binary into a build folder
function installPrebuilt() {
  const buildDir = path.join(root, "build");
  const prebuilt =
    process.platform === "win32"
      ? path.join(root, "./bindings/nodejs/prebuilds/win32-x64/")
      : path.join(root, "./bindings/nodejs/prebuilds/linux-x64/");
  if (!fs.existsSync(buildDir)) fs.mkdirSync(buildDir);
  logger.info(`Installing prebuilt binaries: ${prebuilt}`);
  const linq = "linq.node";
  const zmq = "libzmq-v142-mt-4_3_4.dll";
  const czmq = "libczmq.dll";
  const prebuiltLinq = path.join(prebuilt, linq);
  const prebuiltZmq = path.join(prebuilt, zmq);
  const prebuildCzmq = path.join(prebuilt, czmq);
  copyFileSync(linq, path.join(buildDir, linq));
  if (fs.existsSync(prebuiltZmq)) {
    fs.copyFileSync(prebuiltZmq, path.join(buildDir, zmq));
    fs.copyFileSync(prebuiltCzmq, path.join(buildDir, czmq));
  }
}

// Attempt to build binaries using users compiler
function tryConfig(args) {
  logger.info(`Configuring: ${JSON.stringify(args)}`);
  const result = cp.spawnSync("cmake", args, spawnEnv);
  if (!(result.status === 0)) throw new Error("CMake failed to configure!");
}

function tryBuild(buildDir) {
  const args = [
    `--build`,
    `${sanitizePath(buildDir)}`,
    "--config",
    "Release",
    "--target",
    `install`,
  ];
  logger.info(`Building: ${args}`);
  const result = cp.spawnSync("cmake", args, spawnEnv);
  if (!(result.status === 0)) throw new Error("CMake failed to build!");
}

function seekRoot(start, count) {
  try {
    let file = fs.readFileSync(path.join(start, "package.json"));
    JSON.parse(file);
    return path.resolve(start);
  } catch {
    return count ? seekRoot(path.join(start, ".."), --count) : undefined;
  }
}

function readFileName() {
  return (
    (require.main && require.main.filename) ||
    (process.mainModule && process.mainModule.filename)
  );
}

(async () => {
  const filename = readFileName(),
    start = path.join(path.dirname(filename), ".."),
    sourceDir = seekRoot(start, 10),
    buildDir = path.join(sourceDir, "build-linq"),
    installDir = path.join(buildDir, "install");
  let json = JSON.parse(fs.readFileSync(path.join(sourceDir, "package.json")));
  if (json && (json = json["linq"])) logger.info("Loading JSON config");
  if (__env.LINQ_USE_PREBUILT || (json && json.prebuilt)) {
    installPrebuilt();
    logger.info("Installed prebuilt binaries OK");
    process.exit(0);
  } else {
    try {
      tryConfig(cmakeArgs({ sourceDir, buildDir, installDir, config: json }));
      tryBuild(buildDir);
    } catch (e) {
      logger.error(`${e} ...Installing prebuilt`);
      installPrebuilt();
      logger.info("Installed prebuilt binaries OK");
      process.exit(0);
    }
  }
})();
