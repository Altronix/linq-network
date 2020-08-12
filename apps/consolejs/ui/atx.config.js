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
  apiServer: {
    httpPort: 3002,
    httpsPort: 3003,
    zmtpPort: 33455,
    zmtpsPort: 33456,
    cert: false, // TODO
    key: false, // TODO
    container: true
  },
  /* Device IP address (Proxy or container) */
  devices: [
    {
      image: 'linq2',
      host: '127.0.0.1',
      httpPort: 44000,
      httpsPort: 44001,
      container: true
    },
    {
      image: 'linq2',
      host: '127.0.0.1',
      httpPort: 44002,
      httpsPort: 44003,
      container: false
    }
  ]
};
