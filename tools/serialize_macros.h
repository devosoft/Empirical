// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is 
// Copyright (C) Michigan State University, 2015. It is licensed 
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_SERIALIZE_MACROS_H
#define EMP_SERIALIZE_MACROS_H

#include "macros.h"

#define EMP_SERIALIZE_INIT_VAR(VAR) VAR(emp::serialize::SetupLoad(pod, &VAR, true))

// Use this macro to automatically build methods in a class to save and load data.
#define EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME, BASE_LOAD, BASE_STORE, ...) \
  using emp_load_return_type = emp::serialize::DataPod;                 \
  void EMP_Store(emp::serialize::DataPod & pod) const {                 \
    BASE_STORE;                                                         \
    emp::serialize::Store(pod, __VA_ARGS__);                            \
  }                                                                     \
  explicit CLASS_NAME(emp::serialize::DataPod & pod)                    \
    : BASE_LOAD EMP_WRAP_ARGS(EMP_SERIALIZE_INIT_VAR, __VA_ARGS__) {    \
  }



// Version to use in stand-alone classes.
#define EMP_SETUP_DATAPOD(CLASS_NAME, ...) \
  EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME, , , __VA_ARGS__)

// Version to use in derived classes (with a base that also needs to be serialized).
#define EMP_SETUP_DATAPOD_D(CLASS_NAME, BASE_CLASS, ...)    \
  EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME,                    \
                             EMP_CALL_BASE_1(BASE_CLASS),   \
                             BASE_CLASS::EMP_Store(pod),    \
                             __VA_ARGS__)

// Version to use in derived classes (with TWO bases that need to be serialized).

#define EMP_SETUP_DATAPOD_D2(CLASS_NAME, BASE_CLASS1, BASE_CLASS2, ...) \
  EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME,                                \
                             EMP_CALL_BASE_2(BASE_CLASS1, BASE_CLASS2), \
                             BASE_CLASS1::EMP_Store(pod); BASE_CLASS2::EMP_Store(pod), \
                             __VA_ARGS__)

#define EMP_CALL_BASE_1(BASE1) BASE1(pod),
#define EMP_CALL_BASE_2(BASE1, BASE2) BASE1(pod), BASE2(pod),

#endif
