#ifndef EMP_UI_BASE_H
#define EMP_UI_BASE_H

#include <functional>
#include <ostream>
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
      : temp_name(in_name), callback(in_cb), label(in_label)
      , autofocus(false), disabled(false), width(-1), height(-1), title("")
      , callback_id(JSWrap(callback)) { ; }
    ~Button() {
      // @CAO Need to cleanup callabck! 
    }

    Button & TempName(const std::string & in_name) { temp_name = in_name; return *this; }
    Button & Callback(const std::function<void()> & in_cb) {
      // @CAO Need to cleanup old callback!
      callback = in_cb;
      callback_id = JSWrap(callback);
      return *this;
    }
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
      : temp_name(in_name), url(in_url), alt_text(""), width(-1), height(-1) { ; }

    Image & TempName(const std::string & in_name) { temp_name = in_name; return *this; }
    Image & URL(const std::string & in_url) { url = in_url; return *this; }
    Image & Alt(const std::string & in_alt) { alt_text = in_alt; return *this; }
    Image & Width(int w) { width = w; return *this; }
    Image & Height(int h) { height = h; return *this; }
    Image & Size(int w, int h) { width = w, height = h; return *this; }

    const std::string & GetTempName() const { return temp_name; }
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

  class Table {
  protected:
    std::string temp_name;
    int cols;
    int rows;

    std::vector<TableCell> cell_info;

    void WriteHTML(std::ostream & os) {
    }
  public:
    Table(int in_cols, int in_rows, const std::string & in_name="")
      : temp_name(in_name), cols(in_cols), rows(in_rows) { ; }

    Table & TempName(const std::string & in_name) { temp_name = in_name; return *this; }
    Table & Cols(int c) { cols = c; return *this; }
    Table & Rows(int r) { rows = r; return *this; }
    Table & Size(int c, int r) { cols = c; rows = r; return *this; }

    const std::string & GetTempName() const { return temp_name; }
  };

  template <typename VAR_TYPE>
  std::function<std::string()> Var(VAR_TYPE & var) {
    return [&var](){ return emp::to_string(var); };
  }

};
};

#endif
