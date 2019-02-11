Coding guidelines and review checklist
======================================

This document is for those who want to contribute code or documentation fixes
to the Empirical project and describes our coding standards as well as our
code review process.

This document has been adapted from the `khmer project
<https://khmer.readthedocs.org/en/v1.4.1/dev/coding-guidelines-and-review.html>`_

C++ standards
-------------

We use C++11 features throughout the project and currently that is the
de-facto standard version to use.

All code should be in header files for ease of inclusion into Emscripten
projects.

Files that define a single class should be named after that class. Files that
define sets of functions or multiple classes should have an all-lowercase name
that describes its contents.

All files and all directories must be levelized. This is partly enforced
through all files being headerfiles (and thus we cannot have circular
dependencies), but for clean coding practices (and easy of unit testing) whole
directories should not refer to each other bidirectionally either. See
`Large-Scale C++ Software Design by John Lakos
<http://www.amazon.com/Large-Scale-Software-Design-John-Lakos/dp/0201633620/>`_
for a strong pro-levelization argument.

In-code identifier formatting is always hard to settle upon. The guidelines
below are for consistency.

 - Variable names should be all_lowercase, with words separated by underscores
 - Function names should be CamelCase() unless they are meant to mimic a
   function from the C++ standard library, at which point they can be
   all_lowercase to fit in.
 - User-defined types should be CamelCase
 - Constants should be ALL_UPPERCASE, with words separated by underscores
 - Template parameters should be ALL_UPPERCASE.
 - Typedefs should match the casing of the types they are aliasing. For
   example, a typedef on a template parameter might be all uppercase, while a
   typedef on a user-defined type should be CamelCase.

Guidelines based on Emscripten Limitations
-------------------------------------------

 - Try to avoid use of 64-bit integers (that is, the "long long" type).
   Emscripten has to emulate these and they can cause a notable slowdown.

 - Do not rely on exceptions when possible. Emscripten is slow at dealing with
   them and they can slow down code even when not triggered.

 - Do not write multi-threaded code that uses shared state. Javascript cannot
   (yet) handle such code and as such Emscripten cannot compile it. Note that
   Emscripten does have experimental support of pthreads.

 - Obviously, do not use any architecture-specific tricks, such as assuming
   endianness, doing unaligned reads or writes, directly accessing registers,
   etc.

Please see the `Emscripten doc page
<https://kripken.github.io/emscripten-site/docs/porting/guidelines/portability_guidelines.html>`_
for a full list.

General Standards
-----------------

All plain-text files should have line widths of 100 characters or less unless
that is unsupported for the particular file format.

All contributions should have their spelling checked before being committed to
the codebase.

Vim users can run::

        :setlocal spell spelllang=en_us

to automagically check the spelling within the file being edited.

Checklist
---------

Copy and paste the following into a pull request comment when it is ready for
review::

   - [ ] Is it mergeable?
   - [ ] Did it pass the tests?
   - [ ] Does 'make doc' succeed?
   - [ ] If you added code, is it tested? Look at the output for 'make diff-cover'
   - [ ] Was a spellchecker run on the source code and documentation after
     changes were made?

It's expected that before requesting a code review the author of the PR will have checked all these
things on their own. It's also expected that whomever reviews the PR will check these individual
items as well. Though the CI runs most of these and will pass/fail the PR accordingly it is not
infallible and the whole point of having a code review process is to have human eyes go over the
changes to the codebase.
