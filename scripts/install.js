const cp = require("child_process");
const fs = require("fs");
const path = require("path");

if (process.platform === "win32") {
} else {
  const cmd = path.join(__dirname, "install.sh");
  const args = ["-A"];
  cp.spawn(cmd, args, { env: process.env, stdio: "inherit" });
}
