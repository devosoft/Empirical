#ifndef EMP_UI_BASE_H
#define EMP_UI_BASE_H

namespace emp {
namespace JQ {

  template <typename VAR_TYPE>
  std::function<std::string()> Var(VAR_TYPE & var) {
    return [&var](){ return emp::to_string(var); };
  }

};
};

#endif
