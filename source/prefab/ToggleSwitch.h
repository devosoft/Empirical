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
                // @param fa_name the font awesome class of the desired icon
                ToggleSwitch(emp::web::Input & checkbox, const std::string & in_name=""): web::Element("span", in_name){
                    *this << checkbox;
                    *this << label;
                    // NOTE: If you added a class to the input before you called this constructor, 
                    // it will be overridden here. Instead add the class after creating this object
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