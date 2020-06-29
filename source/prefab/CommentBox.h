#ifndef EMP_COMMENT_BOX_H
#define EMP_COMMENT_BOX_H

#include "../web/Div.h"
#include "../tools/string_utils.h"

namespace emp {
    namespace prefab{
        class CommentBox: public web::Div {
            private:
                std::string box_base = this->GetID();
                web::Div triangle{emp::to_string(box_base, "_triangle")};
                web::Div all_content{emp::to_string(box_base, "_all_content")};
                web::Div desktop_content{emp::to_string(box_base, "_desktop_content")};
                web::Div mobile_content{emp::to_string(box_base, "_mobile_content")};
            public:
                CommentBox(std::string id=""): web::Div(id){
                    *this << triangle;
                    *this << all_content;
                    all_content << desktop_content;
                    all_content << mobile_content;
                    
                    triangle.SetAttr("class", "commentbox_triangle");
                    all_content.SetAttr("class", "commentbox_content");
                    mobile_content.SetAttr("class", "mobile_commentbox");
                    }
                template <typename T>
                void AddContent(T val){
                    desktop_content << val;
                }
                template <typename T>
                void AddMobileContent(T val){
                    mobile_content << val;
                }
        };
    }
}

#endif