#ifndef EMP_WEB_WIDGET_H
#define EMP_WEB_WIDGET_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Widgets maintain individual components on a web page and link to Elements
//
//  WidgetInfo contains the basic information for all Widgets
//  Widget is a generic base class, with a shared pointer to WidgetInfo
//  WidgetFacet is a template that allows Set* methods to return derived return-type.
//
//  In other files, Widgets will be used to define specific elements.
//  ELEMENTInfo maintains information about the specific widget (derived from WidgetInfo)
//  ELEMENT interfaces to ELEMENTInfo so multiple elements use same core; derived from WidgetFacet
//
//
//  Development notes:
//  * Change 'active' flag to an activity state enum, which can include INACTIVE,
//    WAITING (for document ready), LIVE, and ERROR (due to not having an element?).
//

#include <string>

#include "../tools/mem_track.h"
#include "../tools/vector.h"

#include "events.h"
#include "Style.h"


namespace emp {
namespace web {

  // Setup some types we will need later
  class Button;
  class Canvas;
  class Image;
  class Table;
  class Text;
  class Selector;
  class Slate;

  namespace internal {

      // Provide a quick method for generating unique IDs when not otherwise specified.
    static std::string NextWidgetID() {
      static int next_id = 0;
      return emp::to_string("emp__", next_id++);
    }
    
    // Pre-declate WidgetInfo so classes can inter-operate.
    class WidgetInfo;

    // Widget is a smart pointer to a WidgetInfo object, plus some basic accessors.
    class Widget {
      friend WidgetInfo;
    protected:
      WidgetInfo * info;

      // If an Append doesn't work with current class, forward it to the parent.
      template <typename FWD_TYPE> Widget & ForwardAppend(FWD_TYPE && arg);

      // Activates need to be delayed until the document is ready, when DoActivate will be called.
      void DoActivate();

      Widget & SetInfo(WidgetInfo * in_info);

    public:
      Widget(const std::string & id);
      Widget(WidgetInfo * in_info=nullptr);
      Widget(const Widget & in) : Widget(in.info) { ; }
      Widget & operator=(const Widget & in) { return SetInfo(in.info); }
      
      virtual ~Widget();

      bool IsNull() const { return info == nullptr; }
      bool IsActive() const;

      virtual bool IsSlate() const { return false; }
      virtual bool IsTable() const { return false; }
      virtual bool IsText() const { return false; }
      virtual bool AppendOK() const;
      virtual std::string GetWrapperTag() const { return "span"; }

      std::string GetID() const;
      virtual std::string CSS(const std::string & setting);
      bool HasCSS(const std::string & setting);
      bool HasChild(const Widget & test_child);

      WidgetInfo * operator->() { return info; }
      bool operator==(const Widget & in) const { return info == in.info; }
      bool operator!=(const Widget & in) const { return info != in.info; }
      operator bool() const { return info != nullptr; }

      // An active widget makes live changes to the webpage (once document is ready)
      // An inactive widget just records changes internally.
      void Activate() {
        OnDocumentReady( std::function<void(void)>([this](){ this->DoActivate(); }) );
      }
      void Deactivate(bool top_level=true);
      bool ToggleActive();

      // Setup << operator to redirect to Append.
      template <typename IN_TYPE> Widget operator<<(IN_TYPE && in_val);
    };
    

    // WidgetInfo is a base class containing information needed by all GUI widget classes
    // (Buttons, Images, etc...).  It take in a return type to be cast to for accessors.
    
    class WidgetInfo {
      friend Widget;
    protected:
      // Smart-pointer info
      int ptr_count;                  // How many widgets are pointing to this info?

      // Basic info about a widget
      std::string id;                 // ID used for associated element.
      Style style;                    // CSS Style

      // Track hiearchy
      Widget parent;                  // Which widget is this one contained within?
      emp::vector<Widget> children;   // What widgets are contained in this one?
      bool append_ok;                 // Can we add more children?
      bool active;                    // Is this element active in DOM?


      // WidgetInfo cannot be built unless within derived class, so constructor is protected
      WidgetInfo(const std::string & in_id="")
        : ptr_count(1), id(in_id), parent(nullptr), append_ok(true), active(false)
      {
        EMP_TRACK_CONSTRUCT(WebWidgetInfo);
        if (id == "") id = NextWidgetID();
      }

      // No copies of INFO allowed
      WidgetInfo(const WidgetInfo &) = delete;
      WidgetInfo & operator=(const WidgetInfo &) = delete;

      virtual ~WidgetInfo() { EMP_TRACK_DESTRUCT(WebWidgetInfo); }

