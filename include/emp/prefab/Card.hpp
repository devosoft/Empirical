/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Card.hpp
 *  @brief Wraps a Bootstrap card.
 */

#ifndef EMP_PREFAB_CARD_HPP_INCLUDE
#define EMP_PREFAB_CARD_HPP_INCLUDE

#include "../tools/string_utils.hpp"
#include "../web/Div.hpp"

#include "Collapse.hpp"
#include "FontAwesomeIcon.hpp"

namespace emp {
namespace prefab {

  namespace internal {
    /**
     * Shared pointer held by instances of Card class representing
     * the same conceptual Card DOM object.
     * Contains state that should persist while Card DOM object
     * persists.
     */
    class CardInfo : public web::internal::DivInfo {

    public:
      using on_toggle_fun_t = std::function<void()>;
    protected:
      on_toggle_fun_t toggle_handler;
    public:
      /**
       * Construct a shared pointer to manage Card state.
       * @param in_id HTML ID of Card div
       */
      CardInfo(const std::string & in_id="")
      : DivInfo(in_id), toggle_handler([]() {;}) { ; }

      /**
       * Get the on-toggle function for this component.
       * @return the function called whenever the card's state
       * is toggled open or closed
       */
      on_toggle_fun_t & GetOnToggle() {
        return toggle_handler;
      }

      /**
       * Set the on-toggle function for this component.
       * @param on_toggle the function to be called whenever the
       * card's state is toggled open or closed.
       */
      void SetOnToggle(on_toggle_fun_t on_toggle) {
        toggle_handler = on_toggle;
      }
    };
  }

  /// Use Card class to create Bootstrap style cards.
  class Card : public web::Div {

    using on_toggle_fun_t = internal::CardInfo::on_toggle_fun_t;

  private:
    /**
     * Get shared info pointer, cast to Card-specific type.
     * @return cast pointer
     */
    internal::CardInfo * Info() {
      return dynamic_cast<internal::CardInfo *>(info);
    }

    /**
     * Get shared info pointer, cast to const Card-specific type.
     * @return cast pointer
     */
    const internal::CardInfo * Info() const {
      return dynamic_cast<internal::CardInfo *>(info);
    }

  protected:
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
     * @param state indicate whether card should be STATIC, INIT_OPEN, or INIT_CLOSED (default STATIC)
     * @param show_glyphs should toggle icons show in collapsible card header? (default true)
     * @param id user defined ID for card Div, (default emscripten generated)
     */
    Card(
      const std::string & state="STATIC",
      const bool & show_glyphs=true,
      const std::string & id=""
    ) : Card(state, show_glyphs, new internal::CardInfo(id)) { ; }

  protected:
    /**
     * A protected constructor for a Card.
     * @param state indicate whether card should be STATIC, INIT_OPEN, or INIT_CLOSED (default STATIC)
     * @param show_glyphs should toggle icons show in collapsible card header? (default true)
     * @param info_ref a pointer to the underlying ReadoutPanelInfo object for this ReadoutPanel
     * or a pointer to a derived info object (simulating inheritance)
     */
    Card(
      const std::string & state,
      const bool & show_glyphs,
      internal::CardInfo * in_info
    ) : Div(in_info) {

      AddBootstrap();
      if (state == "STATIC") { // static card with no toggle enabled
        static_cast<Div>(*this) << card_header; // Cast to Div prevents redefined
        static_cast<Div>(*this) << card_body;   // stream operator from being used
      } else {
        // card is collapsible, make the collapse link the head of the card
        prefab::CollapseCoupling accordion(card_header,
          card_body,
          state == "INIT_OPEN",
          emp::to_string(card_base+ "_card_collapse")
        );

        Div header_div{accordion.GetControllerDiv()};
        static_cast<Div>(*this) << header_div;
        static_cast<Div>(*this) << accordion.GetTargetDiv();

        on_toggle_fun_t & tog = GetOnToggle();
        header_div.OnClick([header_div, &toggle = tog](){
          toggle();
          // TODO: toggle really should accept boolean so the user can trigger
          // different events on open/close to prevent double click issues
          // but getting that state is currently impossible, see
          // https://github.com/devosoft/Empirical/issues/440.
        });

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
  public:
    /**
     * Add content to header section of card.
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
     * Add content to body section of card.
     *
     * @param val can be a web element or primitive type
     * @deprecated Use stream operator instead
     */
    template <typename T>
    [[deprecated("Use the stream operator (<<) to add to card body")]]
    void AddBodyContent(T val) {
      card_body << val;
    }

    /**
     * Add content to the body section of the card.
     *
     * @param in_val can be a web element or primitive type
     */
    template <typename IN_TYPE>
    emp::prefab::Card & operator<<(IN_TYPE && in_val) {
      card_body << std::forward<IN_TYPE>(in_val);
      return (*this);
    }

    /**
     * Sets the on-toggle function for this component.
     * @param on_toggle the function to be called whenever the card's state is
     * toggled open or closed
     */
    void SetOnToggle(on_toggle_fun_t on_toggle) {
      Info()->SetOnToggle(on_toggle);
    }

    /**
     * Get the on-toggle function for this component.
     * @return the function to be called whenever the card's state is
     * toggled open or closed
     */
    on_toggle_fun_t & GetOnToggle() {
      return Info()->GetOnToggle();
    }

  };
}
}

#endif // #ifndef EMP_PREFAB_CARD_HPP_INCLUDE
