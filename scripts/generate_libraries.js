if (process.env["LINQ_NETWORK_SHARED"]) {
  // NOTE spaces required
  process.stdout.write("-llinqnetwork ");
  process.stdout.write("-lzmq ");
} else {
  // NOTE spaces required
  process.stdout.write("../build/install/lib/liblinqnetwork.a ");
  if (process.platform === "win32") {
    process.stdout.write("../build/install/lib/libzmq-v142-mt-s-4_3_4.lib");
  }
}
