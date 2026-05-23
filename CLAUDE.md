# CLAUDE.md

Operating rules for working in this codebase. Read all of it before making changes.

---

## Project goals

This is a header-only C++ library intended for scientific software development.  All code should be:
- **Reliable**.  Correctness is top priority.  Asserts should be included where possible to ensure code is operating to expectations.  Unit tests should verify correct behavior, covering typical use cases and edge cases.  Interfaces should be intuitive so they are not accidentally misused.  All pitfalls should be clearly marked.
- **Useful**. Interfaces should be easy to understand an make effective use of.  All code should be intended for use in a variety of scientific software project, not specialized for a single project.  Common scientific use cases not covered in the standard library are high priority.
- **Efficient**. Many scientific software projects must run for weeks or months to produce required data.  All components should be made as generally efficient as possible.  High priority should be given to components likely to be speed bottlenecks.
- **Compilable with Emscripten**. One goal is to make it easy to put scientific software on the web.  As such, whenever possible components should be usable with the Emscripten compiler to WebAssembly.

## Project specifications

- **Language standard:** C++23. Prefer modern C++23 idioms (ranges, `constexpr`, `std::expected`, concepts, `[[nodiscard]]`, designated initializers, etc.) over older equivalents unless there is a cost to efficiency. If a modern construct is cleaner but measurably slower on a hot path, keep the older form and leave a `// PERF:` comment explaining why.  If unsure, ask.
- **Target / toolchain:** Must compile and run under **Emscripten** (WebAssembly) as well as native clang and gcc. Don't introduce code that builds natively but breaks the Emscripten build. When a feature's portability is uncertain, flag it rather than assuming.
- **No exceptions.** The project is built `-fno-exceptions` for Emscripten performance. Do not add `throw`, `try`, or `catch`, and do not call standard library APIs in a way that relies on exceptions for control flow. For error handling use `std::expected` if run-time responses are reasonable, or `emp_assert` (defined in `include/emp/base/assert.hpp`) if it is programmer error. Treat "I'll just throw here" as a rule violation, not a shortcut.
- **Build commands:** Standard builds should have Makefiles.  In general, `make` should compile natively and `make web` should compile with Emscripten.
- **Directory layout:** All C++ header files associated with the library should be in `include/emp/`, with tests going into the parallel `tests/` directory.
- **Dependencies:** This project minimizes the use of dependencies.  Adding additional third-party dependencies is a decision that requires asking first, not a default move. Prefer the standard library or what's already vendored.

## Tools in the Empirical

All of the header files to include by library users are in `include/emp/`.  Helpful files to make use of are:

- `base/assert.hpp` - Defines macro `emp_assert` which behaves as a normal assert, but works well with Emscripten and allows additional arguments in the assert.  Literal strings args are printed as messages.  Variables or expressions are shown followed by what they evaluate to.
- `base/notify.hpp` - Provides dynamic error handling that can be overridden for any particular executable. Use `emp::notify::Error(...)` and `emp::notify::Warning(...)` for error/warning reporting. The word "Error:" or "Warning:" will print and then all arguments will be printed in sequence to the appropriate output method. `Error` will abort the run, while `Warning` will not.
- `base/Ptr.hpp` - `emp::Ptr<T>` does auditable pointer handling; it acts like a raw pointer, but in debug mode will point out any mis-handling.  Do not use raw pointers or shared pointers (though `std::unique_ptr` is okay to use.)
- `base/vector.hpp` and `base/array.hpp` - Identical to `std::vector` and `std::array`, but auditable in debug mode.
- `Bits.hpp` - Defines `emp::BitVector`, `emp::BitSet`, and other bit handling tools, all with the same core interface. These are fast and easy to use.
- `compiler/Lexer.hpp` - Allows for the dynamic definition of a Lexer (`emp::Lexer`) using regular expressions.
- `config/SettingsManager.hpp` - Helps handle configuration variables, both through a config file and command-line argument processing.
- `data/DataOutput.hpp` - Provides a simple way of managing output files; the programmer can supply lambdas that are called to fill out rows in an output CSV file.
- `datastructs/RobinHoodMap.hpp` and `datastructs/RobinHoodSet.hpp`- provide fast and efficient hash table implementations using Robin Hood hashing.
- `geometry/` - A set of useful classes when working with 2D geometric problems. 
- `io/CPPFile.hpp` - Provides tools when generating an output file that will contain C++ code.
- `io/File.hpp` - Provides general tools for working with files where you want to load an entire file into memory to manipulate and possibly save again.
- `math/combos.hpp` - Tools to step through all possible subset combinations of a given size.
- `math/Distribution.hpp` - Tools to work with pre-defined distributions of values, often that you want to draw a random value from.
- `math/Random.hpp` - A random number generator that is faster, more random, and easier to use than the standard library.
- `math/Range.hpp` - The `emp::Range` class tracks the simple boundaries for a range of numbers.
- `tools/GridSize.hpp` - Defines `emp::GridSize` and `emp::GridPos` to make working with grid positions more intuitive.
- `tools/String.hpp` - Derived from `std::string`, adding many new member functions to simplify string manipulation.
- `tools/Timer.hpp` - A profiling tool.  Provide the timer name as a template parameter and it will start tracking time on construction and stop on destruction, aggregating over all instances.
- `web/` - This directory holds many tools for creating and modifying web pages, using Emscripten.

