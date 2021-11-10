# Coding guidelines and review checklist

This document is for those who want to contribute code or documentation
fixes to the Empirical project and describes our coding standards as
well as our code review process.

This document has been adapted from the [khmer
project](https://khmer.readthedocs.org/en/v1.4.1/dev/coding-guidelines-and-review.html)

## C++ standards

We use C++17 features throughout the project and currently that is the
de-facto standard version to use.

All code should be in header files for ease of inclusion into Emscripten
projects.

Files that define a single class should be named after that class. Files
that define sets of functions or multiple classes should have an
all-lowercase name that describes its contents.

All files and all directories must be levelized. This is partly enforced
through all files being headerfiles (and thus we cannot have circular
dependencies), but for clean coding practices (and easy of unit testing)
whole directories should not refer to each other bidirectionally either.
See [Large-Scale C++ Software Design by John
Lakos](http://www.amazon.com/Large-Scale-Software-Design-John-Lakos/dp/0201633620/)
for a strong pro-levelization argument.

In-code identifier formatting is always hard to settle upon. The
guidelines below are for consistency.

> -   Variable names should be all_lowercase, with words separated by
>     underscores
> -   Function names should be CamelCase() unless they are meant to
>     mimic a function from the C++ standard library, at which point
>     they can be all_lowercase to fit in.
> -   User-defined types should be CamelCase
> -   Constants should be ALL_UPPERCASE, with words separated by
>     underscores
> -   Template parameters should be ALL_UPPERCASE.
> -   Typedefs or using statements should be all lowercase with an _t at
>     the end of the alaised type name.
>     For example, a typedef on a template parameter CONTAINER_T might be
>     container_t.

## Guidelines based on Emscripten Limitations

> -   Do not rely on exceptions when possible. Emscripten is slow at
>     dealing with them and they can slow down code even when not
>     triggered.  By default, we compile with exceptions disabled.
> -   Do not write multi-threaded code that uses shared state.
>     Javascript cannot (yet) handle such code and as such Emscripten
>     cannot compile it. Note that Emscripten does have experimental
>     support of pthreads.
> -   Obviously, do not use any architecture-specific tricks, such as
>     assuming endianness, doing unaligned reads or writes, directly
>     accessing registers, etc.

Please see the [Emscripten doc
page](https://kripken.github.io/emscripten-site/docs/porting/guidelines/portability_guidelines.html)
for a full list.

## Commenting in files

All code should be well-commented such that it can be understood by a skilled
C++ programmer that is not familiar with the code base.  Comments should be
written in Doxygen format where appropriate.

Each file should have a header at the top to describe the goals of that file.
This header should include copyright information as well as the name of the file,
a brief description, and its status.  Statuses include:

| Status | Meaning
| ------ | -------
| DESIGN | Notes are in place, and some functionality may work, but needs more engineering.
| ALPHA | Some basic functionality works, but more features still need to be added and tested
| BETA | Basic functionality is all in place, but needs more thorough testing.
| RELEASE | Well-tested functionality and used in multiple projects, at least by authors
| STABLE | Used by many non-authors in assorted projects without fixes for extended period
| BROKEN | Once worked (at least BETA level), but now needs to be repaired (not abandoned!)
| DEPRECATED | Functionality has been replaced and should shift to replacement.

An example header might look like:

```
/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020.
 *
 *  @file Ptr.hpp
 *  @brief A wrapper for pointers that does careful memory tracking (but only in debug mode).
 *  @note Status: BETA
 *
 *  Ptr objects behave as normal pointers under most conditions.  However, if a program is
 *  compiled with EMP_TRACK_MEM set, then these pointers perform extra tests to ensure that
 *  they point to valid memory and that memory is freed before pointers are released.
 *
 *  If you want to prevent pointers to pointers (a common source of errors, but MAY be done
 *  intentionally) you can define EMP_NO_PTR_TO_PTR
 *
 *  If you trip an assert, you can re-do the run a track a specific pointer by defining
 *  EMP_ABORT_PTR_NEW or EMP_ABORT_PTR_DELETE to the ID of the pointer in question.  This will
 *  allow you to track the pointer more easily in a debugger.
 *
 *  @todo Track information about emp::vector and emp::array objects to make sure we don't
 *    point directly into them? (A resize() could make such pointers invalid!) Or better, warn
 *    it vector memory could have moved.
 *  @todo Get working with threads
 */
 ```

Each class should have at least a one-sentence description of the goals of that
class (unless it is the ONLY class in a file and the descriptions would be identical).
Each function should have at least a one-sentence description; parameters and
return value should also be described unless obvious from the function/parameter names.

Sections of code should have a comment at the top, explaining what this section does.
More detailed comments are only needed if that section might be non-intuitive for an
outside programmer.  Detailed comment can be either in the form of a fuller explanation
at the top of a section, or line-by-line hand holding.

Comments should always focus on intention and reasoning, not merely restating
what the code is obviously doing.
Comments are especially criticial for bug fixes or for warnings in non-intuitive
code -- it is important to not just indicate what is now happening, but why
seemingly intuitive alternative methods are not correct, especially if it seems like it
would be simpler code.
Make sure to always FIX comments when you change code -- out of date comments
are far worse than no comments at all.


## General Standards

All plain-text files should have line widths of 100 characters or less
unless that is unsupported for the particular file format or creates a
major loss in readability.

All contributions should have their spelling checked before being
committed to the codebase.  For example, the VSCode plug-in
"Code Spell Checker" is a good choice.

Vim users can run:

```
    :setlocal spell spelllang=en_us
```

to automagically check the spelling within the file being edited.

If there is an Empirical replacement for a standard C++ feature in
the include/emp/base/ directory, you should use that replacement.
Specific cases include:
> -  Use emp::Ptr<> instead of raw pointers.
> -  Use emp_assert() instead of standard library assert().
> -  Use emp::array istead of std::array.
> -  Use emp::optional instead of std::optional.
> -  Use emp::vector instead of std::vector.


## Checklist

Copy and paste the following into a pull request comment when it is
ready for review:

    - [ ] Is it mergeable?
    - [ ] Did it pass the tests?
    - [ ] Does the source code follow the Empirical coding standards?
    - [ ] Has the code been commented (especially non-intuitive sections)
    - [ ] Was a spellchecker run on the source code and documentation after
      changes were made?

It's expected that before requesting a code review the author of the PR
will have checked all these things on their own. It's also expected
that whomever reviews the PR will check these individual items as well.
Though the CI runs most of these and will pass/fail the PR accordingly
it is not infallible and the whole point of having a code review process
is to have human eyes go over the changes to the codebase.
