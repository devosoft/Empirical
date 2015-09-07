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
//  * Change 'active' flag to an activity state enum, which can include:
//      INACTIVE - Widget is not currently part of the DOM.
//      WAITING - As soon as document is ready, widgit will be live.
//      STATIC - Part of DOM, but not immediately updating when changes occur.
//      LIVE - Widget is part of DOM and should be updated immediately on change.
//

#include <string>

#include "../emtools/init.h"
#include "../tools/mem_track.h"
#include "../tools/vector.h"

#include "events.h"
#include "Style.h"

namespace emp {
namespace web {

  // Setup some types we will need later
  namespace internal {
    // Pre-declate WidgetInfo so classes can inter-operate.
    class WidgetInfo;
    class SlateInfo;

    // Quick method for generating unique IDs when not otherwise specified.
    static std::string NextWidgetID() {
      static int next_id = 0;
      return emp::to_string("emp__", next_id++);
    }
    
    // Base class for command-objects that can be fed into widgets.
    class WidgetCommand {
    public:
      virtual bool Trigger(WidgetInfo &) const = 0;
    };
  }
  
  

  // Widget is a smart pointer to a WidgetInfo object, plus some basic accessors.
  class Widget {
    friend internal::WidgetInfo; friend internal::SlateInfo;
  protected:
    using WidgetInfo = internal::WidgetInfo;
    WidgetInfo * info;
    
    // If an Append doesn't work with current class, forward it to the parent.
    template <typename FWD_TYPE> Widget & ForwardAppend(FWD_TYPE && arg);
    
    Widget & SetInfo(WidgetInfo * in_info);
    
    // Internally, we can treat a Widget as a pointer to its WidgetInfo.
    WidgetInfo * operator->() { return info; }
    
    // Give derived classes the ability to access widget info.
    static WidgetInfo * Info(const Widget & w) { return w.info; }
    
  public:
    Widget(const std::string & id);
    Widget(WidgetInfo * in_info=nullptr);
    Widget(const Widget & in) : Widget(in.info) { ; }
    Widget & operator=(const Widget & in) { return SetInfo(in.info); }
    
    enum ActivityState { INACTIVE, WAITING, STATIC, ACTIVE };

    virtual ~Widget();
    
    bool IsNull() const { return info == nullptr; }
    bool IsInactive() const;
    bool IsWaiting() const;
    bool IsStatic() const;
    bool IsActive() const;
    
    virtual bool AppendOK() const;

    virtual bool IsSlate() const { return false; }
    virtual bool IsTable() const { return false; }
    virtual bool IsText() const { return false; }

    // Checks to see if this widget can be converted to other types...
    bool ButtonOK() const;
    bool CanvasOK() const;
    bool ImageOK() const;
    bool SelectorOK() const;
    bool SlateOK() const;
    bool TableOK() const;
    bool TextOK() const;
    
    std::string GetID() const;
    bool HasChild(const Widget & test_child);
    
    // CSS-related options may be overridden in derived classes that have multiple styles.
    virtual std::string GetCSS(const std::string & setting);
    virtual bool HasCSS(const std::string & setting);
    
    bool operator==(const Widget & in) const { return info == in.info; }
    bool operator!=(const Widget & in) const { return info != in.info; }
    operator bool() const { return info != nullptr; }      
    
    // An active widget makes live changes to the webpage (once document is ready)
    // An inactive widget just records changes internally.
    void Activate();
    void Deactivate(bool top_level=true);
    bool ToggleActive();

    // Clear and redraw the current widget on the screen.
    void Redraw();
      
    
    Widget & AddDependant(const Widget & w);
    
    // Setup << operator to redirect to Append; option preparation can be overridden.
    virtual void PrepareAppend() { ; }
    template <typename IN_TYPE> Widget operator<<(IN_TYPE && in_val);
    
    // Debug...
    std::string GetInfoType() const;
  };
  
  namespace internal {

    // WidgetInfo is a base class containing information needed by all GUI widget classes
    // (Buttons, Images, etc...).  It take in a return type to be cast to for accessors.
    
    class WidgetInfo {
    public:
      // Smart-pointer info
      int ptr_count;                  // How many widgets are pointing to this info?

      // Basic info about a widget
      std::string id;                 // ID used for associated element.
      Style style;                    // CSS Style

      // Track hiearchy
      Widget parent;                  // Which widget is this one contained within?
      emp::vector<Widget> children;   // Widgets contained in this one.
      emp::vector<Widget> dependants; // Widgets to be refreshed if this one is triggered
      bool append_ok;                 // Can we add more children?
      Widget::ActivityState state;    // Is this element active in DOM?

