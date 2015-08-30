# d3-emscripten
This repository contains the begginnings of a an emscripten wrapper for d3.js. Once it's done, it will let the user write code from C++ to create javascript visualizations in d3.js.

Dependencies:
* Emscripten (https://github.com/kripken/emscripten) - must be used as compiler for any of this to work
* Empirical (https://github.com/mercere99/Empirical) - provides JSWrap, for making callback functions not painful

Status:
Selection class is mostly functional - you can bind data, do transitions, and create interactions.

To use:
* Include "emp::Initialize();" at the start of your main function so JSWrap (and other tools from Empirical) will work
* Write callback functions in C++, and then call emp::JSWrap(functionname, "functionname") to make it accessible in both Javascript and C++. You can now pass the function name as an argument to functions that take functions.
