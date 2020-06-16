#ifndef EMP_CARD_INTERFACE_H
#define EMP_CARD_INTERFACE_H

#include "../web/Div.h"
#include "../tools/string_utils.h"

namespace emp {
    class Card {
        private:
            web::Div card;
            std::string card_base = card.GetID();
            web::Div card_header{emp::to_string(card_base, "_card_header")};
            web::Div card_body{emp::to_string(card_base, "_card_body")};
            void AddBootstrap(){
                card.SetAttr("class", "card");
                card_header.SetAttr("class", "card-header");
                card_body.SetAttr("class", "card-body");
            }
        public:
            // Constructor for a plain card
            Card(){
                card << card_header;
                card << card_body;
                AddBootstrap();
                }
            // Constructor for a card with a collapsible body
            // @param collapse_id (string) - the id for the data-target of the collapse link
            // @param expanded (boolean) - true if collapse is open by default, false otherwise
            Card(std::string collapse_id, bool expanded){
                card << card_header;
                web::Div collapse(collapse_id);
                std::cout << "created collapse div\n";
                collapse << card_body;
                std::cout << "put card body in collapse div\n";
                card << collapse;
                std::cout << "put collapse div in card\n";
                AddBootstrap();
                if (expanded){
                    collapse.SetAttr("class", "collapse show");
                }
                else{
                    collapse.SetAttr("class", "collapse");
                }
                }
            template <typename T>
            void AddHeaderContent(T val){
                card_header << val;
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