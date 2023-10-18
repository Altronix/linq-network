if (process.env["LINQ_NETWORK_BUILDROOT"]) {
  // NOTE spaces required
  process.stdout.write("-llinqnetwork ");
  process.stdout.write("-lzmq ");
} else {
  // NOTE spaces required
  if (process.platform === "win32") {
    process.stdout.write("../build/install/lib/linqnetwork.lib ");
    process.stdout.write("../build/install/lib/libzmq-v143-mt-s-4_3_4.lib");
  } else {
    process.stdout.write("../build/install/lib/liblinqnetwork.a ");
    process.stdout.write("../build/install/lib/libzmq.a");
  }
}
