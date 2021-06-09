# Hello World

Empirical allows you to compile your C++ to target

- running at the command line (e.g., native machine code) and
- running in the web browser (e.g., javascript assembly code).

This how-to aims to walk you through all the nitty gritty required to
successfully compile a simple "Hello World" for both targets.

## Install: Native C++ Compiler

In the Unix-verse (e.g., Linux / MacOS) commonly used compilers include
`gcc` and `clang`. From this point onwards, we assume that you're
working with `gcc`. Unless you really know what you're doing, you'll
want to have `gcc` installed. The good news is: you might already!

Bring up a terminal and try entering:

```shell
which gcc
```

If `which` spits out a path, then you have `gcc` installed! If `which`
says "not found," you'll need to go ahead and install `gcc`. For
Linux users, your package manager (e.g., `yum`, `apt`, etc.) is probably
the way to go about this. For MacOS users, you'll need to get Apple's
"Command Line Tools for Xcode." Either way, give it a quick web search
(e.g., "install gcc on [my operating system]") and there should be
plenty of how-to guides that walk you through step-by-step.

TODO Windows... Maybe you should try git for Windows (e.g.,
"GitBash")?

## Compile & Run: Command Line

Assuming you haven't already pulled down a clone of Empirical, let's
get your working environment all set.

```shell
git clone https://github.com/devosoft/Empirical
cd Empirical/examples/ProjectTemplate
```

Let's take a look at what we want to compile.

`source/native/project_name.cc`:

```cpp
// This is the main function for the NATIVE version of this project.

#include <iostream>

#include "emp/base/vector.hpp"
#include "emp/config/command_line.hpp"

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  std::cout << "Hello World!" << '\n';
}
```

This part is where Empirical source is brought in.

```cpp
#include "emp/base/vector.hpp"
#include "emp/config/command_line.hpp"
```

The main function uses Empirical's vector and argument parsing tools to
process command line options, but doesn't do anything with them. Then,
we print "Hello World!".

Let's compile!

```shell
make
```

If you `ls`, you should now see the executable `project_name` has been
created. Let's run!

```shell
./project_name
```

## Install: Web C++ Compiler

In order to compile for web, you'll need the [emscripten LLVM-to-Web
Compiler](https://emscripten.org). If you're a new user, you (probably)
don't have this set up so we'll walk you through step-by-step.

We aim for Empirical to track the current release of emscripten. As of
mid-February 2019, Empirical is compatible with emscripten's
contemporary release, version `v1.38.27`. Your best bet for compiling
with Empirical is to install the latest version of emscripten. However,
if you run into roadblocks later on and think it might be because of
breaking changes in emscripten that Empirical hasn't tracked, let us
know by posting on [our issue
tracker](https://github.com/devosoft/Empirical/issues)!

```shell
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
```

When you want to use the emscripten compiler, you'll want to hop over
to the `emsdk` directory and run

```shell
source ./emsdk_env.sh
```

in order to load emscripten's odds and ends into your `PATH`. You only
need to do this once per terminal session (e.g., the first time you want
to use emscripten in a terminal session).

## Compile & Run: Web Browser

Assuming your working directory is still
`Empirical/examples/ProjectTemplate` and you have loaded up emscripten
(e.g., `source ./emsdk_env.sh`), compiling for web is a snap!

Let's take a look at what we want to compile first, though.

`source/web/project_name-web.cc`:

```cpp
//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  doc << "<h1>Hello, world!</h1>";
}
```

The line

```cpp
#include "emp/web/web.hpp"
```

brings in Empirical's web tools, which provide a convenient interface
for C++ code to interact with browser-y bits like html and Javascript.

The line

```cpp
UI::Document doc("emp_base");
```

creates a persistent `UI::Document` object (e.g., outside the scope of
the `main` function) that hooks into the `"emp_base"` div in
`web/project_name.html`.

Then, in `main`, we write our message to the `"emp_base"` div (wrapped
in some html markup formatting... e.g., `<h1>` and `</h1>`).

You can find a more comprehensive explanation of the contents of this
*.cpp* file in our [Quick Start Guide for Web
Tools](3-WebTools).

Let's compile

```shell
make web
```

We should now have `web/project_name.js` and `web/project_name.js.mem`
ready to go. You can verify this by entering `ls web` at your command
line.

We'll need to locally serve our working directory in order to view our
compiled product in a web browser. Python provides a handy, no-hassle
tool for this.

Try running

```shell
python3 -m http.server
```

at your command line. If it starts up, then great! Just leave it running
for now.

If you only have Python 2 installed, try running

```shell
python -m SimpleHTTPServer
```

at your command line.

If you don't have any Python installed, a step-by-step guide for your
operating system is probably only a quick web search away.
Alternatively, go ahead and use your web serving tool of choice.

Pop open your favorite browser and point the address bar to
<http://localhost:8000/web/project_name.html>.

Voila!

You can end your web serving process by closing the terminal window
you're working in or entering `<ctrl>-c` a the command line.

## Extending the Project Template

We've used the project template to run some simple "hello world" code
natively and in the browser.

If you're wondering how to extend the project template, we have a quick
start guide on exactly that [here](4-UsingProjectTemplate).
