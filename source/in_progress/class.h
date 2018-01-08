//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
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
//  Further, if you make the members private and need to add accessors, you can use:
//
//  EMP_ACCESSORS( NAME,
//                 TYPE1, ID1,
//                 TYPE2, ID2,
//                 etc... )
//
//  Finally, if you want it to build a specialized constructor for only these elements
//  (which should be rare, since if you're not adding more to the class, you should just
//   trigger EMP_CLASS), you can use:
//
//  EMP_CONSTRUCTOR( NAME,
//                   TYPE1, ID1, DEFAULT1,
//                   TYPE2, ID2, DEFAULT2,
//                   etc... )
//
//
//  Developer notes:
//  Goal: Make it trival to build a class that has self-reflection.
//        As such, it should interact will with serializaion and Javascript conversion.
//
//  An alternative option is to do something similar to the config object.  In order to build
//  a dynamic class, one would just create a file with a full set of macro calls.  They would
//  need to be included several times -- it might not be possible, but if it is it wouild allow
//  an arbitrarily large class definition.

#ifndef EMP_CLASS_H
#define EMP_CLASS_H

// EMP_MEMBERS expects a series of type/id pairs to setup.
#define EMP_CLASS_MEMBER_DECLARE(TYPE, ID) TYPE ID;
#define EMP_CLASS_MEMBERS(...) EMP_WRAP_ARG_PAIRS(EMP_MEMBER_DECLARE, __VA_ARGS__)

// EMP_CLASS_ACCESSORS_DECLARE expect the name of this class, the type of the id,
// and the name of the id.
#define EMP_CLASS_ACCESSORS_DECLARE(NAME, TYPE, ID)                     \
  public: TYPE Get ## ID() { return m_ ## ID; }                         \
  NAME & Set ## ID(const TYPE & _in) { m_ ## ID = _in;  return *this; }

// EMP_CLASS_ACCESSORS expect the name of the class followd by type/id pairs to setup.
#define EMP_CLASS_ACCASSORS(NAME, ...) @CAO

// EMP_CLASS_CONSTRUCTOR takes the class name followd by a set of types, ids, and default
// values which it uses to setup initialization.
#define EMP_CLASS_CONSTRUCTOR(NAME, ...) @CAO

#define EMP_CLASS( NAME, ...)                                        \
  class NAME {                                                       \
  private:                                                           \
    EMP_CLASS_MEMBERS(EMP_FILTER_ARGS((i,i,x), __VA_ARGS__))         \
  public:                                                            \
    EMP_CLASS_ACCESSORS(NAME, EMP_FILTER_ARGS((i,i,x), __VA_ARGS__)) \
  };


#endif
