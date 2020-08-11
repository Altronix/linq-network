const autoprefixer = require('autoprefixer');
const { resolve } = require('path');

module.exports = () => {
  return {
    output: { filename: 'bundle.js' },
    resolve: { extensions: ['.js', '.scss', '.ts'] },
    module: {
      rules: [
        {
          test: /\.scss$/,
          use: [
            { loader: 'css-loader', options: { sourceMap: true } },
            {
              loader: 'postcss-loader',
              options: {
                plugins: () => [autoprefixer({ grid: false })],
                sourceMap: true
              }
            },
            {
              loader: 'sass-loader',
              options: {
                sourceMap: true,
                sassOptions: { includePaths: [__dirname + '/../node_modules'] }
              }
            }
          ]
        },
        {
          test: /\.(png|jpe?g|svg)$/,
          loader: 'url-loader',
          options: {
            name: 'assets/[name].[ext]'
          }
        },
        {
          test: /\.(eot|ttf|woff|otf|woff2)$/,
          loader: 'file-loader'
        },
        {
          test: /\.js$/,
          loader: 'babel-loader',
          query: {
            sourceType: 'unambiguous',
            presets: ['@babel/preset-env'],
            plugins: [
              '@babel/plugin-proposal-class-properties',
              '@babel/plugin-transform-runtime',
              [
                '@babel/plugin-proposal-decorators',
                { decoratorsBeforeExport: true }
              ]
            ]
          }
        },
        {
          test: /\.tsx?$/,
          loader: 'ts-loader'
        }
      ]
    }
  };
};
