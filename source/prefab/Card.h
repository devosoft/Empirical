#ifndef EMP_CARD_INTERFACE_H
#define EMP_CARD_INTERFACE_H

#include "../web/Div.h"
#include "../tools/string_utils.h"
#include "Collapse.h"
#include "FontAwesomeIcon.h"

namespace emp {
    class Card {
        private:
            web::Div card;
            web::Div collapse_link;
            web::Div collapse_body;
            std::string card_base = card.GetID();
            web::Div card_header{emp::to_string(card_base, "_card_header")};
            web::Div card_body{emp::to_string(card_base, "_card_body")};
            void AddBootstrap(){
                card.SetAttr("class", "card");
                card_header.SetAttr("class", "card-header");
                card_body.SetAttr("class", "card-body");
            }
        public:
            enum Collapse {none, open, closed}; // collapse options for constructing a card
            Card(Collapse state=none, bool showGlyphs=true){
                if(state != none){ // if card is collapsible, make the collapse link the head of the card
                    if(state == open){ 
                        emp::Collapse accordion(card_header, card_body, emp::to_string(card_base + "_card_collapse"), true);
                        collapse_link = accordion.GetLinkDiv();
                        collapse_body = accordion.GetToggleDiv();
                    }
                    else{
                        emp::Collapse accordion(card_header, card_body, emp::to_string(card_base + "_card_collapse"), false);
                        collapse_link = accordion.GetLinkDiv();
                        collapse_body = accordion.GetToggleDiv();
                    }
                    if(showGlyphs){ // by default add glyphs to a collapsible card
                        emp::FontAwesomeIcon up("fa-angle-double-up float-right collapse_toggle btn-link");
                        emp::FontAwesomeIcon down("fa-angle-double-down float-right collapse_toggle btn-link");
                        card_header << up.GetDiv();
                        card_header << down.GetDiv();

                    }
                    card << collapse_link;
                    card << collapse_body;
                }
                else{ // plain card with no toggle enabled
                    card << card_header;
                    card << card_body;
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
            web::Div & GetDiv() {return card;}
            web::Div GetBodyDiv() {return card_body;}
    };
}

#endif