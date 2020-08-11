const merge = require('webpack-merge');
const webpack = require('webpack');
const commonConfig = require('./webpack.common');

module.exports = merge(commonConfig(), {
  mode: 'development',
  entry: [__dirname + '/../src/app-root.ts']
});