      void AddChild(Widget in) {
        // If the inserted widget is already active, remove it from its old position.
        if (in->parent) {
          // @CAO Remove inserted widget from old parent
          // @CAO If active, make sure parent is redrawn.
          // @CAO Set inactive.
        }
        emp_assert (!in->active && "Cannot insert a stand-alone active widget!");

        children.emplace_back(in);

        // If the new parent is active, set widget (and children) active and refesh all!
        if (active) {
        }
      }

      // By default, elements should forward unknown appends to their parents.
      virtual Widget Append(const std::string & text) { return ForwardAppend(text); }
      virtual Widget Append(const std::function<std::string()> & fn) { return ForwardAppend(fn); }
      // virtual Widget Append(emp::web::Button info) { return ForwardAppend(info); }
      // virtual Widget Append(emp::web::Canvas info) { return ForwardAppend(info); }
      // virtual Widget Append(emp::web::Image info) { return ForwardAppend(info); }
      // virtual Widget Append(emp::web::Selector info) { return ForwardAppend(info); }
      // virtual Widget Append(emp::web::Slate info) { return ForwardAppend(info); }
      // virtual Widget Append(emp::web::Table info) { return ForwardAppend(info); }
      // virtual Widget Append(emp::web::Text info) { return ForwardAppend(info); }
      virtual Widget Append(Widget info) { return ForwardAppend(info); }

      // Convert arbitrary inputs to a string and try again!
      virtual Widget Append(char in_char) { return Append(emp::to_string(in_char)); }
      virtual Widget Append(double in_num) { return Append(emp::to_string(in_num)); }
      virtual Widget Append(int in_num) { return Append(emp::to_string(in_num)); }
      virtual Widget Append(uint32_t in_num) { return Append(emp::to_string(in_num)); }

      // If an Append doesn't work with current class, forward it to the parent.
      template <typename FWD_TYPE>
      Widget ForwardAppend(FWD_TYPE && arg) {
        emp_assert(parent && "Trying to forward append to parent, but no parent!");
        return parent.info->Append(std::forward<FWD_TYPE>(arg));
      }

      // All derived widgets must suply a mechanism for providing associated HTML code.
      virtual void GetHTML(std::stringstream & ss) = 0;

      // Assume that the associated ID exists and replace it with the currnet HTML code.
      void ReplaceHTML() {
        std::stringstream ss;

        // If this node is active, fill in its contents; otherwise make it an empty span.
        if (active) GetHTML(ss);
        else ss << "<span id=" << id << "></span>";

        // Now do the replacement.
        EM_ASM_ARGS({
            var widget_id = Pointer_stringify($0);
            var out_html = Pointer_stringify($1);
            $('#' + widget_id).replaceWith(out_html);
          }, id.c_str(), ss.str().c_str());
      }
      
    public:
      virtual std::string GetType() { return "web::WidgetInfo"; }

      // Register is used so we can lookup classes by name.
      // Overridden in classes that manage multiple element; below is the default version.
      virtual void Register(Widget & new_widget) { if (parent) parent->Register(new_widget); }
      virtual void Unregister(Widget & old_widget) { if (parent) parent->Unregister(old_widget); }
    
    };


    // Implementation of Widget methods...

    Widget::Widget(const std::string & id) {
      // We are creating a new widget; in derived class, make sure:
      // ... to assign info pointer to new object of proper *Info type
      // ... NOT to increment info->ptr_count since it's initialized to 1.
    }

    Widget::Widget(WidgetInfo * in_info) {
      info = in_info;
      if (info) info->ptr_count++;
    }

    Widget::~Widget() {
      // We are deleting a widget.
      if (info) {
        info->ptr_count--;
        if (info->ptr_count == 0) delete info;
      }
    }

    Widget & Widget::SetInfo(WidgetInfo * in_info) {
      // If the widget is already set correctly, stop here.
      if (info == in_info) return *this;
      
      // Clean up the old info that was previously pointed to.
      if (info) {
        info->ptr_count--;
        if (info->ptr_count == 0) delete info;
      }

      // Setup new info.
      info = in_info;
      if (info) info->ptr_count++;

      return *this;
    }

    bool Widget::IsActive() const {
      if (!info) return false;
      return info->active;
    }

    bool Widget::AppendOK() const { return info->append_ok; }
    std::string Widget::GetID() const { return info ? info->id : ""; }
    std::string Widget::CSS(const std::string & setting) {
      return info ? info->style.Get(setting) : "";
    }
    bool Widget::HasCSS(const std::string & setting) {
      return info ? info->style.Has(setting) : false;
    }

    bool Widget::HasChild(const Widget & test_child) {
      if (!info) return false;
      for (const Widget & c : info->children) if (c == test_child) return true;
      return false;
    }
    
