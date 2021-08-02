# Prefab Components

The prefabricated components in Empirical compose basic components and add features to make more complicated user interfaces.
Here's a log of some stylistic and design choices that might be useful in future development.

### Overriding Streaming

Most components use the stream operator (`<<`) to add user defined content.
However, many prefabricated components have multiple layers of divs and other tags so one might want to override this operator to have a different behavior.
However, this may accidentally cause an infinite loop if you're using something like `*this << sub_component;` to set up the component.
The trick is to use `static_cast<BaseClass>(*this) << sub_component;` where `BaseClass` is the class you're inheriting from (probably `Div`) to prevent using your newly overridden streaming operator in the initial construction.

### Inheritance

Since web components are actually shells around shared pointers getting inheritance to work properly can be tricky.
These components look something like this:

```cpp
namespace internal {
  class BaseInfo {
    // basic members
    public:
    BaseInfo() {
      // setup for info
    }
  };
}

class Base {
  internal::BaseInfo * info;

  public:
  Base(), info(new internal::BaseInfo()) {
    // basic setup
  }
}
```

Manipulating things like styling and adding event listeners happens through an instance of `Base` but the real information is stored in the member `info`.
Now suppose we want to create a component that has all the proprties of `Base` but has additional properties.
So really we need a derived info object:

```cpp
namespace internal {
  class DerivedInfo : public BaseInfo {
    // additional members
    public:
    DerivedInfo() : BaseInfo() {
      // further setup
    }
  };
}

```

But for the outer class if we do

```cpp
class Derived : public Base {

  public:
  Derived() : Base() {
    info = new internal::DerivedInfo()
    // Aah! lost the basic setup done in Base()
  }
}
```

then we have overwritten any modifications done in the base class (and possibly created a memory leak)!
Instead we should modify `Base` to have a protected constructor taking an `internal::BaseInfo` pointer.
The default constructor then delegates to the protected one.

```cpp
class Base {
  internal::BaseInfo * info;
  protected:
  Base(internal::BaseInfo * in_info), info(in_info) {
    // basic setup
  }

  public:
  Base() : Base(new internal::BaseInfo()) { ; }
}
```

Then in the `Derived` class we have,

```cpp
class Base {
  internal::BaseInfo * info;
  protected:
  Derived(internal::BaseInfo * info_ref) : Base(info_ref) {
    // extra fancy setup stuff
  }
  public:
  Derived() : Base(new internal::DerivedInfo()) { ; }
}
```
This way we maintain control over what `info` points to from any derived class while still ensuring constructors that add structure/attributes are called in the right order.

So the TL;DR version is:
to allow your prefab component to support inheritance,

* Use a protected constructor that takes an `Info` pointer to do the setup
* Have other constructors delegate to the protected constructor
