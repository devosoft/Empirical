#ifndef EMP_TOGGLE_SWITCH_H
#define EMP_TOGGLE_SWITCH_H

#include "../web/Element.h"
#include "../web/Input.h"
#include "../web/Widget.h"
#include "../tools/string_utils.h"

namespace emp {
namespace prefab {
  class ToggleSwitch: public web::Element {
    private:
      // Label for switch
      web::Element label{emp::to_string("label")};
    public:
      /// @param in_cb callback function passed onto input constructor
      /// @param in_label text for switch label
      /// @param is_checked is the switched defaulted to on?
      ToggleSwitch(const std::function<void(std::string)> & in_cb,
      const std::string & in_label, bool is_checked=false, const std::string & t_switch_id="")
        : web::Element("span", t_switch_id){

        // When constructing checkbox input, do not use built in label
        // functionality because for toggle switch classes to work,
        // the label element must come after input element. Label element is
        // placed before input element in Input constructor.
        web::Input checkbox(in_cb, "checkbox", "", "", false, is_checked);
        *this << checkbox;
        *this << label;
        if (in_label != ""){
          label << in_label;
        }
        checkbox.SetAttr("class", "custom-control-input");
        this->SetAttr("class", "custom-control custom-switch");
        this->SetCSS(
          "clear", "none",
          "display", "inline"
        );
        label.SetAttr(
          "class", "custom-control-label",
          "for", checkbox.GetID()
        );
      }

      /// Add label to switch after constructor
      template <typename T>
      void AddLabel(const T usr_label){
        label << usr_label;
      }
  };
}
}

#endif