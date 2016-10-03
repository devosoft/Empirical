// Karma configuration
// Generated on Mon Oct 03 2016 15:06:06 GMT-0400 (EDT)

module.exports = function(config) {
  config.set({

    // base path that will be used to resolve all patterns (eg. files, exclude)
    basePath: '../../',

    // frameworks to use
    // available frameworks: https://npmjs.org/browse/keyword/karma-adapter
    frameworks: ['mocha'],

    // list of files / patterns to load in the browser
    files: [
      {pattern: 'examples/web/jquery-1.11.2.min.js'},
      {pattern: 'third-party/node_modules/mocha/mocha.js'},
      {pattern: 'third-party/node_modules/chai/chai.js'},
      {pattern: 'web/d3/d3.min.js'},
      {pattern: 'web/d3/d3-tip.js'},
      {pattern: 'tests/web/test_header.js'},
      {pattern: 'tests/test-data/lineage-example.json', included: false},
      {pattern: 'tests/test-data/test-line-graph.csv', included: false},
      {pattern: 'tests/web/test_visualizations.js.map', included: false},
      {pattern: 'tests/web/test_visualizations.js'}
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
    logLevel: config.LOG_INFO,


    // enable / disable watching file and executing tests whenever any file changes
    autoWatch: false,


    // start these browsers
    // available browser launchers: https://npmjs.org/browse/keyword/karma-launcher
    browsers: ['Firefox'],


    // Continuous Integration mode
    // if true, Karma captures browsers, runs the tests and exits
    singleRun: true,

    // Concurrency level
    // how many browser should be started simultaneous
    concurrency: Infinity
  })
}
