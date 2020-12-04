#ifndef EMP_MODAL_HPP
#define EMP_MODAL_HPP

#include "../tools/string_utils.hpp"
#include "../web/Element.hpp"
#include "../web/Div.hpp"
#include "../web/Button.hpp"
#include "../web/Widget.hpp"

namespace emp {
namespace prefab {
  /**
   * Use Modal class to add a Bootstrap style pop-up
   * modal to your web app.
   */
  class Modal: public web::Div {
    private:
      web::Div modal_dialog; // necessary for Bootstrap structure
      web::Div modal_content; // necessary for Bootstrap structure
      web::Div modal_header; // header content goes here
      web::Div modal_body; // body content goes here
      web::Div modal_footer; // footer content goes here

    public:
      Modal(const std::string & id=""): web::Div(id) {
        // nest Divs propery
        *this << modal_dialog;
        modal_dialog << modal_content;
        modal_content << modal_header;
        modal_content << modal_body;
        modal_content << modal_footer;
        // add styling
        this->SetAttr(
          "class", "modal",
          "id", this->GetID(),
          "data-backdrop", "static",
          "data-keyboard", "false",
          "tabindex", "-1"
        );
        modal_dialog.SetAttr("class", "modal-dialog");
        modal_content.SetAttr("class", "modal-content");
        modal_header.SetAttr("class", "modal-header");
        modal_body.SetAttr("class", "modal-body");
        modal_footer.SetAttr("class", "modal-footer");
      }

      /**
       * Add contnet to the header section of the modal
       * Content can be of any type
       */
      template <typename T>
      void AddHeaderContent(T val) {
        modal_header << val;
      }

      /**
       * Add content to the body section of the modal
       * Content can be of any type
       */
      template <typename T>
      void AddBodyContent(T val) {
        modal_body << val;
      }

      /**
       * Add content to the footer section of the modal
       * Content can be of any type
       */
      template <typename T>
      void AddFooterContent(T val) {
          modal_footer << val;
      }

      /**
       * Adds an X button in the upper right corner of the modal so user can close it
       * Should be called after all desired header content is added
       * Best practice is to call this method, unless a close button is added somewhere
       * else in the modal using AddButton()
       */
      void AddClosingX() {
        web::Button close_btn([](){;}, "x");
        modal_header << close_btn;
        close_btn.SetAttr(
          "class", "close float-right",
          "data-dismiss", "modal",
          "aria-label", "Close"
        );
      }

      /**
       * Add properties to a button element to make it open and close
       * the modal. The button can then be added to the modal to close it
       * or anywhere else in the web app to trigger the modal to open
       */
      void AddButton(web::Button & btn) {
        btn.SetAttr(
          "data-toggle", "modal",
          "data-target", "#" + this->GetID()
        );
      }

      /**
       * Overrides SetBackground() in WidgetFacet so that the color
       * will make the background of the modal change as expected.
       */
      void SetBackground(const std::string color) {
        modal_content.SetBackground(color);
      }
  };
}
}

#endif
