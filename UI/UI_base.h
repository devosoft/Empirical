#ifndef EMP_UI_BASE_H
#define EMP_UI_BASE_H

#include <functional>
#include <ostream>
#include <string>

namespace emp {
namespace UI {

  namespace internal {
  
    // CSS_Info contains information about a single CSS Setting.
    
    struct CSS_Info {
      std::string setting;
      std::string value;
      
      CSS_Info(const std::string & s, const std::string & v) : setting(s), value(v) { ; }
    };

    // Widget_base is a base class containing information needed by all GUI widget classes
    // (Buttons, Images, etc...).
    
    class Widget_base {
    protected: 
      std::string temp_name;
      int width;
      int height;
      
      std::vector<CSS_Info> css_mods;
      
      Widget_base() : temp_name(""), width(-1), height(-1) { ; }
    };
    
    // Widget_wrap is a template wrapper to make sure all derived widgets return the
    // correct types when setting values.
    
    template <typename DETAIL_TYPE, typename... ARG_TYPES>
    class Widget_wrap : public DETAIL_TYPE {
    public:
      Widget_wrap(ARG_TYPES... args, const std::string & in_name="") : DETAIL_TYPE(args...)
      { Widget_base::temp_name = in_name; }

      Widget_wrap & TempName(const std::string & in_name) { Widget_base::temp_name = in_name; return *this; }
      Widget_wrap & Width(int w) { Widget_base::width = w; return *this; }
      Widget_wrap & Height(int h) { Widget_base::height = h; return *this; }
      Widget_wrap & Size(int w, int h) {
        Widget_base::width = w;
        Widget_base::height = h;
        return *this;
      }
      
      Widget_wrap & Background(const std::string & color) {
        Widget_base::css_mods.emplace_back("background-color", color);
        return *this;
      }
      
      const std::string & GetTempName() const { return Widget_base::temp_name; }
    };
    
    
    // Class to detail specs of buttons.
    
    class Button_detail;
  };

  using Button = internal::Widget_wrap<internal::Button_detail,
                                       const std::function<void()> &,
                                       const std::string &>;

  namespace internal {

    class Button_detail : public Widget_base {
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
      Button_detail(const std::function<void()> & in_cb, const std::string & in_label)
        : callback(in_cb), label(in_label)
        , autofocus(false), disabled(false), title("")
        , callback_id(JSWrap(callback))
      { ; }
      ~Button_detail() {
        // @CAO Need to cleanup callback! 
      }

      Button & Callback(const std::function<void()> & in_cb) {
        // @CAO Need to cleanup old callback before setting up new one.
        callback = in_cb;
        callback_id = JSWrap(callback);
        return (Button &) *this;
      }
      Button & Label(const std::string & in_label) { label = in_label; return (Button &) *this; }

      Button & Autofocus(bool in_af) { autofocus = in_af; return (Button &) *this; }
      Button & Disabled(bool in_dis) { disabled = in_dis; return (Button &) *this; }
      Button & Title(const std::string & t) { title = t; return (Button &) *this; }
    };


    // Class to detail specifications for images.

    class Image_detail;
  };

  using Image = internal::Widget_wrap<internal::Image_detail, const std::string &>;

  namespace internal {
    class Image_detail : public Widget_base {
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
      Image_detail(const std::string & in_url)
        : url(in_url), alt_text("") { ; }
      
      Image & URL(const std::string & in_url) { url = in_url; return (Image &) *this; }
      Image & Alt(const std::string & in_alt) { alt_text = in_alt; return (Image &) *this; }
    };
    
    class Table_detail;
  }
  
  using Table = internal::Widget_wrap<internal::Table_detail, int, int>;

  struct TableCell {
    int row;      // In which row is the upper-left corner of this cell?
    int col;      // In which column is the upper-left corner of this cell?
    int width;    // How many columns wide is this cell?.
    int height;   // How many rows tall is this cell?
    bool header;  // Is this cell a header?
    bool masked;  // Is this cell hidden by another?
  };

  namespace internal {
    class Table_detail : public Widget_base {
    protected:
      int cols;
      int rows;
      
      std::vector<TableCell> cell_info;
      
      void WriteHTML(std::ostream & os) {
      }
    public:
      Table_detail(int in_cols, int in_rows) : cols(in_cols), rows(in_rows) { ; }
      
      Table & Cols(int c) { cols = c; return (Table &) *this; }
      Table & Rows(int r) { rows = r; return (Table &) *this; }
    };
  }

  // class Text : public UI_base {
  // protected:
  // public:
  //   Text(const std::string & in_name="") : UI_base(in_name) { ; }
  // };


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
