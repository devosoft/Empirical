![Empirical Logo](/doc/EmpiricalBanner.png)

Empirical is a library of tools for developing efficient, reliable, and accessible scientific
software.  The provided code is header-only and encapsulated into the emp namespace, so it
is simple to incorporate into existing projects.

[![CI](https://github.com/devosoft/Empirical/workflows/CI/badge.svg)](https://github.com/devosoft/Empirical/actions?query=workflow%3ACI+branch%3Amaster) [![Documentation Status](https://readthedocs.org/projects/empirical/badge/?version=latest)](https://empirical.readthedocs.io/en/latest/?badge=latest) [![DOI](https://zenodo.org/badge/24824563.svg)](https://zenodo.org/badge/latestdoi/24824563) [![codecov](https://codecov.io/gh/devosoft/Empirical/branch/master/graph/badge.svg)](https://codecov.io/gh/devosoft/Empirical)
[![DockerHub](https://img.shields.io/badge/DockerHub-Hosted-blue)](https://hub.docker.com/r/devosoft/empirical)
![Documentation Coverage](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fdevosoft%2FEmpirical%2Fgh-storage%2Fstats%2Fdoc-coverage.json)

See our [Built With Empirical Gallery](https://empirical.readthedocs.io/en/latest/BuiltWithEmpiricalGallery) for examples of web tools built with Empirical.

See our [Quick Start Guides](https://empirical.readthedocs.io/en/latest/QuickStartGuides) to start using the library.

Starting out on a new project that will use Empirical?
Take a look at the [cookiecutter Empirical project template](https://github.com/devosoft/cookiecutter-empirical-project) for automatically setting up a directory structure with all the files you need to get started -- step-by-step instructions included!

Tools in Empirical include:

* Web-enabled elements to facilitate compiling to JavaScript and with a full web interfaces,
  using Mozilla's Emscripten compiler (from C++ to high-efficiency JavaScript).
* Debug tools to facilitate audited memory management; these tools are turned off outside of
  debug mode allowing the full speed of raw pointers and normal standard library components.
* A wide range of Helper tools to streamline common scientific computing tasks such as
  configuration, data management, mathematical manipulations, etc.
  All of these are designed to be  easy-to-use and efficient.
* A powerful set of evolution tools for building Artificial Life or Evolutionary Computation
  software.
* A signal-action system that allows for efficient customization of tools, and flexible
  software that can easily facilitate plug-in functionality.

See <https://empirical.readthedocs.io/en/latest> for more detailed documentation
on the available source files.

# Directory Structure

| Folder       | Description
| ------------ | ----
| demos/       | examples of mini projects using Empirical
| doc/         | Documentation files (including for auto generation of docs)
| examples/    | Example code for many of the above tools (sub-directory structure parallels source/)
| include/     | Header files that make up the Empirical library (see below for details)
| Planning/    | Scattered notes about future development (most notes found in GitHub issue tracker)
| tests/       | Unit tests
| third-party/ | Non-empirical dependencies

# Sub-folders in include/emp directory

| Sub-Folder  | Description
| ----------- | ----
| base/       | Debug tools used throughout Empirical for fast and efficient memory management
| bits/       | Tools for conveniently and efficiently working with bit representations
| config/     | Tools to build a simple configuration system (including compile-time optimizations)
| control/    | Signaling tools allowing for fast and dynamic interaction across components
| compiler/   | Components for compiling code
| data/       | Tools for easy data management and output
| datastructs/| A variety of useful data structures and tools for using data structures
| debug/      | Tools to facilitate debugging (particularly of emscripten-generated web code)
| functional/ | Tools for handling functions, sets of functions, and memoization
| games/      | Implementations of simple games
| geometry/   | Geometry and physics-based tools for implementing simple virtual worlds
| hardware/   | Implementation of basic virtual hardware that is easy to configure
| io/         | Tools for handling input/output
| matching/   | Tools for tag-based matching
| math/       | Generally useful tools for math and randomness
| meta/       | Helpful tools for template meta-programming
| prefab/     | Prefabricated web elements to drop onto web pages
| scholar/    | Tools for tracking authors, citations, and bibliographies
| tools/      | Basic tools that are generally useful for scientific software
| web/        | Tools for building web interfaces (more low-level than in prefab)
| Evolve/     | Specialized tools for building Artificial Life or Evolutionary Computation projects
| in_progress/| Tools that are being worked on, but not at all ready for public consumption

# Development Status

Empirical is under heavy development.  As such, many source files have at least one of the
following statuses:

| Status | Meaning
| ------ | -------
| DESIGN | Notes are in place, but no (or minimal) working code
| EXPLORATORY | Some functionality may work, but needs re-engineering to get right
| ALPHA | Some basic functionality works, but more features still need to be added and tested
| BETA | Basic functionality is all in place, but needs more thorough testing.
| RELEASE | Well-tested functionality and used in multiple projects, at least by authors
| STABLE | Used by many non-authors in assorted projects without fixes for extended period
| ABANDONED | May have worked, or may not; no longer being developed
| BROKEN | Once worked (at least BETA level), but now needs to be repaired (not abandoned!)
| CLEANUP | At least BETA, but needs code cleanup, fixing of warnings, etc
| DEPRECATED | Functionality has been replaced and should shift to replacement.
| EXTERNAL | Part of another project (cited above); not developed as part of Empirical

# Levelization

The structure of Empirical is levelized to facilitate development and testing.  This means
that all directories are on a level where dependencies are either internal to that directory
or on lower levels.  Likewise, within a directory, files have a level and depend only on other
files at lower levels.

| Level | Folders
| ----  | ----
| 0 |  base/
| 1 |  meta/
| 2 |  tools/
| 3 |  config/  control/  data/  games/  geometry/  hardware/  scholar/
| 4 |  Evolve/ (data, control)  web/ (config, control)
