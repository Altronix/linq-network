const fs = require("fs");
const sysDir = process.platform === "win32" ? "win32-x64" : "linux-x64";
fs.copyFileSync(`${sysDir}/linq.node`, "linq.node");
