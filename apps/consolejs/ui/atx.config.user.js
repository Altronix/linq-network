module.exports = {
  /* Name of your development session (will show up as prefix of logs and etc */
  session: 'atx.config.default',
  /* Host IP address */
  host: '127.0.0.1',
  /* Host IP of your docker daemon */
  dockerHost: '172.17.0.1',
  /* WebpackDevServer config settings */
  devServer: {
    host: '127.0.0.1',
    port: 3008,
    open: true,
    overlay: true
  },
  /* Backend API server */
  apiServer: {},
  /* Device IP address (Proxy or container) */
  devices: []
};
