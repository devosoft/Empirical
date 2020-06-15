#ifndef EMP_COMMENT_BOX_H
#define EMP_COMMENT_BOX_H

#include "../config/config.h"
#include "../web/Div.h"
#include "../web/Element.h"
#include "../web/Input.h"
#include "../tools/string_utils.h"

namespace emp {
    class CommentBox {
        private:
            web::Div box_element;
            std::string box_base = box_element.GetID();
            web::Div triangle{emp::to_string(box_base, "_triangle")};
            web::Div all_content{emp::to_string(box_base, "_all_content")};
            web::Div desktop_content{emp::to_string(box_base, "_desktop_content")};
            web::Div mobile_content{emp::to_string(box_base, "_mobile_content")};
        public:
            CommentBox(){
                box_element << triangle;
                box_element << all_content;
                all_content << desktop_content;
                all_content << mobile_content;
                //TODO: change class names to reflect comment box
                triangle.SetAttr("class", "dropdown_triangle");
                all_content.SetAttr("class", "dropdown_content");
                mobile_content.SetAttr("class", "mobile_dropdown");
                }
            template <typename T>
            void AddContent(T val){
                desktop_content << val;
            }
            template <typename T>
            void AddMobileContent(T val){
                mobile_content << val;
            }
            web::Div & GetDiv() {return box_element;}
    };
}

#endif