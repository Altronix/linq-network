const cp = require("child_process");
const fs = require("fs");
const path = require("path");

if (process.platform === "win32") {
  const cmd = path.join(__dirname, "install.ps1");
  const args = [cmd, "-A", "-p", "win32-x64"];
  cp.spawn("powershell.exe", args, {
    env: process.env,
    stdio: "inherit",
    shell: true,
  });
} else {
  const cmd = path.join(__dirname, "install.sh");
  const args = ["-A"];
  cp.spawn(cmd, args, { env: process.env, stdio: "inherit" });
}
