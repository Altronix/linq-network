const merge = require('webpack-merge');
const webpack = require('webpack');
const commonConfig = require('./webpack.common');
const { CleanWebpackPlugin } = require('clean-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = merge(
  commonConfig(),
  { output: { publicPath: '/' } },
  {
    mode: 'development',
    entry: [__dirname + '/../src/app-root.ts'],
    plugins: [
      new CleanWebpackPlugin(),
      new HtmlWebpackPlugin({
        title: 'Demo',
        template: __dirname + '/../src/index.html'
      })
    ]
  }
);