      // WidgetInfo cannot be built unless within derived class, so constructor is protected
      WidgetInfo(const std::string & in_id="")
        : ptr_count(1), id(in_id), parent(nullptr), append_ok(true), state(Widget::INACTIVE)
      {
        EMP_TRACK_CONSTRUCT(WebWidgetInfo);
        if (id == "") id = NextWidgetID();
      }

      // No copies of INFO allowed
      WidgetInfo(const WidgetInfo &) = delete;
      WidgetInfo & operator=(const WidgetInfo &) = delete;

      virtual ~WidgetInfo() {
        EMP_TRACK_DESTRUCT(WebWidgetInfo); 
      }

      virtual bool IsButtonInfo() const { return false; }
      virtual bool IsCanvasInfo() const { return false; }
      virtual bool IsImageInfo() const { return false; }
      virtual bool IsSelectorInfo() const { return false; }
      virtual bool IsSlateInfo() const { return false; }
      virtual bool IsTableInfo() const { return false; }
      virtual bool IsTextInfo() const { return false; }
      virtual bool IsTextAreaInfo() const { return false; }

      void AddChild(Widget in) {
        // If the inserted widget is already active, remove it from its old position.
        if (in->parent) {
          emp_assert(!"Currently cannot insert widget if already active!");
          // @CAO Remove inserted widget from old parent
          // @CAO If active, make sure parent is redrawn.
          // @CAO Set inactive.
        }
        emp_assert (in->state != Widget::ACTIVE && "Cannot insert a stand-alone active widget!");

        // Setup parent-child relationship
        children.emplace_back(in);
        in->parent = Widget(this);
        Register(in);

        // If this element (as new parent) is active, anchor widget and activate it!
        if (state == Widget::ACTIVE) {
          // Create a span tag to anchor the new widget.
          EM_ASM_ARGS({
              parent_id = Pointer_stringify($0);
              child_id = Pointer_stringify($1);
              $('#' + parent_id).append('<span id=\'' + child_id + '\'></span>');
            }, id.c_str(), in.GetID().c_str());

          // Now that the new widget has some place to hook in, activate it!
          in->DoActivate();
        }
      }

      // Record dependants.  Dependants are only acted upon when this widget's action is
      // triggered (e.g. a button is pressed)
      void AddDependant(Widget in) {
        dependants.emplace_back(in);
      }

      template <typename... T>
      void AddDependants(Widget first, T... widgets) {
        AddDependant(first);
        AddDependants(widgets...);
      }

      void AddDependants() { ; }

      void UpdateDependants() {
        for (auto & d : dependants) d->ReplaceHTML();
      }

      // Activate is delayed until the document is ready, when DoActivate will be called.
      void DoActivate(bool top_level=true) {
        // Activate this widget and its children.
        state = Widget::ACTIVE;
        for (auto & child : children) child->DoActivate(false);
        
        // Finally, put everything on the screen.
        if (top_level) ReplaceHTML();   // Print full contents to document.
      }

      // By default, elements should forward unknown appends to their parents.
      virtual Widget Append(const std::string & text) { return ForwardAppend(text); }
      virtual Widget Append(const std::function<std::string()> & fn) { return ForwardAppend(fn); }
      virtual Widget Append(Widget info) { return ForwardAppend(info); }

      // Convert arbitrary inputs to a string and try again!
      virtual Widget Append(char in_char) { return Append(emp::to_string(in_char)); }
      virtual Widget Append(double in_num) { return Append(emp::to_string(in_num)); }
      virtual Widget Append(int in_num) { return Append(emp::to_string(in_num)); }
      virtual Widget Append(uint32_t in_num) { return Append(emp::to_string(in_num)); }

      // Handle special commands
      virtual Widget Append(const emp::web::internal::WidgetCommand & cmd) {
        if (cmd.Trigger(*this)) return Widget(this);
        return ForwardAppend(cmd);  // Otherwise pass the Close to parent!
      }


      // If an Append doesn't work with current class, forward it to the parent.
      template <typename FWD_TYPE>
      Widget ForwardAppend(FWD_TYPE && arg) {
        emp_assert(parent && "Trying to forward append to parent, but no parent!", id);
        return parent.info->Append(std::forward<FWD_TYPE>(arg));
      }

      // All derived widgets must suply a mechanism for providing associated HTML code.
      virtual void GetHTML(std::stringstream & ss) = 0;

      // Derived widgets may also provide JavaScript code to be run on redraw.
      virtual void TriggerJS() { ; }

