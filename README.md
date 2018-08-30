Empirical is a library of tools for developing efficient, reliable, and accessible scientific
software.  The provided code is header-only and encapsulated into the emp namespace, so it
is simple to incorporate into existing projects.

[![Build Status](https://travis-ci.org/devosoft/Empirical.svg?branch=master)](https://travis-ci.org/devosoft/Empirical) [![Documentation Status](https://readthedocs.org/projects/empirical/badge/?version=latest)](https://empirical.readthedocs.io/en/latest/?badge=latest)

See the doc/QuickStartGuides/ folder to start using the library.

Tools in Empirical include:
* Web-enabled elements to facilitate compiling to JavaScript and with a full web interfaces,
  using Mozilla's Emscripten compiler (from C++ to high-efficiency JavaScript).
* Debug tools to facilitate audited memory management; these tools are turned off outside of
  debug mode allowing the full speed of raw pointers and normal standard library components.
* A wide range of Helper tools to streamline common scientific computing tasks such as
  configuration, data mangement, mathematical manipulations, etc.
  All of these are designed to be  easy-to-use and efficient.
* A powerful set of evolution tools for building Artificial Life or Evolutionary Computation
  software.
* A signal-action system that allows for efficient customization of tools, and flexible
  software that can easily facilitate plug-in functionality..

See https://empirical.readthedocs.io/en/latest for more detailed documentation
on the available source files.

# Directory Structure

| Folder       | Description
| ------------ | ----
| apps/        | mini projects using Empirical with broad utility
| doc/         | Documentation files (including for auto generation of docs)
| examples/    | Example code for many of the above tools (sub-directory structure parallels source/)
| Planning/    | Scattered notes about future development (most notes found in GitHub issue tracker)
| source/      | Header files that make up the Empirical library (see below for details)
| tests/       | Unit tests
| third-party/ | Non-empirical dependencies


# Sub-folders in source/ directory

| Sub-Folder  | Description
| ----------- | ----
| base/       | Debug tools used throughout Empirical for fast and efficient memory management
| config/     | Tools to build a simple configuration system (including compile-time optimizations)
| control/    | Signialing tools allowing for fast and dynamic interaction across components
| data/       | Tools for easy data management and output
| geometry/   | Geometry and physics-based tools for implementing simple virtual worlds
| hardware/   | Implementation of basic virtual hardware that is easy to configure
| meta/       | Helpful tools for template meta-programming
| scholar/    | Tools for tracking authors, citations, and bibliographies
| tools/      | Many basic tools that are generally useful for scientific software
| web/        | Tools for building web interfaces
| Evolve/        | Specialized tools for building Artificial Life or Evolutionary Computation projects
| in_progress/ | Tools that are being worked on, but not at all ready for public consumption


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
