# Base Tools

A handful of tools are available in the include/emp/base/ folder in
Empirical. These mimic basic functionality available in C++ or its
standard library, but provide extra protection against common memory use
errors and additional information to the developer.  These protections
can be turned off by comiling with -DNDEBUG.

## base/assert.h

This file adds an `emp_assert` macro that can handle all of the same
functionality as the standard library assert, but with additional
features. Specifically, additional arguments may be added that are
printed when the assert is triggered. For example, the line

```cpp
emp_assert(i < 10, i);
```

if triggered, would print something to the effect of

```shell
Assert Error (In assert.cc line 6): i < 10
i: [1844674]
Abort
```

Indicating that not only was `i >= 10`, but its current value is
1844674.

If compiled as part of a web app (with emscripten) `emp_assert`: will
automatically use the JavaScript Alert function to indicate any assert
failures. Asserts are all disabled (fully removed from compiled code) if
compiled using the `NDEBUG` option (for most compilers, this
deactivation is accomplished by using the `-DNDEBUG` flag at compile
time.)

## base/array.h and base/vector.h

These files setup the `emp::array<...>` and `emp::vector<...>` template
objects, which behave almost identically to `std::array<...>` and
`std::vector<...>`, respectively. The one difference is that they do
bounds checking when they are indexed into or specific size matters. As
with asserts, these additional bounds checks are removed when compiled
with the `NDEBUG` option.

## base/Ptr.h

The `emp::Ptr<...>` template provides an alternate method of building
pointers, but with the ability to turn on additional debugging
facilities; unlike assert, array, and vector, no debugging is performed
by default due to substantial run-time costs. For example declaring a
variable as `emp::Ptr<int>` is the same as declaring it `int *`.

If the `EMP_TRACK_MEM` option is set (`-DEMP_TRACK_MEM` compiler flag)
then all pointer usage is tracked and many simple memory errors will be
identified during execution, such as using or deleting an unallocated
pointer.

Most usage of the `Ptr` class is identical to raw pointers, including
all operator. Differences include:

-   Rather than using `new TYPE` to allocate a new pointer, use
    `emp::NewPtr<TYPE>`
-   If allocating an array, use `emp::NewArrayPtr<TYPE>(SIZE)`
-   To delete use the `.Delete()` or `.DeleteArray()` member function
    (not `delete` or `delete[]`).
-   To cast one `Ptr` type to another, use the `.Cast<TYPE>` member
    function.
-   To dynamic cast (double-checking types and returning `nullptr` on
    failure), use `.DynamicCast<TYPE>`

To convert a `Ptr`-allocated pointer to the raw form, use the `Raw()`
member function. Make sure that any pointer allocated as a `Ptr` type is
also freed as a `Ptr` type.
