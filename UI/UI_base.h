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

  template <typename VAR_TYPE>
  std::function<std::string()> Var(VAR_TYPE & var) {
    return [&var](){ return emp::to_string(var); };
  }

  // Live keyword means that whatever is passed in needs to be re-evaluated every update.

  template <typename VAR_TYPE>
  std::function<std::string()> Live(VAR_TYPE & var) {
    return [&var](){ return emp::to_string(var); };
  }

  template <typename RET_TYPE>
  std::function<std::string()> Live(const std::function<RET_TYPE()> & fun) {
    return [fun](){ return emp::to_string(fun()); };
  }

  std::function<std::string()> Live(const std::function<double()> & fun) {
    return [fun](){ return emp::to_string(fun()); };
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
