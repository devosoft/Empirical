#ifndef EMP_UI_BASE_H
#define EMP_UI_BASE_H

#include <functional>
#include <ostream>
#include <string>

namespace emp {
namespace UI {

  // Base class for UI elements.

  class UI_base {
  protected:
    std::string temp_name;
    int width;
    int height;

  public:
    UI_base(const std::string & in_name) : temp_name(in_name), width(-1), height(-1) { ; }
    virtual ~UI_base() { ; }

    UI_base & TempName(const std::string & in_name) { temp_name = in_name; return *this; }
    UI_base & Width(int w) { width = w; return *this; }
    UI_base & Height(int h) { height = h; return *this; }
    UI_base & Size(int w, int h) { width = w; height = h; return *this; }

    const std::string & GetTempName() const { return temp_name; }
  };


  // Class to detail specs of buttons.

  class Button : public UI_base {
  protected:
    std::function<void()> callback;
    std::string label;

    bool autofocus;
    bool disabled;
    std::string title;

    uint32_t callback_id;
    
    void WriteHTML(std::ostream & os) {
      os << "<button";                          // Start the button tag.
      if (title != "") {                          // If the button has a title, add it!
        os << " title=\"" << title << "\"";
      }
      if (width > 0 || height > 0) {              // If the button has a style, add it!
        os << " style=\"";
        if (width > 0) os << "width:" << width << "px;";
        if (height > 0) os << "height:" << height << "px;";
        os << "\"";
      }

      // Indicate action on click.
      os << " onclick=\"empCppCallback(" << std::to_string(callback_id) << ")\"";

      os << ">" << label << "</button>";       // Close and label the button.
    }
  public:
    Button(const std::function<void()> & in_cb,
           const std::string & in_label="",
           const std::string & in_name="")
      : UI_base(in_name), callback(in_cb), label(in_label)
      , autofocus(false), disabled(false), title("")
      , callback_id(JSWrap(callback)) { ; }
    ~Button() {
      // @CAO Need to cleanup callabck! 
    }

    Button & Callback(const std::function<void()> & in_cb) {
      // @CAO Need to cleanup old callback!
      callback = in_cb;
      callback_id = JSWrap(callback);
      return *this;
    }
    Button & Label(const std::string & in_label) { label = in_label; return *this; }

    Button & Autofocus(bool in_af) { autofocus = in_af; return *this; }
    Button & Disabled(bool in_dis) { disabled = in_dis; return *this; }
    Button & Title(const std::string & t) { title = t; return *this; }
  };

  class Image : public UI_base {
  protected:
    std::string url;

    std::string alt_text;

    void WriteHTML(std::ostream & os) {
      os << "<img src=\"" << url << "\" alt=\"" << alt_text << "\"";
      if (width > 0 || height > 0) {
        os << " style=\"";
        if (width > 0) os << "width:" << width << "px;";
        if (height > 0) os << "height:" << height << "px;";
        os << "\"";
      }
      os << ">";
    }

  public:
    Image(const std::string & in_url, const std::string & in_name="")
      : UI_base(in_name), url(in_url), alt_text("") { ; }

    Image & URL(const std::string & in_url) { url = in_url; return *this; }
    Image & Alt(const std::string & in_alt) { alt_text = in_alt; return *this; }
  };

  struct TableCell {
    int row;      // In which row is the upper-left corner of this cell?
    int col;      // In which column is the upper-left corner of this cell?
    int width;    // How many columns wide is this cell?.
    int height;   // How many rows tall is this cell?
    bool header;  // Is this cell a header?
    bool masked;  // Is this cell hidden by another?
    // @CAO color?
  };

  class Table : public UI_base {
  protected:
    int cols;
    int rows;

    std::vector<TableCell> cell_info;

    void WriteHTML(std::ostream & os) {
    }
  public:
    Table(int in_cols, int in_rows, const std::string & in_name="")
      : UI_base(in_name), cols(in_cols), rows(in_rows) { ; }

    Table & Cols(int c) { cols = c; return *this; }
    Table & Rows(int r) { rows = r; return *this; }
  };


  class Text : public UI_base {
  protected:
  public:
    Text(const std::string & in_name="") : UI_base(in_name) { ; }
  };


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

};
};

#endif
