const merge = require('webpack-merge');
const webpack = require('webpack');
const commonConfig = require('./webpack.common');
const { CleanWebpackPlugin } = require('clean-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const glob = require('glob');
const logger = require('./logger.js');

const pattern = process.env.PORTAL_WEBPACK_GLOB;
const search =
  pattern && pattern.length
    ? __dirname + `/../src/**/*/*${pattern}*.test.ts`
    : __dirname + `/../src/**/*.test.ts`;
const entry = glob.sync(search);

entry.forEach(e => logger.info(`TEST FILE: ${e}`));
logger.info(`Compiling tests, please wait...`);
module.exports = merge(commonConfig(), {
  mode: 'development',
  entry
});
