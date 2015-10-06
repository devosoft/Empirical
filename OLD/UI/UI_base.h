#ifndef EMP_UI_BASE_H
#define EMP_UI_BASE_H

#include <functional>
#include <map>
#include <ostream>
#include <string>

namespace emp {
namespace UI {

  // Specialty functions...

  std::string Link(const std::string & url, const std::string & text="") {
    std::string out_string = "<a href=\"";
    out_string += url;
    out_string += "\">";
    out_string += (text == "") ? url : text;
    out_string += "</a>";
    return out_string;
  }

  // template <typename VAR_TYPE>
  // std::function<std::string()> Var(VAR_TYPE & var) {
  //   return [&var](){ return emp::to_string(var); };
  // }

  
  // Live keyword means that whatever is passed in needs to be re-evaluated every update.
  namespace internal {

    // If a variable is passed in to live, construct a function to look up its current value.
    template <typename VAR_TYPE>
    std::function<std::string()> Live_impl(VAR_TYPE & var, bool) {
      return [&var](){ return emp::to_string(var); };
    }
    
    // // If a function is passed in, call it and use the return value.
    // template <typename RET_TYPE>
    // std::function<std::string()> Live_impl(const std::function<RET_TYPE()> & fun, bool) {
    //   return [fun](){ return emp::to_string(fun()); };
    // }

    // If anything else is passed in, assume it is a function!
    template <typename IN_TYPE>
    std::function<std::string()> Live_impl(IN_TYPE && fun, int) {
      return [fun](){ return emp::to_string(fun()); };
    }
  }

  template <typename T>
  std::function<std::string()> Live(T && val) {
    return internal::Live_impl(std::forward<T>(val), true);
  }


  class Close {
  private:
    std::string close_id;
  public:
    Close(const std::string & id) : close_id(id) { ; }

    const std::string & GetID() const { return close_id; }
  };

  struct GetCell {
    int row;
    int col;
    int row_span;  // Stored row & column info to pass on.
    int col_span;

    GetCell(int r, int c) : row(r), col(c), row_span(0), col_span(0) { ; }
    GetCell & RowSpan(int rs) { row_span = rs; return *this; }
    GetCell & ColSpan(int cs) { col_span = cs; return *this; }
  };

  struct GetRow {
    int row;
    GetRow(int r) : row(r) { ; }
  };

};
};

#endif
