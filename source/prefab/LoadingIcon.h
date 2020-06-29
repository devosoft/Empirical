#ifndef EMP_LOADING_ICON_H
#define EMP_LOADING_ICON_H

#include "../web/Element.h"
#include "../web/Div.h"
#include "../web/Widget.h"
#include "../tools/string_utils.h"
#include "../base/errors.h"

namespace emp {
    namespace prefab {
        class LoadingIcon: public web::Element {
            private:
                web::Element icon{emp::to_string("span")};
                web::Element text{emp::to_string("span")};
            public:
                LoadingIcon(const std::string & in_name=""): web::Element("span", in_name){
                    *this << icon;
                    *this << text;
                    icon.SetAttr("class", "fa fa-spinner fa-pulse fa-3x fa-fw");
                    text.SetAttr("class", "sr-only");
                    text << "Loading...";
                }
                // TODO: << operator throw error
                // template <typename T>
                // void operator<<(T invalid){
                //     emp::LibraryError("Not allowed to add code to the loading icon");
                // }
        };
    }
}

#endif