//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
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
//  Library users should not need to access Widgets directly, only specific derived types.
//
//  Tips for using widgets:
//
//  * If you are about to make a lot of changes at once, run Freeze(), make the changes, and
//    then run Activate() again.  Freeze prevents widgets from being updated immediately.


#ifndef EMP_WEB_WIDGET_H
#define EMP_WEB_WIDGET_H

#include <string>

#include "../base/vector.h"
#include "../tools/mem_track.h"

#include "Attributes.h"
#include "events.h"
#include "init.h"
#include "Listeners.h"
#include "Style.h"

namespace emp {
namespace web {

  // Setup some types we will need later
  namespace internal {
    // Pre-declate WidgetInfo so classes can inter-operate.
    class WidgetInfo;
    class SlateInfo;
    class TableInfo;

    // Quick method for generating unique IDs when not otherwise specified.
    static int NextWidgetNum(bool inc_num=true) {
      static int next_id = 0;
      if (!inc_num) return next_id;
      return next_id++;
    }
    static std::string NextWidgetID() {
      return emp::to_string("emp__", NextWidgetNum());
    }

    // Base class for command-objects that can be fed into widgets.
    class WidgetCommand {
    public:
      virtual ~WidgetCommand() { ; }
      virtual bool Trigger(WidgetInfo &) const = 0;
    };
  }



  // Widget is a smart pointer to a WidgetInfo object, plus some basic accessors.
  class Widget {
    friend internal::WidgetInfo; friend internal::SlateInfo; friend internal::TableInfo;
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

    // Four activity states for any widget.
    //   INACTIVE - Not be in DOM at all.
    //   WAITING  - Will become active once the page finishes loading.
    //   FROZEN   - Part of DOM, but not updating on the screen.
    //   ACTIVE   - Fully active; changes are reflected as they happen.

    enum ActivityState { INACTIVE, WAITING, FROZEN, ACTIVE };

  public:
    Widget(const std::string & id);
    Widget(WidgetInfo * in_info=nullptr);
    Widget(const Widget & in) : Widget(in.info) { ; }
    Widget & operator=(const Widget & in) { return SetInfo(in.info); }

    virtual ~Widget();

    bool IsNull() const { return info == nullptr; }

    // Some debugging helpers...
    std::string InfoTypeName() const;

    bool IsInactive() const;
    bool IsWaiting() const;
    bool IsFrozen() const;
    bool IsActive() const;

    bool AppendOK() const;
    void PreventAppend();

    // Checks to see if this widget can be trivially converted to other types...
    bool ButtonOK() const;
    bool CanvasOK() const;
    bool ImageOK() const;
    bool SelectorOK() const;
    bool SlateOK() const;
    bool TableOK() const;
    bool TextOK() const;

    bool IsSlate() const { return SlateOK(); }
    bool IsTable() const { return TableOK(); }
    bool IsText() const { return TextOK(); }

    std::string GetID() const;

    // CSS-related options may be overridden in derived classes that have multiple styles.
    virtual std::string GetCSS(const std::string & setting);
    virtual bool HasCSS(const std::string & setting);

    // And other attribute-related options
    virtual std::string GetAttr(const std::string & setting);
    virtual bool HasAttr(const std::string & setting);

    bool operator==(const Widget & in) const { return info == in.info; }
    bool operator!=(const Widget & in) const { return info != in.info; }

    operator bool() const { return info != nullptr; }

    // An active widget makes live changes to the webpage (once document is ready)
    // An inactive widget just records changes internally.
    void Activate();
    void Freeze();
    virtual void Deactivate(bool top_level=true);
    bool ToggleActive();

    // Clear and redraw the current widget on the screen.
    void Redraw();

    // Methods to look up previously created elements, by type.
    Widget & Find(const std::string & test_name);

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
      std::string id;                 // ID used for associated DOM element.
      Style style;                    // CSS Style
      Attributes attr;                // HTML Attributes about a class.
      Listeners listen;               // Listen for web events

      // Track hiearchy
      WidgetInfo * parent;            // Which WidgetInfo is this one contained within?
      emp::vector<Widget> dependants; // Widgets to be refreshed if this one is triggered
      Widget::ActivityState state;    // Is this element active in DOM?


      // WidgetInfo cannot be built unless within derived class, so constructor is protected
      WidgetInfo(const std::string & in_id="")
        : ptr_count(1), id(in_id), parent(nullptr), state(Widget::INACTIVE)
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

      // Some debugging helpers...
      virtual std::string TypeName() const { return "WidgetInfo base"; }

      virtual bool IsButtonInfo() const { return false; }
      virtual bool IsCanvasInfo() const { return false; }
      virtual bool IsImageInfo() const { return false; }
      virtual bool IsSelectorInfo() const { return false; }
      virtual bool IsSlateInfo() const { return false; }
      virtual bool IsTableInfo() const { return false; }
      virtual bool IsTextInfo() const { return false; }
      virtual bool IsTextAreaInfo() const { return false; }
      virtual bool IsD3VisualiationInfo() const { return false; }

