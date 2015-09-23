#ifndef EMP_CLASS_H
#define EMP_CLASS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  A re-engineering of tuple_struct.h, intended to be usable throughout Empirical.
//
//  The EMP_CLASS macro builds an inexpensive class that
//  * Can fully reflect on its contents. (todo)
//  * Is easily serializable (todo)
//  * Can be effortlessly moved between C++ and Javascript (todo)
//
//  EMP_CLASS( NAME,
//             TYPE1, ID1, DEFAULT1,
//             TYPE2, ID2, DEFAULT2,
//             etc... )
//
//  If you want to just build member functions inside of a user-defined class, you
//  can alternatively use:
//
//  EMP_MEMBERS( TYPE1, ID1,
//               TYPE2, ID2,
//               etc... )
//
//  Note that if you are building only members, the macro does not have control over
//  the constructors, and as such cannot set defaults.
//

// EMP_MEMBERS expects a series of type/id pairs to setup.
#define EMP_CLASS_MEMBER_DECLARE(TYPE, ID) TYPE ID;
#define EMP_CLASS_MEMBERS(...) EMP_WRAP_ARG_PAIRS(EMP_MEMBER_DECLARE, __VA_ARGS__)

#define EMP_CLASS_ACCESSORS_DECLARE(NAME, TYPE, ID)                     \
  public: TYPE Get ## ID() { return m_ ## ID; }                         \
  NAME & Set ## ID(const TYPE & _in) { m_ ## ID = _in;  return *this; }

#define EMP_CLASS_ACCASSORS(NAME, ...) @CAO

#define EMP_CLASS_CONSTRUCTOR(NAME, ...) @CAO

#define EMP_CLASS( NAME, ...)                                  \
  class NAME {                                                 \
  private:                                                     \
    EMP_CLASS_MEMBERS(EMP_FILTER_ARGS((i,i,x), __VA_ARGS__))         \
  public:                                                      \
    EMP_CLASS_ACCESSORS(NAME, EMP_FILTER_ARGS((i,i,x), __VA_ARGS__)) \
  };


#endif
