const helpers = require("./helpers");
const cp = require("child_process");
const fs = require("fs");

// linq-network dependency
function buildDependencies() {
  const config = helpers.cmakeConfigArgs({
    sourceDir: "./",
    buildDir: "./build",
    installDir: "./build/install",
  });
  const build = helpers.cmakeBuildArgs("./build");
  helpers.spawn("cmake", config);
  helpers.spawn("cmake", build);
}

// Build nodejs addon
function buildAddon() {
  helpers.spawn("node-gyp", ["configure"]);
  helpers.spawn("node-gyp", ["build"]);
}

// Build nodejs module
function buildTs() {
  helpers.spawn("./node_modules/.bin/tsc", ["-p", "./bindings/node/lib"]);
}

function install() {
  fs.mkdirSync("./dist");
  fs.mkdirSync("./dist/src");

  if (
    process.env["LINQ_NETWORK_BUILD_ADDON"] === "TRUE" ||
    process.env["LINQ_NETWORK_BUILD_ADDON"] === "True" ||
    process.env["LINQ_NETWORK_BUILD_ADDON"] === "true" ||
    process.env["LINQ_NETWORK_BUILD_ADDON"] === "1" ||
    process.env["LINQ_NETWORK_BUILD_ADDON"] === 1
  ) {
    if (
      process.env["LINQ_NETWORK_SHARED"] === "TRUE" ||
      process.env["LINQ_NETWORK_SHARED"] === "True" ||
      process.env["LINQ_NETWORK_SHARED"] === "true" ||
      process.env["LINQ_NETWORK_SHARED"] === "1" ||
      process.env["LINQ_NETWORK_SHARED"] === 1
    ) {
      helpers.info("Shared libraries selected...");
      helpers.info("Skipping build dependencies...");
    } else {
      helpers.info("Building dependencies...");
      buildDependencies();
    }
    helpers.info("Building NodeJS Addon...");
    buildAddon();
    fs.copyFileSync("./build/Release/linq.node", `dist/src/linq.node`);
  } else {
    helpers.info("Using prebuilt binaries...");
    // TODO fetch prebuilts
  }

  helpers.info("Building NodeJS module...");
  buildTs();
}

helpers.info("linq-network environment vars:");
Object.keys(process.env)
  .filter((key) => key.toLowerCase().startsWith("linq_network"))
  .forEach((k) => helpers.info(`${k} = ${process.env[k]}`));

install();
helpers.info("Complete!");
