exports = module.exports;

// https://github.com/dotnet/msbuild/issues/5726
exports.sanitizeEnv = function (envArg = process.env) {
  const env = { ...envArg };
  if (env.Path && env.PATH) {
    const PATH = [...env.Path.split(";"), ...env.Path.split(";")];
    env.PATH = [...new Set(PATH)].join(";");
    delete env.Path;
  }
  return env;
};

exports.sanitizePath = function (path) {
  return path.replace(" ", "\\ ");
};
