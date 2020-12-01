#ifndef EMP_TOGGLE_SWITCH_HPP
#define EMP_TOGGLE_SWITCH_HPP

#include "../tools/string_utils.hpp"
#include "../web/Element.hpp"
#include "../web/Input.hpp"
#include "../web/Widget.hpp"

namespace emp {
namespace prefab {
  class ToggleSwitch: public web::Element {
    private:
      // Label for switch
      web::Element label_element{emp::to_string("label")};
    public:
      /**
       * @param cb callback function passed onto input constructor
       * @param label text for switch label
       * @param is_checked is the switched defaulted to on?
       */
      ToggleSwitch(
        const std::function<void(std::string)> & cb,
        const std::string & label,
        const bool is_checked=false,
        const std::string & t_switch_id="")
        : web::Element("span", t_switch_id) {

        /*
         * When constructing checkbox input, do not use built in label
         * functionality because for toggle switch classes to work,
         * the label element must come after input element. Label element is
         * placed before input element in Input constructor.
         */
        web::Input checkbox(cb, "checkbox", "", "", false, is_checked);
        *this << checkbox;
        *this << label_element;
        if (label != "") {
          label_element << label;
        }
        checkbox.SetAttr("class", "custom-control-input");
        this->SetAttr("class", "custom-control custom-switch");
        this->SetCSS(
          "clear", "none",
          "display", "inline"
        );
        label_element.SetAttr(
          "class", "custom-control-label",
          "for", checkbox.GetID()
        );
      }

      /// Add label to switch after constructor
      template <typename T>
      void AddLabel(const T usr_label) {
        label_element << usr_label;
      }
  };
}
}

#endif
