const cp = require("child_process");
const fs = require("fs");
const path = require("path");

if (process.platform === "win32") {
  const cmd = path.join(__dirname, "install.sh");
  const args = ["-A", "-a", "win32-x64"];
  cp.spawn(cmd, args, { env: process.env, stdio: "inherit" });
} else {
  const cmd = path.join(__dirname, "install.sh");
  const args = ["-A"];
  cp.spawn(cmd, args, { env: process.env, stdio: "inherit" });
}
