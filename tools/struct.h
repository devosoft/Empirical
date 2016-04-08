#ifndef EMP_STRUCT_H
#define EMP_STRUCT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  A re-engineering of tuple_struct.h, intended to be usable throughout Empirical.
//
//  The EMP_STRUCT macro builds an inexpensive struct that
//  * Can fully reflect on its contents. (todo)
//  * Is easily serializable (todo)
//  * Can be effortlessly moved between C++ and Javascript (todo)
//
//  EMP_STRUCT( NAME,
//              TYPE1, ID1, DEFAULT1,
//              TYPE2, ID2, DEFAULT2,
//              etc... )
//

#define EMP_STRUCT( NAME, ...)                               \
  struct NAME {                                              \
    EMP_ASSEMBLE_MACRO(EMP_STRUCT_MEM_, __VA_ARGS__)     \
  };

EMP_STRUCT_MEM_3(TYPE, ID, DEFAULT, ...) TYPE ID;
EMP_STRUCT_MEM_6(TYPE, ID, DEFAULT, ...) TYPE ID; EMP_STRUCT_MEM_3(__VA_ARGS__)
EMP_STRUCT_MEM_30(TYPE, ID, DEFAULT, ...) TYPE ID; EMP_STRUCT_MEM_27(__VA_ARGS__)

#endif
