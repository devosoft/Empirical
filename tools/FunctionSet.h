#ifndef EMP_FUNCTION_SET_H
#define EMP_FUNCTION_SET_H

#include <functional>
#include <vector>

namespace emp {
  
  template <typename... ARG_TYPES> class FunctionSet {
  private:
    std::vector<std::function<void(ARG_TYPES...)> > fun_set;

  public:
    FunctionSet() { ; }
    ~FunctionSet() { ; }

    int GetSize() const { return (int) fun_set.size(); }

    void Add(const std::function<void(ARG_TYPES...)> & in_fun) {
      fun_set.push_back(in_fun);
    }

    void Run(ARG_TYPES... params) {
      for (std::function<void(ARG_TYPES...)> & cur_fun : fun_set) {
        cur_fun(params...);
      }
    }
  };

};

#endif
