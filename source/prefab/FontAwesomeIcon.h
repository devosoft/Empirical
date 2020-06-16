#ifndef EMP_FONT_AWESOME_ICON_H
#define EMP_FONT_AWESOME_ICON_H

#include "../web/Element.h"
#include "../tools/string_utils.h"

namespace emp {
    class FontAwesomeIcon {
        private:
            web::Element icon{emp::to_string("span")};
        public:
            // @param fa_name the font awesome class of the desired icon and any other necessary CSS class
            FontAwesomeIcon(std::string fa_name){
                std::string full_class = emp::to_string("fa ", fa_name);
                std::cout << full_class << std::endl;
                icon.SetAttr("class", full_class);
                }
            web::Div & GetDiv() {return icon;}
    };
}

#endif