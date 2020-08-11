const RESET = '\x1b[0m';
const BOLD = '\x1b[1m';
const BOLD_OFF = '\x1b[21m';
const BLINK = '\x1b[5m';
const BLINK_OFF = '\x1b[25m';
const UNDERLINE = '\x1b[4m';
const UNDERLINE_OFF = '\x1b[24m';

const BLACK = '\x1b[30m';
const RED = '\x1b[31m';
const GREEN = '\x1b[32m';
const YELLOW = '\x1b[33m';
const BLUE = '\x1b[34m';
const MAGENTA = '\x1b[35m';
const CYAN = '\x1b[36m';
const WHITE = '\x1b[37m';
const DEFAULT = '\x1b[39m';
const GRAY = '\x1b[90m';
const LIGHT_RED = '\x1b[91m';
const LIGHT_GREEN = '\x1b[92m';
const LIGHT_YELLOW = '\x1b[93m';
const LIGHT_BLUE = '\x1b[94m';
const LIGHT_MAGENTA = '\x1b[95m';
const LIGHT_CYAN = '\x1b[96m';
const LIGHT_WHITE = '\x1b[97m';

const BACKGROUND_BLACK = '\x1b[40m';
const BACKGROUND_RED = '\x1b[41m';
const BACKGROUND_GREEN = '\x1b[42m';
const BACKGROUND_YELLOW = '\x1b[43m';
const BACKGROUND_BLUE = '\x1b[44m';
const BACKGROUND_MAGENTA = '\x1b[45m';
const BACKGROUND_CYAN = '\x1b[46m';
const BACKGROUND_WHITE = '\x1b[47m';
const BACKGROUND_DEFAULT = '\x1b[49m';
const BACKGROUND_LIGHT_GRAY = '\x1b[100m';
const BACKGROUND_LIGHT_RED = '\x1b[101m';
const BACKGROUND_LIGHT_GREEN = '\x1b[102m';
const BACKGROUND_LIGHT_YELLOW = '\x1b[103m';
const BACKGROUND_LIGHT_BLUE = '\x1b[104m';
const BACKGROUND_LIGHT_MAGENTA = '\x1b[105m';
const BACKGROUND_LIGHT_CYAN = '\x1b[106m';
const BACKGROUND_LIGHT_WHITE = '\x1b[107m';

const colorMap = {
  trace: `${LIGHT_BLUE}`,
  debug: `${CYAN}`,
  info: `${GREEN}`,
  warn: `${YELLOW}`,
  error: `${RED}`,
  fatal: `${MAGENTA}`
};

exports = module.exports;

exports.log = function(channel, message) {
  const now = `${CYAN}|SYS|`;
  const arrow = `${WHITE}=>`;
  const level = `${colorMap[channel]}${channel.slice(-5).toUpperCase()}`;
  const m = `${LIGHT_BLUE}${message}${RESET}`;
  console.log(`${now} ${arrow} ${level} ${m}`);
};

exports.traice = function(message) {
  this.log('trace', message);
};

exports.debug = function(message) {
  this.log('debug', message);
};

exports.info = function(message) {
  this.log('info', message);
};

exports.warn = function(message) {
  this.log('warn', message);
};

exports.error = function(message) {
  this.log('error', message);
};

exports.fatal = function(message) {
  this.log('fatal', message);
};
