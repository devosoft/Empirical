#ifndef EMP_CARD_INTERFACE_H
#define EMP_CARD_INTERFACE_H

#include "../web/Div.h"
#include "../tools/string_utils.h"
#include "Collapse.h"
#include "FontAwesomeIcon.h"

namespace emp {
    namespace prefab{
        class Card : public web::Div{
            private:
                web::Div collapse_link;
                web::Div collapse_body;
                std::string card_base = this->GetID();
                web::Div card_header{emp::to_string(card_base, "_card_header")};
                web::Div card_body{emp::to_string(card_base, "_card_body")};
                void AddBootstrap(){
                    this->SetAttr("class", "card");
                    card_header.SetAttr("class", "card-header");
                    card_body.SetAttr("class", "card-body");
                    card_header.AddClass("collapse_toggle_card_header");
                }
            public:
                enum Collapse {NONE, OPEN, CLOSED}; // collapse options for constructing a card 
                Card(Collapse state=NONE, bool showGlyphs=true, const std::string & in_name=""): web::Div(in_name){
                    if(state != NONE){ // if card is collapsible, make the collapse link the head of the card
                        if(state == OPEN){ 
                            prefab::Collapse accordion(card_header, card_body, true, emp::to_string(card_base+ "_card_collapse"));
                            collapse_link = accordion.GetLinkDiv();
                            collapse_body = accordion.GetToggleDiv();
                        }
                        else{
                            prefab::Collapse accordion(card_header, card_body, false, emp::to_string(card_base + "_card_collapse"));
                            collapse_link = accordion.GetLinkDiv();
                            collapse_body = accordion.GetToggleDiv();
                        }
                        if(showGlyphs){ // by default add glyphs to a collapsible card
                        // TODO: Don't use float, just set it to align to the right (need to make icon decend from div first)
                            prefab::FontAwesomeIcon up("fa-angle-double-up");
                            prefab::FontAwesomeIcon down("fa-angle-double-down");
                            card_header << up;
                            card_header << down;
                            up.AddClass("float-right btn-link collapse_toggle collapse_toggle_card_header setting_heading");
                            down.AddClass("float-right btn-link collapse_toggle collapse_toggle_card_header setting_heading");

                        }
                        *this << collapse_link;
                        *this << collapse_body;
                    }
                    else{ // plain card with no toggle enabled
                        *this << card_header;
                        *this << card_body;
                    }
                    AddBootstrap();
                }
                template <typename T>
                void AddHeaderContent(T val, bool link_content=false){
                    if(link_content){ 
                        // add bootstrap link properities to content (hover, underline, etc.), 
                        // but does not have a target or href because it is assumed that
                        // this content will control the card collapse, which is done in the
                        // constructor. If you want the content to link somewhere else, 
                        // specify that in the web element/div param val {and set link_content=true TODO: verify this}.
                        web::Div btn_link;
                        btn_link.SetAttr("class", "btn-link");
                        card_header << btn_link << val;
                    }
                    else{
                        card_header << val;
                    }
                }
                template <typename T>
                void AddBodyContent(T val){
                    card_body << val;
                }
                // TODO: override operator<< so that it streams into body content
        };
    }
}

#endif