      // Assume that the associated ID exists and replace it with the currnet HTML code.
      void ReplaceHTML() {
        std::stringstream ss;
        
        // If this node is static, don't change it!
        if (state == Widget::STATIC) return;

        // If this node is active, fill put its contents in ss; otherwise make ss an empty span.
        if (state == Widget::ACTIVE) GetHTML(ss);
        else ss << "<span id=" << id << "></span>";
        
        // Now do the replacement.
        EM_ASM_ARGS({
            var widget_id = Pointer_stringify($0);
            var out_html = Pointer_stringify($1);
            $('#' + widget_id).replaceWith(out_html);
          }, id.c_str(), ss.str().c_str());
        
        // If active update style, trigger JS, and recurse to children!
        if (state == Widget::ACTIVE) {
          // Update the style
          style.Apply(id);

          // Run associated Javascript code, if any (e.g., to fill out a canvas)
          TriggerJS();

          for (auto & child : children) child->ReplaceHTML();
        }
      }

    public:
      virtual std::string GetType() { return "web::WidgetInfo"; }

      // Register is used so we can lookup classes by name.
      // Overridden in classes that manage multiple element; below is the default version.
      virtual void Register(Widget & new_widget) { if (parent) parent->Register(new_widget); }
      virtual void Unregister(Widget & old_widget) { if (parent) parent->Unregister(old_widget); }
    
    };

  }  // end namespaceinternal

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

  enum ActivityState { INACTIVE, WAITING, STATIC, ACTIVE };

  bool Widget::IsInactive() const { if (!info) return false; return info->state == INACTIVE; }  
  bool Widget::IsWaiting() const { if (!info) return false; return info->state == WAITING; }
  bool Widget::IsStatic() const { if (!info) return false; return info->state == STATIC; }
  bool Widget::IsActive() const { if (!info) return false; return info->state == ACTIVE; }
  
  bool Widget::AppendOK() const { return info->append_ok; }
  std::string Widget::GetID() const { return info ? info->id : ""; }
  
  bool Widget::HasChild(const Widget & test_child) {
    if (!info) return false;
    for (const Widget & c : info->children) if (c == test_child) return true;
    return false;
  }
  
  bool Widget::ButtonOK() const { if (!info) return false; return info->IsButtonInfo(); }
  bool Widget::CanvasOK() const { if (!info) return false; return info->IsCanvasInfo(); }
  bool Widget::ImageOK() const { if (!info) return false; return info->IsImageInfo(); }
  bool Widget::SelectorOK() const { if (!info) return false; return info->IsSelectorInfo(); }
  bool Widget::SlateOK() const { if (!info) return false; return info->IsSlateInfo(); }
  bool Widget::TableOK() const { if (!info) return false; return info->IsTableInfo(); }
  bool Widget::TextOK() const { if (!info) return false; return info->IsTextInfo(); }

  std::string Widget::GetCSS(const std::string & setting) {
    return info ? info->style.Get(setting) : "";
  }
  bool Widget::HasCSS(const std::string & setting) {
    return info ? info->style.Has(setting) : false;
  }
  
  
  void Widget::Activate() {
    auto * cur_info = info;
    info->state = WAITING;
    OnDocumentReady( std::function<void(void)>([cur_info](){ cur_info->DoActivate(); }) );
  }
  
  void Widget::Deactivate(bool top_level) {
    // Skip if we are not active.
    if (!info || info->state == INACTIVE) return;
    
    info->state = INACTIVE;
    
    // Deactivate all of the children (marking them as not at the top level.)
    for (auto & child : info->children) child.Deactivate(false);
    
    // If we are at the top level, clear the contents by replaceing the HTML.
    if (top_level) info->ReplaceHTML();
  }

  bool Widget::ToggleActive() {
    emp_assert(info);
    if (info->state != INACTIVE) Deactivate();
    else Activate();
    return info->state;
  }

  void Widget::Redraw() {
    emp_assert(info);
    info->ReplaceHTML();
  }
  
  Widget & Widget::AddDependant(const Widget & w) {
    info->AddDependant(w);
    return *this;
  }
  
  template <typename IN_TYPE>
  Widget Widget::operator<<(IN_TYPE && in_val) {
    PrepareAppend();
    return info->Append(std::forward<IN_TYPE>(in_val));
  }

  std::string Widget::GetInfoType() const {
    if (!info) return "UNINITIALIZED";
    return info->GetType();
  }
  
  
  namespace internal {
    // WidgetFacet is a template that provides accessors into Widget with a derived return type.
  