---

## Rules

### 1. Don't assume. Don't hide confusion. Surface tradeoffs.

Proceed without asking on low-stakes, easily-reversible choices (local naming, obvious control flow). **Stop and ask** when a non-trivial choice affects public API shape, data structures, on-disk/serialized formats, dependencies, build configuration, or anything expensive to unwind later. When more than one reasonable design exists with tradeoffs, present those tradeoffs and your recommendation **before** implementing — don't silently pick one and move on. Mark unresolved points inline with `// QUESTION:` or `// ASSUMPTION:` rather than burying them in prose or omitting them.

### 2. Touch only what you must.

Don't refactor, rename, or reformat code outside a task's scope, even if it looks improvable — that pollutes the diff and the review. Keep changes minimal and reviewable. If you notice an unrelated bug or other real issue, **note it** (in your summary or a `// TODO:` / `// BUG:` comment) rather than either silently fixing it (scope creep) or silently ignoring it (negligence). Scope discipline is the goal, not indifference to defects.

### 3. Define success criteria. Loop until verified — then stop.

State the success criteria *before* implementing; if they are non-obvious, confirm them. "Verified" has a specific meaning here: the code **compiles without warnings under C++23 using the project's Makefile and appropriate target**. If you are unsure of which target to use, ask.  If you've recently used a target and it includes the relevant edited file(s), you may assume it's okay to keep using.  If the criteria can't be met after a couple of honest attempts, **stop and report what's failing** with specifics. Do not weaken the criteria, delete or skip a failing test (if there are any), or thrash to manufacture a green result.

---

## Additional expectations

- **Performance posture.** This is performance-sensitive code. Avoid unnecessary heap allocation, copies, or slow algorithms in hot paths. Do not micro-optimize speculatively either — if a change is performance-motivated, say so with a justification.
- **Comments explain *why*, not *what*.** The code says what it does. Comments should capture intent, invariants, non-obvious constraints, and the reasons behind tradeoffs — especially anything Emscripten- or perf-driven that would otherwise look arbitrary to a future reader.
- **Match existing style.** Naming, header/include conventions, error handling pattern, and structure should look like surrounding code, not like a generic example. When in doubt, find the nearest analogous code in the repo and follow it.
- **Try to keep lines to 100 chars or less.** Only allow longer lines if it will lead to much clearer code.
- **Don't invent APIs or facts.** If you're unsure whether a standard library feature, an Emscripten capability, or an internal function exists or behaves as you think, say so and verify rather than asserting it confidently.
- **Summarize what changed and why.** End substantive work with a short summary: what changed, what was verified and how, any assumptions made, and anything noted-but-not-fixed under rule 3.
- **Ask before large or destructive moves.** Mass renames, file moves, deleting code that looks unused, or changing the build system are "ask first" actions even if they seem clearly correct.
- **Alert if these rules are problematic.** If any of these rules are substantially reducing the quality of the project, please note this an suggest a better alternative way to meet the goals.
