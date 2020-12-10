#ifndef EMP_CARD_HPP
#define EMP_CARD_HPP

#include "../tools/string_utils.hpp"
#include "../web/Div.hpp"

#include "Collapse.hpp"
#include "FontAwesomeIcon.hpp"

namespace emp {
namespace prefab {
  /// Use Card class to create Bootstrap style cards.
  class Card : public web::Div {
  private:
    // ID of card Div to be used in ID of associated card sub components
    std::string card_base = this->GetID();
    // all header content will be added here
    web::Div card_header{emp::to_string(card_base, "_card_header")};
    // all body content will be added here
    web::Div card_body{emp::to_string(card_base, "_card_body")};
    // Asssigns classes to card elements for styling
    void AddBootstrap() {
      this->SetAttr("class", "card");
      card_header.SetAttr("class", "card-header");
      card_body.SetAttr("class", "card-body");
    }

  public:
    /**
     * @param state indicate whether card should be STAITC, INIT_OPEN, or INIT_CLOSED (default STAITC)
     * @param show_glyphs should toggle icons show in collapsible card header? (default true)
     * @param id user defined ID for card Div, (default emscripten generated)
     */
    Card(
      const std::string & state="STATIC",
      const bool & show_glyphs=true,
      const std::string & id=""
    ): web::Div(id) {

      AddBootstrap();
      if (state == "STATIC") { // static card with no toggle enabled
        *this << card_header;
        *this << card_body;
      } else {
        // card is collapsible, make the collapse link the head of the card
        prefab::CollapseCoupling accordion(card_header,
          card_body,
          state == "INIT_OPEN",
          emp::to_string(card_base+ "_card_collapse")
        );
        *this << accordion.GetControllerDiv();
        *this << accordion.GetTargetDiv();

        if (show_glyphs) { // by default add glyphs to a collapsible card
          prefab::FontAwesomeIcon up("fa-angle-double-up");
          prefab::FontAwesomeIcon down("fa-angle-double-down");
          card_header << up;
          card_header << down;
          up.AddAttr("class", "toggle_glyph");
          down.AddAttr("class", "toggle_glyph");
        }
        card_header.AddAttr("class", "collapse_toggle_card_header");
      }
    }

    /**
     * Add content to header section of card
     *
     * @param val content to be added to header, can be a web element or primitive type
     * @param link_content indicates whether the content should have Bootstrap link properties (default false)
     */
    template <typename T>
    void AddHeaderContent(T val, const bool link_content=false) {
      /*
       * Note: val can be a controller of a target area (made with CollapseCoupling class)
       * but when added to header of the card, it will also trigger the card
       * to collapse/expand
       */
      if (link_content) {
        /*
         * Add bootstrap link properities to content (hover, underline, etc.),
         * but does not set a target or href because it is assumed that
         * this content will control the card collapse, which is done in the
         * constructor.
         */
        web::Div btn_link;
        btn_link.SetAttr("class", "btn-link");
        card_header << btn_link << val;
      } else {
        card_header << val;
      }
    }

    /**
     * Add content to body section of card
     *
     * @param val can be a web element or primitive type
     */
    template <typename T>
    void AddBodyContent(T val) {
      card_body << val;
    }

    /*
     * TODO: override operator<< so that it streams into body of card
     * Methods tried so far (without success) are listed in
     * Issue #345 (https://github.com/devosoft/Empirical/issues/345)
     */
  };
}
}

#endif