      // If not overloaded, pass along widget registration to parent.
      virtual void Register_recurse(Widget & w) { if (parent) parent->Register_recurse(w); }
      virtual void Register(Widget & w) { if (parent) parent->Register(w); }
      virtual void Unregister_recurse(Widget & w) { if (parent) parent->Unregister_recurse(w); }
      virtual void Unregister(Widget & w) { if (parent) parent->Unregister(w); }

      // Some nodes can have children and need to be able to recursively register them.
      virtual void RegisterChildren(SlateInfo * registrar) { ; }   // No children by default.
      virtual void UnregisterChildren(SlateInfo * regestrar) { ; } // No children by default.

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

      void UpdateDependants() { for (auto & d : dependants) d->ReplaceHTML(); }


      // Activate is delayed until the document is ready, when DoActivate will be called.
      virtual void DoActivate(bool top_level=true) {
        state = Widget::ACTIVE;         // Activate this widget and its children.
        if (top_level) ReplaceHTML();   // Print full contents to document.
      }

      virtual bool AppendOK() const { return false; } // Most widgets can't be appended to.
      virtual void PreventAppend() { emp_assert(false, TypeName()); } // Only for appendable widgets.

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
        return parent->Append(std::forward<FWD_TYPE>(arg));
      }

      // All derived widgets must suply a mechanism for providing associated HTML code.
      virtual void GetHTML(std::stringstream & ss) = 0;

      // Derived widgets may also provide JavaScript code to be run on redraw.
      virtual void TriggerJS() { ; }

      // Assume that the associated ID exists and replace it with the current HTML code.
      virtual void ReplaceHTML() {
        // If this node is frozen, don't change it!
        if (state == Widget::FROZEN) return;

        // If this node is active, fill put its contents in ss; otherwise make ss an empty span.
        std::stringstream ss;
        if (state == Widget::ACTIVE) GetHTML(ss);
        else ss << "<span id='" << id << "'></span>";

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
          attr.Apply(id);
          listen.Apply(id);

          // Run associated Javascript code, if any (e.g., to fill out a canvas)
          TriggerJS();
        }
      }

