const helpers = require("./helpers");
const cp = require("child_process");
const fs = require("fs");

// Read arguments from command line
function processArgs(args = process.argv.slice(2)) {
  return {
    sourceDir: helpers.sanitizePath(args[0]),
    buildDir: helpers.sanitizePath(args[1]),
    installDir: helpers.sanitizePath(args[2]),
    bindingDir: helpers.sanitizePath(args[3]),
  };
}

// Start building
const { sourceDir, buildDir, installDir, bindingDir } = processArgs();
try {
  const addonDir = `dist/${
    process.platform === "win32" ? "win32-x64" : "linux-x64"
  }`;
  const configArgs = helpers.makeCmakeConfigArgs({
    sourceDir,
    buildDir,
    installDir,
  });
  const buildArgs = helpers.makeCmakeBuildArgs({ buildDir });
  helpers.spawn("cmake", configArgs);
  helpers.spawn("cmake", buildArgs);
  helpers.spawn("yarn", ["node-gyp", "configure"]);
  helpers.spawn("yarn", ["node-gyp", "build"]);
  helpers.spawn("yarn", ["tsc", "-p", bindingDir]);
  if (!fs.existsSync(addonDir)) fs.mkdirSync(addonDir);
  fs.copyFileSync("./package.json", "dist/package.json");
  fs.copyFileSync("./build/Release/linq.node", `dist/linq.node`);
  fs.copyFileSync("./build/Release/linq.node", `${addonDir}/linq.node`);
  fs.copyFileSync("./scripts/postinstall.js", "dist/postinstall.js");
} catch (e) {
  helpers.error(`${e}`);
  process.exit(-1);
}
