#ifndef EMP_SERIALIZE_MACROS_H
#define EMP_SERIALIZE_MACROS_H

#include "macros.h"

#define EMP_SERIALIZE_INIT_VAR(VAR) VAR(emp::serialize::SetupLoad<decltype(VAR)>(pod, VAR))

// Use this macro to automatically build methods in a class to save and load data.
#define EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME, BASE_LOAD, BASE_STORE, ...) \
  typedef emp::serialize::DataPod EMP_DataPod_t;                        \
  void EMP_Store(emp::serialize::DataPod & pod) {                       \
    BASE_STORE;                                                         \
    emp::serialize::Store(pod, __VA_ARGS__);                            \
  }                                                                     \
  explicit CLASS_NAME(emp::serialize::DataPod & pod) BASE_LOAD {        \
    emp::serialize::Load(pod, __VA_ARGS__);                             \
  }


// Version to use in stand-along classes.
#define EMP_SETUP_DATAPOD(CLASS_NAME, ...) \
  EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME, , , __VA_ARGS__)

// Version to use in derived classes (with a base that also needs to be serialized).
#define EMP_SETUP_DATAPOD_D(CLASS_NAME, BASE_CLASS, ...)    \
  EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME,                    \
                             :BASE_CLASS(pod),              \
                             BASE_CLASS::EMP_Store(pod),    \
                             __VA_ARGS__)

// Version to use in derived classes (with TWO bases that need to be serialized).

#define EMP_SETUP_DATAPOD_D2(CLASS_NAME, BASE_CLASS1, BASE_CLASS2, ...) \
  EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME,                                \
                             EMP_CALL_BASE_2(BASE_CLASS1, BASE_CLASS2), \
                             BASE_CLASS1::EMP_Store(pod); BASE_CLASS2::EMP_Store(pod), \
                             __VA_ARGS__)

#define EMP_CALL_BASE_2(BASE1, BASE2) :BASE1(pod), BASE2(pod)


#endif
