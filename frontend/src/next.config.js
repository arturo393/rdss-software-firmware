const withTM = require("next-transpile-modules")(["react-konva", "konva"])

module.exports = withTM({
  eslint: {
    // Warning: Dangerously allow production builds to successfully complete even if
    // your project has ESLint errors.
    ignoreDuringBuilds: true,
  },
  images: {
    loader: "akamai",
  },
})
