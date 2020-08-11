// utils.js

let path = require('path'),
  fs = require('fs'),
  cp = require('child_process'),
  logger = require('./logger');

exports = module.exports;

// Get location of our script
exports.moduleLocation = async function() {
  let filename =
    (require.main && require.main.filename) ||
    (process.mainModule && process.mainModule.filename);
  if (!filename) throw new Error('System not supported');
  return path.dirname(filename);
};

// Find our package.json
exports.seekRoot = async function(name = '') {
  const filename =
      (require.main && require.main.filename) ||
      (process.mainModule && process.mainModule.filename),
    start = path.join(path.dirname(filename), '..'),
    count = 10;
  return await (async function seek(start, count) {
    try {
      let file = await fs.promises.readFile(path.join(start, 'package.json'));
      let test = JSON.parse(file);
      if (name.length && test.name === name) {
        return { root: start, json: test };
      } else {
        return count ? seek(path.join(start, '..'), --count) : undefined;
      }
    } catch (e) {
      console.log(e);
      return count ? seek(path.join(start, '..'), --count) : undefined;
    }
  })(start, count);
};

exports.execute = function(cmd) {
  return new Promise((resolve, reject) => {
    cp.exec(cmd, (err, stdout, stderr) => {
      if (err) return reject(stderr);
      resolve(
        stdout
          .toString()
          .replace(/\r\n/g, ' ')
          .replace(/\n/g, ' ')
      );
    });
  });
};

exports.startDeviceContainer = async function(userConfig = {}) {
  // ip -4 addr show docker0 | grep -Po 'inet \K[\d.]+'
  let config = Object.assign(
    {},
    {
      name: 'Debug.Container',
      image: 'linq2',
      dockerHost: '172.17.0.1',
      zmtpPort: 33455,
      zmtpsPort: 33456
    },
    userConfig
  );
  const httpPort = config.httpPort ? `-p ${config.httpPort}:8080 ` : '';
  const httpsPort = config.httpsPort ? `-p ${config.httpsPort}:8081 ` : '';
  const cmd =
    `docker run -d ` +
    `--name=${config.name} ` +
    `${httpPort}` +
    `${httpsPort}` +
    `${config.image} ` +
    `-n 127.0.0.1:8080 ` +
    `-c ${config.dockerHost}:${config.zmtpPort} ` +
    `-s ${config.secure ? 1 : 0} ` +
    `-p 0`;
  logger.debug(`Running command [${cmd}]`);
  logger.info(`Booting container: ${config.name}`);
  return exports.execute(cmd);
};

exports.startAtxmonContainer = async function(userConfig = {}) {
  let config = Object.assign(
    {},
    {
      name: 'atxmon',
      httpPort: 8000,
      httpsPort: 8001,
      zmtpPort: 33455,
      zmtpsPort: 33456
    },
    userConfig
  );
  let args = (
    `run -d --name=${config.name} ` +
    `-p ${config.httpPort}:8000 ` +
    `-p ${config.httpsPort}:8001 ` +
    `-p ${config.zmtpPort}:33455 ` +
    `-p ${config.zmtpsPort}:33456 ` +
    `altronix/atxmon ` +
    `--httpPort 8000 ` +
    `--httpsPort 8001 ` +
    `--zmtpPort 33455 ` +
    `--zmtpsPort 33456}`
  ).split(' ');
  logger.debug(`Running command [${args}]`);
  logger.info(`Booting container: ${config.name}`);
  let shell = process.platform === 'win32' ? true : false;
  return new Promise(resolve => {
    cp.spawn('docker', args, {
      stdio: ['inherit', 'ignore', 'inherit'],
      shell
    }).on('exit', () => resolve());
  });
};

exports.stopDockerContainer = async function(name) {
  let kill = `kill ${name}`.split(' ');
  let rm = `rm ${name}`.split(' ');
  let shell = process.platform === 'win32' ? true : false;
  return new Promise(resolve => {
    logger.info(`Killing container ${name}`);
    cp.spawn('docker', kill, {
      stdio: ['inherit', 'ignore', 'inherit'],
      shell
    }).on('exit', () => {
      logger.info(`Removing container ${name}`);
      cp.spawn('docker', rm, {
        stdio: ['inherit', 'ignore', 'inherit'],
        shell
      }).on('exit', () => resolve());
    });
  });
};

exports.spawn = async function(...args) {
  return new Promise((resolve, reject) => {
    cp.spawn(...args).on('exit', () => resolve(0));
  });
};
