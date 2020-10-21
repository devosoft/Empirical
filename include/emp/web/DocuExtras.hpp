/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2019
 *
 *  @file  DocuExtras.hpp
 *  @brief Control the styling and attributes of an existing div without nuking
*   the content inside it.
 *
 */

#ifndef EMP_WEB_DOCUEXTRAS_H
#define EMP_WEB_DOCUEXTRAS_H

#include "Animate.hpp"
#include "Text.hpp"
#include "Widget.hpp"

#include "init.hpp"

namespace emp {
namespace web {

  /// A widget to track a div in an HTML file, and all of its contents.
  class DocuExtras : private Widget {

  protected:

    class DocuExtrasInfo : public internal::WidgetInfo {

      friend DocuExtras;

      protected:

      DocuExtrasInfo(const std::string & in_id)
        : internal::WidgetInfo(in_id)
      { emp::Initialize(); }

      // No copies of INFO allowed
      DocuExtrasInfo(const DocuExtrasInfo &) = delete;

        // No copies of INFO allowed
      DocuExtrasInfo & operator=(const DocuExtrasInfo &) = delete;

      virtual ~DocuExtrasInfo() { ; }

      virtual void GetHTML(std::stringstream & HTML) override {
         // this should never get called because we don't want to nuke
         // pre-existing HTML inside the target div
        emp_assert(false);
      }

      std::string GetTypeName() const override { return "DocuExtrasInfo"; }

      void Clear() {
        extras.Clear();
      }

    public:
      virtual std::string GetType() override { return "web::DocuExtrasInfo"; }

    }; // end of InputInfo definition

    // Get a properly cast version of indo.
    DocuExtrasInfo * Info() { return (DocuExtrasInfo *) info; }
    const DocuExtrasInfo * Info() const { return (DocuExtrasInfo *) info; }

    /// Apply all HTML details associated with this widget.
    void Apply() { Info()->extras.Apply(Info()->id); }

  public:
    DocuExtras(const std::string & in_name) : Widget(in_name) {
      // When a name is provided, create an associated Widget info.
      info = new DocuExtrasInfo(in_name);
    }

    DocuExtras(const DocuExtras & in) : Widget(in) { ; }
    // DocuExtras(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsDiv()); }

    ~DocuExtras() { ; }

    template <typename SET_TYPE>
    DocuExtras& SetCSS(const std::string & s, SET_TYPE v) {
      Info()->extras.SetStyle<SET_TYPE>(s, v);
      Apply();
      return *this;
    }

    bool HasCSS(const std::string & setting) /* const */ {
      return Info()->extras.HasStyle(setting);
    }

    const std::string & GetCSS(const std::string & setting) const {
      return Info()->extras.GetStyle(setting);
    }

    DocuExtras& RemoveCSS(const std::string & setting) {
      Info()->extras.RemoveStyle(setting);
      Apply();
      return *this;
    }

    template <typename SET_TYPE>
    void SetAttr(const std::string & s, SET_TYPE v) {
      Info()->extras.SetAttr<SET_TYPE>(s, v);
      Apply();
    }

    bool HasAttr(const std::string & setting) /* const */ {
      return Info()->extras.HasAttr(setting);
    }

    const std::string & GetAttr(const std::string & setting) const {
      return Info()->extras.GetAttr(setting);
    }

    DocuExtras& RemoveAttr(const std::string & setting) {
      Info()->extras.RemoveAttr(setting);
      Apply();
      return *this;
    }

    /// Clear all of style, attributes, and listeners.
    DocuExtras& Clear() { Info()->extras.Clear(); Apply(); return *this; }

    /// Have any details been set?
    operator bool() const { return static_cast<bool>(Info()->extras); }

  };

}
}

#endif
