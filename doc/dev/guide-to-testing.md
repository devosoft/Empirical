# Guide to Testing in Empirical

This document details how testing works in Empirical, both for writing
and understanding tests. Empirical makes use of the [Catch testing
framework](https://github.com/philsquared/Catch), the documentation of
which is available
[here](https://github.com/philsquared/Catch/blob/master/docs/Readme.md).

## Running Tests

In the root directory of Empirical, use the maketarget `test`, like so:

```shell
make test
```

The tests will compile and execute automatically, and you should see
output that looks something like this:

```shell
cd tests && make test
make[1]: Entering directory '/home/jgf/git/Empirical/tests'
g++ -std=c++11 test_driver.cc -o test.o
# Execute tests
./test.o
===============================================================================
All tests passed (562 assertions in 27 test cases)
```

If you wish to see detailed coverage data you can use the maketarget
`coverage`:

```shell
make coverage
```

Again, the tests will compile (this time with coverage flags) and
execute, generating coverage data. This data will be analyzed and
stuffed into helpful HTML files. You\'ll see output that initially looks
like the normal tests, followed by a lot of output, and then:

```
Overall coverage rate:
lines......: 81.7% (946 of 1158 lines)
functions..: 87.0% (463 of 532 functions)
```

The HTML info will give breakdowns on line-by-line coverage on each
file. It is highly recommended that you consult these to verify that
code is well covered. To view these files, open
[tests/html/index.html]{.title-ref} in your favorite browser.

## Writing Tests

It is required that contributions to the Empirical library have test
coverage. Though writing tests can be a complex task in some cases the
Catch testing framework is extremely easy to use.

In general the best way to understand how to write tests is to look at
the existing tests. I recommend skimming through `test_tools.cc` for an
overview.

If you are creating a new test file you will need to include the file
you\'ve made in the `test_driver.cc` file. That is, suppose you create a
file `test_potatoes.cc`. You will then need to edit `test_driver.cc` so
that it looks something like this:

```cpp
#define CATCH_CONFIG_MAIN
#include "../third-party/catch/single_include/catch.hpp"
#include "test_tools.cc"
#include "test_geometry.cc"
#include "test_scholar.cc"
#include "test_potatoes.cc"
```

To write a test case you simply use the `TEST_CASE` macro provided by
Catch:

```cpp
TEST_CASE("Test name goes here", "[test classification here]")
{
        // body of test
}
```

Within a test case you can use the `REQUIRE` macro like an assert, to
require certain conditions within the test:

```cpp
REQUIRE(1==1); // will pass, obviously
REQUIRE(1==0); // will fail, and Catch will complain
```

If a `REQUIRE` fails, Catch will expand it for you to show you what was
compared. Supposing we have a test case like the following:

```cpp
TEST_CASE("testing tests", "[demo]")
{
    bool a = false, b = true;
    REQUIRE(a == b);
}
```
It would execute like so:

```shell
demo.cc:4: FAILED:
REQUIRE( a == b )
with expansion:
false == true

===============================================================================
test cases: 1 | 1 failed
assertions: 1 | 1 failed
```

This allows for easier debugging of failed tests.

Catch provides several different frameworks for constructing test cases
which are detailed within [their
documentation](https://github.com/philsquared/Catch/blob/master/docs/tutorial.md).

## Running Tests with Docker
-------------------------

A [devosoft/empirical](https://hub.docker.com/r/devosoft/empirical) Docker image has been set up
to make recreating a development environment on your machine easier.
The first step is to download Docker. <https://docs.docker.com/get-docker/>

To download and run the Docker image, enter the following commands in the Docker terminal
``` bash
docker pull devosoft/empirical:latest
docker run --name emp-tests -dit devosoft/empirical:latest /sbin/init
docker exec -it emp-tests bash -l
```

To exit Docker containter shell
``` bash
CTRL+D
```

Commands to stop and start the Docker container
``` bash
docker start emp-tests
docker stop emp-tests
```

If you get `error: cannot open display: 99` when running Mocha web tests, try
```bash
Xvfb :99 -ac -screen 0 1024x768x8 &
export DISPLAY=:99
```

If you get an error prompting you to *check if server X is already running* after
entering `Xvfb :99 -ac -screen 0 1024x768x8 &`, try this to kill the process
``` bash
ps -ef | grep Xvfb
kill *pid*
```

**Note:** Instructions adapted from <https://andy-carter.com/blog/setting-up-travis-locally-with-docker-to-test-continuous-integration> and <https://github.com/karma-runner/karma-firefox-launcher/issues/93#issuecomment-519333245>
