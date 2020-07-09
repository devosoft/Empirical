#ifndef EMP_MODAL_H
#define EMP_MODAL_H

#include "../web/Element.h"
#include "../web/Div.h"
#include "../web/Button.h"
#include "../web/Widget.h"
#include "../tools/string_utils.h"

namespace emp {
    namespace prefab {
        class Modal: public web::Div {
            std::string modal_html = R"(<div class="modal bd-example-modal-lg" id="my_modal" data-backdrop="static" data-keyboard="false" tabindex="-1">
                    <div class="modal-dialog modal-sm">
                        <div class="modal-content" style="width: 256px">
                        <div class="modal-header">
                            <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                                <span aria-hidden="true">&times;</span>
                            </button>
                        </div>
                        <div class="modal-body">
                            <div class="spinner-border text-light" style="width: 256px; height: 256px;" role="status">
                            <span class="sr-only">Loading...</span>
                        </div>
                        </div>
                        </div>
                        </div>
                    </div>
                </div>)";
                web::Div modal_dialog;
                web::Div modal_content;
                web::Div modal_header;
                web::Div modal_body;
                web::Div modal_footer;
            private:
            public:
                Modal(const std::string & in_name=""): web::Div(in_name){
                    *this << modal_dialog;
                    modal_dialog << modal_content;
                    modal_content << modal_header;
                    modal_content << modal_body;
                    modal_content << modal_footer;
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
                template <typename T>
                void AddHeaderContent(T val){
                    modal_header << val;
                }
                template <typename T>
                void AddBodyContent(T val){
                    modal_body << val;
                }
                template <typename T>
                void AddFooterContent(T val){
                    modal_footer << val;
                }
                // Adds an X button in the upper right corner of the modal so user can close it
                // Should be called after all desired header content is added
                // Best practice is to call this method, unless a close button is added somewhere
                // else in the modal using AddButton()
                void AddClosingX(){
                    web::Button close_btn([](){;}, "x");
                    modal_header << close_btn;
                    close_btn.SetAttr(
                        "class", "close float-right",
                        "data-dismiss", "modal",
                        "aria-label", "Close"
                    );
                }
                void AddButton(web::Button & btn){
                    btn.SetAttr(
                        "data-toggle", "modal",
                        "data-target", "#" + this->GetID()
                    );
                }
                void SetBackground(const std::string color){
                    modal_content.SetBackground(color);
                }
        };
    }
}

#endif