# General Purpose Debugging Tools

## Empirical vectors and arrays

Empirical contains of intelligent versions of STL vectors and arrays
that will warn you if you make common errors, but otherwise behave
identically to the STL version. Most importantly, they will detect
attempts to index to out-of-bounds locations and throw an error. These
features are critical for writing code that will be compiled to
Javascript with Emscripten, because Valgrind doesn't run on Javascript.
They also save a lot of debugging time when writing native C++ code.

"But wait," you might say, "surely all of these additional checks
slow down your code!" This is true when you compile in debug mode (the
default). However, when you compile with the -DNDEBUG flag, these
objects are directly replaced with their STL equivalent, removing any
slowdown. That way, you can get all the debugging benefits while you're
writing your program, but all the speed benefits when you're actually
using it.

### emp::vector Example

```cpp
#include "Empirical/include/emp/base/vector.hpp"

emp::vector<int> vec({1,2,3});
// You can treat this just like an std::vector<int>
```

### emp::array Example

```cpp
#include "Empirical/include/emp/base/array.hpp"

emp::array<int, 3> array({1,2,3});
// You can treat this just like an std::array<int, 3>
```

Empirical asserts
-----------------

These asserts function similarly to normal asserts, with a few important
additional features: - If compiled with Emscripten they will provide
pop-up alerts when run in a web browser. - emp_assert can take
additional arguments. If the assert is triggered, those extra arguments
will be evaluated and printed. - if NDEBUG -or- EMP_NDEBUG is defined,
the expression in emp_assert() is not evaluated. - if EMP_TDEBUG is
defined, emp_assert() goes into test mode and records failures, but
does not abort. (useful for unit tests of asserts)

Example:

```cpp
#include "Empirical/include/emp/base/assert.hpp"

int a = 6;
emp_assert(a==5, a);
```

When compiled in debug mode (i.e. without the -DNDEBUG flag), this will
trigger an assertion error and print the value of a.

### emp_assert API (base/assert.hpp)

% ```{doxygendefine} emp_assert
% :project: Empirical
% ```

```{eval-rst}
.. doxygendefine:: emp_assert
  :project: Empirical
  :no-link:
```

```{eval-rst}
.. doxygendefine:: emp_emscripten_assert
  :project: Empirical
  :no-link:
```

## Empirical pointers

Ptr objects behave as normal pointers under most conditions. However, if
a program is compiled with `EMP_TRACK_MEM` set, then these pointers
perform extra tests to ensure that they point to valid memory and that
memory is freed before pointers are released.

If you trip an assert, you can re-do the run a track a specific pointer
by defining `EMP_ABORT_PTR_NEW` or `EMP_ABORT_PTR_DELETE` to the ID of
the pointer in question. This will allow you to track the pointer more
easily in a debugger.

Example:

```cpp
#include "Empirical/include/emp/base/Ptr.hpp"

emp::Ptr<int> int_ptr;
int_ptr.New(123456); // Store the value 123456 in int_ptr.
std::cout << "*int_ptr = " << *int_ptr << std::endl;
int_ptr.Delete();
```
