const version = require(`${__dirname}/../package.json`).version;

const vsplit = version.split("-");
const candidate = vsplit[1] || "";
const csplit = candidate.split["."];
const tripple = vsplit[0].split(".");
const arg = process.argv[2];

if (arg === "major") {
  process.stdout.write(tripple[0]);
} else if (arg === "minor") {
  process.stdout.write(tripple[1]);
} else if (arg === "patch") {
  process.stdout.write(tripple[2]);
} else if (arg === "candidate") {
  process.stdout.write(candidate);
} else {
  process.stdout.write(version);
}
