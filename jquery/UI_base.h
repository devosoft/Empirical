#ifndef EMP_UI_BASE_H
#define EMP_UI_BASE_H

#include <functional>
#include <string>

namespace emp {
namespace JQ {

  struct Button {
    std::function<void()> callback;
    std::string label;
    std::string name;

    Button(const std::function<void()> & in_cb,
           const std::string & in_label="",
           const std::string & in_name="")
      : callback(in_cb), label(in_label), name(in_name) { ; }
  };

  struct Image {
    std::string url;
    std::string name;

    std::string alt_text;
    int width;
    int height;

    Image(const std::string & in_url, const std::string & in_name="")
      : url(in_url), name(in_name), alt_text(""), width(-1), height(-1) { ; }

    Image & Name(const std::string & in_name) { name = in_name; return *this; }
    Image & Alt(const std::string & in_alt) { alt_text = in_alt; return *this; }
    Image & Width(int w) { width = w; return *this; }
    Image & Height(int h) { height = h; return *this; }
    Image & Size(int w, int h) { width = w, height = h; return *this; }
  };

  struct Table {
    int cols;
    int rows;
    std::string name;

    Table(int in_cols, int in_rows, const std::string & in_name="")
      : cols(in_cols), rows(in_rows), name(in_name) { ; }
  };

  template <typename VAR_TYPE>
  std::function<std::string()> Var(VAR_TYPE & var) {
    return [&var](){ return emp::to_string(var); };
  }

};
};

#endif
