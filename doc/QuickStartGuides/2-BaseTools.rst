Base Tools
==========

A handful of tools are available in the source/base/ folder in Empirical.  These mimic basic
functionality available in C++ or the standard library, but provide extra protection against
common memory use errors and additional information to the developer.

base/assert.h
---------------------

This file adds an :code:`emp_assert` macro that can handle all of the same functionality as the
standard library assert, but with additional features.  Specifically, additional arguments
may be added that are printed when the assert is triggered.  For example, the line

.. code-block:: C++

  emp_assert(i < 10, i);

if triggered, would print something to the effect of

.. code-block:: bash

  Assert Error (In assert.cc line 6): i < 10
  i: [1844674]
  Abort

Indicating that not only was :code:`i >= 10`, but its current value is 1844674.

If compiled as part of a web app (with emscripten) :code:`emp_assert`: will automatically use the
JavaScript Alert function to indicate any assert failures.  Asserts are all disabled (fully
removed from compiled code) if compiled using the :code:`NDEBUG` option (for most compilers, this
deactivation is accomplished by using the :code:`-DNDEBUG` flag at compile time.)


base/array.h and base/vector.h
----------------------------------

These files setup the :code:`emp::array<...>` and :code:`emp::vector<...>` template objects, which behave
almost identically to :code:`std::array<...>` and :code:`std::vector<...>`, respectively.  The one difference
is that they do bounds checking when they are indexed into or specific size matters.  As with
asserts, these additional bounds checks are removed when compiled with the :code:`NDEBUG` option.


base/Ptr.h
-------------

The :code:`emp::Ptr<...>` template provides an alternate method of building pointers, but with the
ability to turn on additional debugging facilities; unlike assert, array, and vector, no
debugging is performed by default due to substantial run-time costs.  For example declaring a
variable as :code:`emp::Ptr<int>` is the same as declaring it :code:`int *`.

If the :code:`EMP_TRACK_MEM` option is set (:code:`-DEMP_TRACK_MEM` compiler flag) then all pointer usage is
tracked and many simple memory errors will be identified during execution, such as using or
deleting an unallocated pointer.

Most usage of the :code:`Ptr` class is identical to raw pointers, including all operator.
Differences include:

* Rather than using :code:`new TYPE` to allocate a new pointer, use :code:`emp::NewPtr<TYPE>`
* If allocating an array, use :code`:emp::NewArrayPtr<TYPE>(SIZE)`
* To delete use the :code:`.Delete()` or :code:`.DeleteArray()` member function (not :code:`delete` or :code:`delete[]`).
* To cast one :code:`Ptr` type to another, use the :code:`.Cast<TYPE>` member function.
* To dynamic cast (double-checking types and returning :code:`nullptr` on failure), use :code:`.DynamicCast<TYPE>`


To convert a :code:`Ptr`-allocated pointer to the raw form, use the :code:`Raw()` member function.  Make sure
that any pointer allocated as a :code:`Ptr` type is also freed as a :code:`Ptr` type.
