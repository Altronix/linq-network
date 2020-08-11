/* eslint-disable import/no-extraneous-dependencies */
const { createDefaultConfig } = require("@open-wc/testing-karma");
const merge = require("deepmerge");
const commonWebpackConfig = require("./scripts/webpack.common.js");

module.exports = (config) => {
  config.set(
    merge(createDefaultConfig(config), {
      files: [
        // runs all files ending with .test in the test folder,
        // can be overwritten by passing a --grep flag. examples:
        //
        // npm run test -- --grep test/foo/bar.test.js
        // npm run test -- --grep test/bar/*
        // {
        //   pattern: config.grep ? config.grep : 'src/**/*.test.ts'
        // }
        { pattern: "./src/__tests__/setup.js", type: "module" },
        "./dist/bundle.js",
      ],

      // preprocessors: {
      //   '**/*.js': ['webpack'],
      //   '**/*.test.js': ['webpack'],
      //   '**/*.ts': ['webpack'],
      //   '**/*.test.ts': ['webpack'],
      //   '**/*.scss': ['webpack']
      // },

      // webpack: commonWebpackConfig(),

      esm: {
        nodeResolve: true,
      },
      // you can overwrite/extend the config further
    })
  );
  return config;
};