    template <typename RETURN_TYPE>
    class WidgetFacet : public Widget {
    protected: 
      // WidgetFacet cannot be built unless within derived class, so constructors are protected
      WidgetFacet(const std::string & in_id="") : Widget(in_id) { ; }
      WidgetFacet(const WidgetFacet & in) : Widget(in) { ; }
      WidgetFacet(const Widget & in) : Widget(in) {
        // Converting from a generic widget; make sure type is correct!
        emp_assert(dynamic_cast<typename RETURN_TYPE::INFO_TYPE *>( Info(in) ) != NULL);
      }
      WidgetFacet(WidgetInfo * in_info) : Widget(in_info) { ; }
      WidgetFacet & operator=(const WidgetFacet & in) { Widget::operator=(in); return *this; }
      virtual ~WidgetFacet() { ; }

      emp::vector<Widget> & Children() { return info->children; }

      // CSS-related options may be overridden in derived classes that have multiple styles.
      virtual void DoCSS(const std::string & setting, const std::string & value) {
        info->style.DoSet(setting, value);
        if (IsActive()) Style::Apply(info->id, setting, value);
      }

    public:
      template <typename SETTING_TYPE>
      RETURN_TYPE & SetCSS(const std::string & setting, SETTING_TYPE && value) {
        emp_assert(info != nullptr);
        DoCSS(setting, emp::to_string(value));
        return (RETURN_TYPE &) *this;
      }

      // Allow multiple CSS settings to be grouped.
      template <typename T1, typename T2, typename... OTHER_SETTINGS>
      RETURN_TYPE & SetCSS(const std::string & setting1, T1 && val1,
                        const std::string & setting2, T2 && val2,
                        OTHER_SETTINGS... others) {
        SetCSS(setting1, val1);                      // Set the first CSS value.
        return SetCSS(setting2, val2, others...);    // Recurse to the others.
      }

      // Allow multiple CSS settigns as a single style object.
      // (still go through DoCSS given need for virtual re-routing.)
      RETURN_TYPE & SetCSS(const Style & in_style) {
        emp_assert(info != nullptr);
        for (const auto & s : in_style.GetMap()) {
          DoCSS(s.first, s.second);
        }
        return (RETURN_TYPE &) *this;
      }

      // @CAO This seems like a bigger deal than just changing the id...
      // RETURN_TYPE & ID(const std::string & in_id) {
      //   info->id = in_id;
      //   return (RETURN_TYPE &) *this;
      // }


      // Size Manipulation
      RETURN_TYPE & SetWidth(double w, const std::string & unit="px") {
        return SetCSS("width", emp::to_string(w, unit) );
      }
      RETURN_TYPE & SetHeight(double h, const std::string & unit="px") {
        return SetCSS("height", emp::to_string(h, unit) );
      }
      RETURN_TYPE & SetSize(double w, double h, const std::string & unit="px") {
        SetWidth(w, unit); return SetHeight(h, unit);
      }

      // Position Manipulation
      RETURN_TYPE & Center() { return SetCSS("margin", "auto"); }
      RETURN_TYPE & SetPosition(int x, int y, const std::string & unit="px") {
        return SetCSS("position", "fixed",
                      "left", emp::to_string(x, unit),
                      "top", emp::to_string(y, unit));
      }
      RETURN_TYPE & SetPositionRT(int x, int y, const std::string & unit="px") {
        return SetCSS("position", "fixed",
                      "right", emp::to_string(x, unit),
                      "top", emp::to_string(y, unit));
      }
      RETURN_TYPE & SetPositionRB(int x, int y, const std::string & unit="px") {
        return SetCSS("position", "fixed",
                      "right", emp::to_string(x, unit),
                      "bottom", emp::to_string(y, unit));
      }
      RETURN_TYPE & SetPositionLB(int x, int y, const std::string & unit="px") {
        return SetCSS("position", "fixed",
                      "left", emp::to_string(x, unit),
                      "bottom", emp::to_string(y, unit));
      }


      // Text Manipulation
      RETURN_TYPE & SetFont(const std::string & font) { return SetCSS("font-family", font); }
      RETURN_TYPE & SetFontSize(int s) { return SetCSS("font-size", emp::to_string(s, "px")); }
      RETURN_TYPE & SetFontSizeVW(double s) { return SetCSS("font-size", emp::to_string(s, "vw")); }
      RETURN_TYPE & SetCenterText() { return SetCSS("text-align", "center"); }

      // Color Manipulation
      RETURN_TYPE & SetBackground(const std::string & v) { return SetCSS("background-color", v); }
      RETURN_TYPE & SetColor(const std::string & v) { return SetCSS("color", v); }
      RETURN_TYPE & SetOpacity(double v) { return SetCSS("opacity", v); }
    };
    
  }

}
}


#endif

