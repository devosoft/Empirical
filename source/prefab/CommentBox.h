#ifndef EMP_COMMENT_BOX_H
#define EMP_COMMENT_BOX_H

#include "../web/Div.h"
#include "../tools/string_utils.h"

/* Notes for how to use

include "prefab/CommentBox.h"

for (int i=0; i < 2; i++){
    emp::CommentBox box; 
    box.AddContent("Box ");
    box.AddContent(i);
    doc << box.GetDiv();
  }

emp::CommentBox box;
UI::Div title("desktop_content");
title << "<h1>Shows in Desktop view</h1>";
UI::Element mobile("span");
mobile << "<p>Here is hidden info</p>";
mobile << "<hr>";
box.AddContent(title);
box.AddMobileContent(mobile);
box.AddMobileContent("<b>Even more hidden info!</b>");
doc << box.GetDiv();

*/

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
        };
    }
}

#endif