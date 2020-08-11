let path = require('path'),
  logger = require('./logger'),
  utils = require('./utils'),
  fs = require('fs'),
  cp = require('child_process');

(async () => {
  try {
    let { root } = await utils.seekRoot('portal'),
      args = process.argv.slice(2),
      env = Object.assign({}, process.env),
      shell = process.platform === 'win32' ? true : false;
    if (args.length) env.PORTAL_WEBPACK_GLOB = args[0];
    await utils.spawn(
      'webpack',
      ['--config', `${root}/scripts/webpack.test.js`],
      { stdio: 'inherit', shell, env }
    );
    await utils.spawn('karma', ['start'], { stdio: 'inherit', shell, env });
  } catch (e) {
    logger.error(e);
  }
})();