    public:
      virtual std::string GetType() { return "web::WidgetInfo"; }

    };

  }  // end namespaceinternal

  // Implementation of Widget methods...

  Widget::Widget(const std::string & id) {
    // We are creating a new widget; in derived class, make sure:
    // ... to assign info pointer to new object of proper *Info type
    // ... NOT to increment info->ptr_count since it's initialized to 1.
    EMP_TRACK_CONSTRUCT(WebWidget);
  }

  Widget::Widget(WidgetInfo * in_info) {
    info = in_info;
    if (info) info->ptr_count++;
    EMP_TRACK_CONSTRUCT(WebWidget);
  }

  Widget::~Widget() {
    // We are deleting a widget.
    if (info) {
      info->ptr_count--;
      if (info->ptr_count == 0) delete info;
    }
    EMP_TRACK_DESTRUCT(WebWidget);
  }

  std::string Widget::InfoTypeName() const { if (IsNull()) return "NULL"; return info->TypeName(); }

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

  bool Widget::IsInactive() const { if (!info) return false; return info->state == INACTIVE; }
  bool Widget::IsWaiting() const { if (!info) return false; return info->state == WAITING; }
  bool Widget::IsFrozen() const { if (!info) return false; return info->state == FROZEN; }
  bool Widget::IsActive() const { if (!info) return false; return info->state == ACTIVE; }

  bool Widget::AppendOK() const { if (!info) return false; return info->AppendOK(); }
  void Widget::PreventAppend() { emp_assert(info); info->PreventAppend(); }

  std::string Widget::GetID() const { return info ? info->id : "(none)"; }

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

  std::string Widget::GetAttr(const std::string & setting) {
    return info ? info->attr.Get(setting) : "";
  }
  bool Widget::HasAttr(const std::string & setting) {
    return info ? info->attr.Has(setting) : false;
  }


  void Widget::Activate() {
    auto * cur_info = info;
    info->state = WAITING;
    OnDocumentReady( std::function<void(void)>([cur_info](){ cur_info->DoActivate(); }) );
  }

  void Widget::Freeze() {
    info->state = FROZEN;
  }

  void Widget::Deactivate(bool top_level) {
    if (!info || info->state == INACTIVE) return;  // Skip if we are not active.
    info->state = INACTIVE;
    if (top_level) info->ReplaceHTML();            // If at top level, clear the contents
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
        // Converting from a generic widget; make sure type is correct or non-existant!
        emp_assert(!in || dynamic_cast<typename RETURN_TYPE::INFO_TYPE *>( Info(in) ) != NULL,
                   in.GetID());
      }
      WidgetFacet(WidgetInfo * in_info) : Widget(in_info) { ; }
      WidgetFacet & operator=(const WidgetFacet & in) { Widget::operator=(in); return *this; }
      virtual ~WidgetFacet() { ; }

      // CSS-related options may be overridden in derived classes that have multiple styles.
      virtual void DoCSS(const std::string & setting, const std::string & value) {
        info->style.DoSet(setting, value);
        if (IsActive()) Style::Apply(info->id, setting, value);
      }
      // Attr-related options may be overridden in derived classes that have multiple attributes.
      virtual void DoAttr(const std::string & setting, const std::string & value) {
        info->attr.DoSet(setting, value);
        if (IsActive()) Attributes::Apply(info->id, setting, value);
      }

    public:
      template <typename SETTING_TYPE>
      RETURN_TYPE & SetCSS(const std::string & setting, SETTING_TYPE && value) {
        emp_assert(info != nullptr);
        DoCSS(setting, emp::to_string(value));
        return (RETURN_TYPE &) *this;
      }
      template <typename SETTING_TYPE>
      RETURN_TYPE & SetAttr(const std::string & setting, SETTING_TYPE && value) {
        emp_assert(info != nullptr);
        DoAttr(setting, emp::to_string(value));
        return (RETURN_TYPE &) *this;
      }
      RETURN_TYPE & On(const std::string & event_name, const std::function<void()> & fun) {
        emp_assert(info != nullptr);
        info->listen.Set(event_name, fun);
        return (RETURN_TYPE &) *this;
      }
      RETURN_TYPE & On(const std::string & event_name,
                       const std::function<void(MouseEvent evt)> & fun) {
        emp_assert(info != nullptr);
        info->listen.Set(event_name, fun);
        return (RETURN_TYPE &) *this;
      }

      // Allow multiple CSS or Attr settings to be grouped.
      template <typename T1, typename T2, typename... OTHER_SETTINGS>
      RETURN_TYPE & SetCSS(const std::string & setting1, T1 && val1,
                        const std::string & setting2, T2 && val2,
                        OTHER_SETTINGS... others) {
        SetCSS(setting1, val1);                      // Set the first CSS value.
        return SetCSS(setting2, val2, others...);    // Recurse to the others.
      }
      template <typename T1, typename T2, typename... OTHER_SETTINGS>
      RETURN_TYPE & SetAttr(const std::string & setting1, T1 && val1,
                            const std::string & setting2, T2 && val2,
                            OTHER_SETTINGS... others) {
        SetAttr(setting1, val1);                      // Set the first CSS value.
        return SetAttr(setting2, val2, others...);    // Recurse to the others.
      }

      // Allow multiple CSS or Attr settings as a single object.
      // (still go through DoCSS/DoAttr given need for virtual re-routing.)
      RETURN_TYPE & SetCSS(const Style & in_style) {
        emp_assert(info != nullptr);
        for (const auto & s : in_style.GetMap()) {
          DoCSS(s.first, s.second);
        }
        return (RETURN_TYPE &) *this;
      }
      RETURN_TYPE & SetAttr(const Attributes & in_attr) {
        emp_assert(info != nullptr);
        for (const auto & a : in_attr.GetMap()) {
          DoAttr(a.first, a.second);
        }
        return (RETURN_TYPE &) *this;
      }


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


      // Positioning
      RETURN_TYPE & SetFloat(const std::string & f) { return SetCSS("float", f); }
      RETURN_TYPE & SetOverflow(const std::string & o) { return SetCSS("overflow", o); }

      // Text Manipulation
      RETURN_TYPE & SetFont(const std::string & font) { return SetCSS("font-family", font); }
      RETURN_TYPE & SetFontSize(int s) { return SetCSS("font-size", emp::to_string(s, "px")); }
      RETURN_TYPE & SetFontSizeVW(double s) { return SetCSS("font-size", emp::to_string(s, "vw")); }
      RETURN_TYPE & SetCenterText() { return SetCSS("text-align", "center"); }

      // Color Manipulation
      RETURN_TYPE & SetBackground(const std::string & v) { return SetCSS("background-color", v); }
      RETURN_TYPE & SetColor(const std::string & v) { return SetCSS("color", v); }
      RETURN_TYPE & SetOpacity(double v) { return SetCSS("opacity", v); }

      // Tables...
      RETURN_TYPE & SetBorder(const std::string & border_info) {
        return SetCSS("border", border_info);
      }
      RETURN_TYPE & SetPadding(double p, const std::string & unit="px") {
        return SetCSS("padding", emp::to_string(p, unit));
      }
    };

  }

}
}


#endif
