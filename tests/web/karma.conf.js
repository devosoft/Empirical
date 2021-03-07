module.exports = function(config) {
  config.set({

    // to enable commandline input
    client: {
      // args: config.name ? ["--name"] : [],
      // note that this works only with `karma start`, not `karma run`
      filename: config.filename,
    },

    // base path that will be used to resolve all patterns (eg. files, exclude)
    basePath: '../../',

    proxies: {
      "/assets/": "/base/tests/web/assets"
    },

    // frameworks to use
    // available frameworks: https://npmjs.org/browse/keyword/karma-adapter
    frameworks: ['mocha'],

    // list of files / patterns to load in the browser
    files: [
      {pattern: 'examples/web/jquery-1.11.2.min.js'},
      {pattern: 'third-party/node_modules/mocha/mocha.js'},
      {pattern: 'third-party/node_modules/chai/chai.js'},
      {pattern: 'include/emp/web/d3/d3.min.js'},
      {pattern: 'include/emp/web/d3/d3-tip.min.js'},
      'https://cdnjs.cloudflare.com/ajax/libs/highlight.js/10.0.0/styles/default.min.css',
      'https://cdnjs.cloudflare.com/ajax/libs/highlight.js/10.0.0/highlight.min.js',
      {pattern: 'include/emp/prefab/HighlightJS.js'},
      'https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.3/umd/popper.min.js',
      'https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta.2/css/bootstrap.min.css',
      'https://cdn.jsdelivr.net/gh/devosoft/Empirical@mastersource/fresh-prefab/DefaultPrefabStyles.less',
      'https://cdn.jsdelivr.net/npm/less',
      'https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta.2/js/bootstrap.min.js',
      {pattern: 'include/emp/prefab/LoadingModal.js'},
      // TODO: put all prefab dependencies in a directory and link it somehow
      // {pattern: 'tests/web/CodeBlock/*', included: false},
      {pattern: 'tests/web/assets/*', included: false},
      {pattern: `tests/web/${config.filename}.js.map`, included: false},
      {pattern: `tests/web/${config.filename}.js`},
      {pattern: `tests/web/${config.filename}.wasm`, included: false, nocache:true},
      {pattern: `tests/web/${config.filename}.wasm.map`, included: false, nocache:true}
    ],


    // list of files to exclude
    exclude: [
    ],


    // preprocess matching files before serving them to the browser
    // available preprocessors: https://npmjs.org/browse/keyword/karma-preprocessor
    preprocessors: {
    },


    // test results reporter to use
    // possible values: 'dots', 'progress'
    // available reporters: https://npmjs.org/browse/keyword/karma-reporter
    reporters: ['spec'],


    // web server port
    port: 9876,


    // enable / disable colors in the output (reporters and logs)
    colors: true,


    // level of logging
    // possible values: config.LOG_DISABLE || config.LOG_ERROR || config.LOG_WARN || config.LOG_INFO || config.LOG_DEBUG
    logLevel: config.LOG_DEBUG,


    // enable / disable watching file and executing tests whenever any file changes
    autoWatch: false,


    // start these browsers
    // available browser launchers: https://npmjs.org/browse/keyword/karma-launcher
    browsers: ['Firefox'],


    // Continuous Integration mode
    // if true, Karma captures browsers, runs the tests and exits
    // set this to false to keep the browser window open to interactively debug
    singleRun: true,

    // Concurrency level
    // how many browser should be started simultaneous
    concurrency: Infinity
  })
}
