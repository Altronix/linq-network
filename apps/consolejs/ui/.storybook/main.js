const merge = require('webpack-merge');
const ours = require('../scripts/webpack.storybook.js');

module.exports = {
  stories: ['../src/**/*.stories.[tj]s'],
  webpackFinal: async (config, { configType }) => {
    //
    // Replace file-loader with url-loader & babel-loader with our babel-loader
    //
    config.module.rules = config.module.rules.filter(obj => {
      let str = JSON.stringify(obj);
      return !(str.includes('file-loader') || str.includes('babel-loader'));
    });

    //
    // Merge storybook webpack with our webpack.
    //
    return merge([config, ours]);
  }
};
