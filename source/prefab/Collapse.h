#ifndef EMP_COLLAPSE_H
#define EMP_COLLAPSE_H

#include "../web/Div.h"
#include "../tools/string_utils.h"

namespace emp {
    class Collapse {
        private:
            web::Div link;
            web::Div toggle;
        public:
            // Constructor used when both the link and toggle area need to be wrapped
            // @param click - a web element that controls the collapsing area
            // @param ref_toggle - a web element that will appear/disappear
            // @param id - string representing a unique id used to allow communication between link and toggle divs
            // @param expanded - true if collapse is open by default, false otherwise 
            template <typename T, typename S>
            Collapse(T click, S ref_toggle, std::string id, bool expanded){
                toggle = web::Div(id);
                link << click;
                toggle << ref_toggle;
                link.SetAttr(
                    "data-toggle", "collapse",
                    "data-target", "#" + id,
                    "class", "collapse_toggle", 
                    "role", "button",
                    "aria-controls", "#" + id
                );
                if(expanded){
                    link.SetAttr("aria-expanded", "true");
                    toggle.SetAttr("class", "collapse show");
                }
                else{
                    link.SetAttr("aria-expanded", "false");
                    toggle.SetAttr("class", "collapse");
                }
            }
            // Constructor used when only the link area needs to be wrapped
            // i.e. there is more than one link to the same toggle area or when adding a link to collapse a card 
            // @param click - a web element that controls the collapsing area
            // @param id - string representing a unique id used to allow communication between link and toggle divs
            // @param expanded - true if collapse is open by default, false otherwise 
            template <typename T>
            Collapse(T click, std::string id, bool expanded){
                link << click;
                link.SetAttr(
                    "data-toggle", "collapse",
                    "data-target", "#" + id,
                    "class", "collapse_toggle", 
                    "role", "button",
                    "aria-controls", "#" + id
                );
                if(expanded){
                    link.SetAttr("aria-expanded", "true");
                }
                else{
                    link.SetAttr("aria-expanded", "false");
                }
            }
            web::Div & GetLinkDiv() {return link;}
            web::Div & GetToggleDiv() {return toggle;}
    };
}

#endif