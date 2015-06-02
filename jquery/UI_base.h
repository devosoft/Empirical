#ifndef EMP_UI_BASE_H
#define EMP_UI_BASE_H

#include <functional>
#include <string>

namespace emp {
namespace JQ {

  class Button {
  protected:
    std::string temp_name;
    std::function<void()> callback;
    std::string label;

    bool autofocus;
    bool disabled;
    int width;
    int height;
    std::string title;

  public:
    Button(const std::function<void()> & in_cb,
           const std::string & in_label="",
           const std::string & in_name="")
      : temp_name(in_name), callback(in_cb), label(in_label)
      , autofocus(false), disabled(false), width(-1), height(-1), title("") { ; }

    Button & TempName(const std::string & in_name) { temp_name = in_name; return *this; }
    Button & Callback(const std::function<void()> & in_cb) { callback = in_cb; return *this; }
    Button & Label(const std::string & in_label) { label = in_label; return *this; }

    Button & Autofocus(bool in_af) { autofocus = in_af; return *this; }
    Button & Disabled(bool in_dis) { disabled = in_dis; return *this; }
    Button & Width(int w) { width = w; return *this; }
    Button & Height(int h) { height = h; return *this; }
    Button & Size(int w, int h) { width = w; height = h; return *this; }
    Button & Title(const std::string & t) { title = t; return *this; }

    const std::string & GetTempName() const { return temp_name; }
  };

  class Image {
  protected:
    std::string temp_name;
    std::string url;

    std::string alt_text;
    int width;
    int height;

  public:
    Image(const std::string & in_url, const std::string & in_name="")
      : temp_name(in_name), url(in_url), alt_text(""), width(-1), height(-1) { ; }

    Image & TempName(const std::string & in_name) { temp_name = in_name; return *this; }
    Image & URL(const std::string & in_url) { url = in_url; return *this; }
    Image & Alt(const std::string & in_alt) { alt_text = in_alt; return *this; }
    Image & Width(int w) { width = w; return *this; }
    Image & Height(int h) { height = h; return *this; }
    Image & Size(int w, int h) { width = w, height = h; return *this; }

    const std::string & GetTempName() const { return temp_name; }
  };

  class Table {
  protected:
    int cols;
    int rows;
    std::string temp_name;

  public:
    Table(int in_cols, int in_rows, const std::string & in_name="")
      : cols(in_cols), rows(in_rows), temp_name(in_name) { ; }

    Table & TempName(const std::string & in_name) { temp_name = in_name; return *this; }

    const std::string & GetTempName() const { return temp_name; }
  };

  template <typename VAR_TYPE>
  std::function<std::string()> Var(VAR_TYPE & var) {
    return [&var](){ return emp::to_string(var); };
  }

};
};

#endif
