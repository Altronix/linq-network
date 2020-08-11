// Boot WebpackDevServer ourself
// https://gist.github.com/michaelrambeau/b04f83ef16fc78feee09

let path = require('path'),
  logger = require('./logger'),
  utils = require('./utils'),
  fs = require('fs'),
  cp = require('child_process'),
  webpack = require('webpack'),
  webpackConfig = require('./webpack.dev.js'),
  WebpackDevServer = require('webpack-dev-server/lib/Server');

// Try and find a suitable atx.config.js file
async function loadAtxConfig() {
  let { root } = await utils.seekRoot('portal');
  return (
    (await requirePath(`${root}/atx.config.user.js`)) ||
    (await requirePath(`${root}/atx.config.js`))
  );
}

// Require a file located on path, return file or undefined
async function requirePath(path) {
  return await fs.promises
    .stat(path)
    .then(() => require(path))
    .catch(e => undefined);
}

// Read apiServer config, load docker container and configure devServer proxy
async function startApiServer(config) {
  if (!config.apiServer) return;
  if (config.apiServer.container) {
    logger.info(`Starting backend [PORT: ${config.apiServer.httpPort}]`);
    const name = `${config.session}.atxmon`;
    await utils.startAtxmonContainer({ name, ...config.apiServer });
  } else {
    logger.info(`Assuming API [PORT: ${config.apiServer.httpPort}] exists!`);
  }
  if (!config.devServer.proxy) config.devServer.proxy = {};
  config.devServer.historyApiFallback = true;
  Object.assign(config.devServer.proxy, {
    '/api/**': {
      target: `http://${config.host}:${config.apiServer.httpPort}`
    }
  });
}

async function startContainers(config) {
  if (!config.containers) return;
  config.containers.forEach(async container => {
    const { name, args, image } = container;
    logger.info(`Starting container [${name}]`);
    await utils.startContainer(name, image, args);
  });
}

// Read services config and attatch services to devProxy.
async function startServices(config) {
  if (!config.services) return;
  if (!config.devServer.proxy) config.devServer.proxy = {};
  config.devServer.historyApiFallback = true;
  Object.keys(config.services).forEach(key => {
    logger.info(`Starting service [${key}]`);
    Object.assign(config.devServer.proxy, { [`${key}`]: config.services[key] });
  });
}

// Stop apiServer container if it is running
async function stopApiServer(config) {
  if (config.apiServer && config.apiServer.container) {
    const name = `${config.session}.atxmon`;
    await utils.stopDockerContainer(name);
  }
}

// Start any k64 docker devices
async function startContainers(config) {
  let devices = config.devices || [];
  devices.forEach(async (c, idx) => {
    const dockerHost = config.dockerHost;
    const { zmtpPort, zmtpsPort } = config.apiServer;
    const { httpPort, httpsPort, image } = c;
    const name = `${config.session}.${image}.${idx}`;
    await utils.startDeviceContainer({
      name,
      image,
      dockerHost,
      httpPort,
      httpsPort,
      zmtpPort,
      zmtpsPort
    });
  });
}

// Stop any k64 docker devices
async function stopContainers(config) {
  let devices = config.devices || [];
  devices.forEach(async (c, idx) => {
    const name = `${config.session}.${c.image}.${idx}`;
    await utils.stopDockerContainer(name);
  });
}

(async () => {
  try {
    const atxConfig = await loadAtxConfig();

    await startApiServer(atxConfig); // TODO deprecate
    await startServices(atxConfig);
    await startContainers(atxConfig);
    webpackConfig.devServer = Object.assign(
      {},
      webpackConfig.devServer,
      atxConfig.devServer
    );

    let devServer = new WebpackDevServer(
      await webpack(webpackConfig),
      webpackConfig.devServer
    );
    let app = devServer.listen(
      atxConfig.devServer.port,
      atxConfig.devServer.host,
      e => {
        if (e) throw e;
      }
    );
    logger.info(
      `Dev Server: ${atxConfig.devServer.host}:${atxConfig.devServer.port}`
    );

    let shuttingDown = false;
    ['SIGINT', 'SIGTERM'].forEach(function(sig) {
      process.on(sig, async function() {
        if (!shuttingDown) {
          shuttingDown = true;
          devServer.close();
          await stopContainers(atxConfig);
          await stopApiServer(atxConfig);
        }
      });
    });
  } catch (e) {
    logger.error(e);
  }
})();
