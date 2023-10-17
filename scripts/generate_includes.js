// Buildroot will export the staging dir so gyp can get the include directory
if (process.env["LINQ_NETWORK_BUILDROOT"]) {
  process.stdout.write(`$(STAGING_DIR)/usr/include `);
  process.stdout.write(`$(STAGING_DIR)/usr/include/altronix `);
} else {
  process.stdout.write("./build/install/include ");
  process.stdout.write("./build/install/include/altronix ");
}
