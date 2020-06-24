#ifndef EMP_FONT_AWESOME_ICON_H
#define EMP_FONT_AWESOME_ICON_H

#include "../web/Element.h"
#include "../web/Div.h"
#include "../web/Widget.h"
#include "../tools/string_utils.h"

namespace emp {
    namespace prefab {
        class FontAwesomeIcon: public web::Element {
            public:
                // @param fa_name the font awesome class of the desired icon
                FontAwesomeIcon(std::string fa_name, const std::string & in_name=""): web::Element("span", in_name){
                    std::string full_class = emp::to_string("fa ", fa_name);
                    std::cout << full_class << std::endl;
                    this->SetAttr("class", full_class);
                    }
        };
    }
}

#endif