    void Widget::DoActivate() {
      if (!info) return;           // Cannot activate a null widget.

      // Activate this widget
      info->active = true;   // Mark as active.
      info->ReplaceHTML();   // Print full contents to document.

      // Now activate all of this widget's children!
      for (auto & child : info->children) child.DoActivate();
    }
    void Widget::Deactivate(bool top_level) {
      // Skip if we are not active.
      if (!info || !info->active) return;

      info->active = false;

      // Deactivate all of the children (marking them as not at the top level.)
      for (auto & child : info->children) child.Deactivate(false);

      // If we are at the top level, clear the contents by replaceing the HTML.
      info->ReplaceHTML();
    }
    bool Widget::ToggleActive() {
      emp_assert(info);
      if (info->active == true) Deactivate();
      else Activate();
      return info->active;
    }

    template <typename IN_TYPE>
    Widget Widget::operator<<(IN_TYPE && in_val) {
      return info->Append(std::forward<IN_TYPE>(in_val));
    }


    // WidgetFacet is a template that provides accessors into Widget with a derived return type.
    
    template <typename RETURN_TYPE>
    class WidgetFacet : public Widget {
    protected: 
      // WidgetFacet cannot be built unless within derived class, so constructors are protected
      WidgetFacet(const std::string & in_id="") : Widget(in_id) { ; }
      WidgetFacet(const WidgetFacet & in) : Widget(in) { ; }
      WidgetFacet(WidgetInfo * in_info) : Widget(in_info) { ; }
      WidgetFacet & operator=(const WidgetFacet & in) { Widget::operator=(in); return *this; }
      virtual ~WidgetFacet() { ; }

      emp::vector<Widget> & Children() { return info->children; }

    public:
      template <typename SETTING_TYPE>
      RETURN_TYPE & CSS(const std::string & setting, SETTING_TYPE && value) {
        info->style.Set(setting, value);
        return (RETURN_TYPE &) *this;
      }

      // Allow multiple CSS settings to be grouped.
      template <typename T1, typename T2, typename... OTHER_SETTINGS>
      RETURN_TYPE & CSS(const std::string & setting1, T1 && val1,
                        const std::string & setting2, T2 && val2,
                        OTHER_SETTINGS... others) {
        CSS(setting1, val1);                      // Set the first CSS value.
        return CSS(setting2, val2, others...);    // Recurse to the others.
      }

      RETURN_TYPE & ID(const std::string & in_id) {
        info->id = in_id;
        return (RETURN_TYPE &) *this;
      }


      // Size Manipulation
      RETURN_TYPE & Width(double w, const std::string & unit="px") {
        return CSS("width", emp::to_string(w, unit) );
      }
      RETURN_TYPE & Height(double h, const std::string & unit="px") {
        return CSS("height", emp::to_string(h, unit) );
      }
      RETURN_TYPE & Size(double w, double h, const std::string & unit="px") {
        Width(w, unit); return Height(h, unit);
      }

      // Position Manipulation
      RETURN_TYPE & Center() { return CSS("margin", "auto"); }
      RETURN_TYPE & SetPosition(int x, int y, const std::string & unit="px") {
        return CSS("position", "fixed",
                   "left", emp::to_string(x, unit),
                   "top", emp::to_string(y, unit));
      }
      RETURN_TYPE & SetPositionRT(int x, int y, const std::string & unit="px") {
        return CSS("position", "fixed",
                   "right", emp::to_string(x, unit),
                   "top", emp::to_string(y, unit));
      }
      RETURN_TYPE & SetPositionRB(int x, int y, const std::string & unit="px") {
        return CSS("position", "fixed",
                   "right", emp::to_string(x, unit),
                   "bottom", emp::to_string(y, unit));
      }
      RETURN_TYPE & SetPositionLB(int x, int y, const std::string & unit="px") {
        return CSS("position", "fixed",
                   "left", emp::to_string(x, unit),
                   "bottom", emp::to_string(y, unit));
      }


      // Text Manipulation
      RETURN_TYPE & Font(const std::string & font) { return CSS("font-family", font); }
      RETURN_TYPE & FontSize(int s) { return CSS("font-size", emp::to_string(s, "px")); }
      RETURN_TYPE & FontSizeVW(double s) { return CSS("font-size", emp::to_string(s, "vw")); }
      RETURN_TYPE & CenterText() { return CSS("text-align", "center"); }

      // Color Manipulation
      RETURN_TYPE & Background(const std::string & v) { return CSS("background-color", v); }
      RETURN_TYPE & Color(const std::string & v) { return CSS("color", v); }
      RETURN_TYPE & Opacity(double v) { return CSS("opacity", v); }
    };
    
  };

};
};  


#endif

