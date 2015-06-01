#ifndef EMP_JQ_VAR_H
#define EMP_JQ_VAR_H

// A simple wrapper for variables to indicate that their value should be tracked.

namespace emp {
namespace JQ {

  class Var_Base {
  public:
    Var_Base() { ; }
    virtual ~Var_Base() { ; }

    virtual std::string AsString() = 0;
  };

  template <typename VAR_TYPE>
  class Var : public Var_Base {
  private:
    VAR_TYPE & var;
  public:
    Var(VAR_TYPE & in_var) : var(in_var) { ; }
    ~Var() { ; }

    VAR_TYPE & GetVar() { return var; }
    const VAR_TYPE & GetVar() const { return var; }

    std::string AsString() { return emp::to_string(var); }
  };

};
};

#endif
