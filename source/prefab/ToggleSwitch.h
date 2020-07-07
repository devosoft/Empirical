#ifndef EMP_TOGGLE_SWITCH_H
#define EMP_TOGGLE_SWITCH_H

#include "../web/Element.h"
#include "../web/Input.h"
#include "../web/Widget.h"
#include "../tools/string_utils.h"

// NOTE: Need to link to Bootstrap 4.5.0 or newer to view the stylized toggle switch
// <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css">

namespace emp {
    namespace prefab {
        class ToggleSwitch: public web::Element {
            private:
                web::Element label{emp::to_string("label")};
            public:
                ToggleSwitch(const std::function<void(std::string)> & in_cb, const std::string & in_type,
          const std::string & in_label, bool is_checked=false, const std::string & t_switch_id=""): web::Element("span", t_switch_id){
                    // When constructing checkbox input, do not use built in label
                    // functionality because for toggle switch classes to work,
                    // the label element must come after input element. Label element is
                    // placed before input element in Input constructor.
                    web::Input checkbox(in_cb, in_type, NULL, "", false, is_checked);
                    *this << checkbox;
                    *this << label;
                    label << in_label;
                    // NOTE: If you added a class to the input before you called this constructor, 
                    // it will be overridden here. Instead, use AddClass after creating this object
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
                template <typename T>
                void AddLabel(const T usr_label){
                    label << usr_label;
                }
        };
    }
}

#endif