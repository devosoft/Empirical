#ifndef EMP_CONFIG_WEB_INTERFACE_H
#define EMP_CONFIG_WEB_INTERFACE_H

#include "../config/config.h"
#include "../web/Div.h"
#include "../web/Element.h"
#include "../web/Input.h"

#include <map>
#include <set>
#include "../tools/set_utils.h"
#include "../tools/string_utils.h"

// Prefab elements
#include "Card.h"
#include "CommentBox.h"
#include "FontAwesomeIcon.h"
#include "Collapse.h"
#include "ToggleSwitch.h"

namespace emp {
    namespace prefab{

    namespace internal {

        /// Shared pointer held by instances of ConfigPanel class representing
        /// the same conceptual ConfigPanel DOM object.
        /// Contains state that should persist while ConfigPanel DOM object
        /// persists.
        class ConfigPanelInfo : public web::internal::DivInfo {

        public:
            using on_change_fun_t = std::function<void(const std::string & val)>;

        private:
            on_change_fun_t on_change_fun{ [](const std::string & val){ ; } };

        public:

            /// Construct a shared pointer to manage ConfigPanel state.
            /// @in_id HTML ID of ConfigPanel div
            ConfigPanelInfo(
              const std::string & in_id=""
            ) : web::internal::DivInfo(in_id)
            { ; }

            /// Get current on-update callback for a ConfigPanel.
            /// @return current callback function handle
            on_change_fun_t & GetOnChangeFun() { return on_change_fun; }

            /// Set on-update callback for a ConfigPanel.
            /// @fun callback function handle
            void SetOnChangeFun(const on_change_fun_t & fun) {
              on_change_fun = fun;
            }

        };

    }

    class ConfigPanel : public web::Div {
        public:
            using on_change_fun_t = internal::ConfigPanelInfo::on_change_fun_t;

        private:
            /// Type of shared pointer shared among instances of ConfigPanel
            /// representing the same conceptual DOM element.
            using INFO_TYPE = internal::ConfigPanelInfo;

            /// Get shared info pointer, cast to ConfigPanel-specific type.
            /// @return cast pointer
            INFO_TYPE * Info() {
              return dynamic_cast<INFO_TYPE *>(info);
            }

            /// Get shared info pointer, cast to const ConfigPanel-specific type.
            /// @return cast pointer
            const INFO_TYPE * Info() const {
              return dynamic_cast<INFO_TYPE *>(info);
            }

            inline static std::set<std::string> numeric_types = {"int", "double", "float", "uint32_t", "uint64_t", "size_t"};
            Config & config;
            web::Div settings_div;
            std::set<std::string> exclude;
            std::map<std::string, web::Div> group_divs;
            std::map<std::string, web::Div> input_divs;
            std::function<std::string(std::string val)> format_label_fun = [](std::string name){
                emp::vector<std::string> sliced = slice(name, '_');
                return to_titlecase(join(sliced, " "));
            };
            void SyncForm(std::string val, std::string input1, std::string input2){
                emp::web::Input div1(settings_div.Find(input1));
                div1.Value(val);
                emp::web::Input div2(settings_div.Find(input2));
                div2.Value(val);
                div1.Redraw();
                div2.Redraw();
            }

        /// Get current on-update callback.
        /// @return current callback function handle
        on_change_fun_t& GetOnChangeFun() {
            return Info()->GetOnChangeFun();
        };

        /// Run on-update callback.
        /// @val TODO what is this?
        void DoOnChangeFun(const std::string & val) {
            Info()->GetOnChangeFun()(val);
        };

        public:

            ConfigPanel(
              Config & c,
              const std::string & div_name = "settings_div"
            ) : config(c)
            , settings_div(div_name) // TODO: construct a settings div without a name?
            { info = new internal::ConfigPanelInfo(div_name); }

            /// Sets on-update callback for a ConfigPanel.
            /// @fun callback function handle
            void SetOnChangeFun(const on_change_fun_t& fun) {
              Info()->SetOnChangeFun(fun);
            }

            template <typename T>
            void SetDefaultRangeFloatingPoint(web::Input & input, T val) {
                if (val > 0 && val < 1) {
                    // This is a common range for numbers to be in
                    input.Min(0);
                    if (val > .1) {
                        input.Max(1);
                    } else {
                        input.Max(val * 100);
                    }
                    input.Step(val/10.0);
                } else if (val > 0) {
                    // Assume this is a positive number
                    input.Min(0);
                    input.Max(val * 10);
                    input.Step(val/10.0);
                } else if (val < 0) {
                    input.Min(val * 10); // since val is negative
                    input.Max(val * -10);
                    input.Step(val/-10.0); // A negative step would be confusing
                }

                // Otherwise val is 0 and we have nothing to go on
            }

            void SetDefaultRangeFixedPoint(web::Input & input, int val) {
                // Default step is 1, which should be fine for fixed point

                if (val > 0) {
                    // Assume this is a positive number
                    input.Min(0);
                    input.Max(val * 10);
                } else if (val < 0) {
                    input.Min(val * 10); // since val is negative
                    input.Max(val * -10);
                }

                // Otherwise val is 0 and we have nothing to go on
            }

            void ExcludeConfig(std::string setting) {
                exclude.insert(setting);
            }

            void Setup(const std::string & id_prefix = "settings_") {
                for (auto group : config.GetGroupSet()) {
                    // std::cout << "GROUP: " << group->GetName() << std::endl;
                    std::string group_name = group->GetName();
                    group_divs[group_name] = web::Div(id_prefix + group_name);
                    settings_div << group_divs[group_name];

<<<<<<< HEAD
                    // Setting card setup
                    web::Div card("card_" + group_name);
                    group_divs[group_name] << card;
                    web::Div card_header("card_header_" + group_name);
                    card << card_header;

					web::Div inline_elements(group_name + "_inline");
=======
                    // Prefab Card
<<<<<<< HEAD
<<<<<<< HEAD
                    emp::Card card("card_collapse_" + group_name, true);
                    group_divs[group_name] << card.GetDiv();
                    // Header formatting 
                    web::Div inline_elements(group_name + "_inline");
>>>>>>> 0595c91... added card element
                    inline_elements.SetAttr("class", "clearfix");
                    card.AddHeaderContent(inline_elements);

                    // Header content
                    web::Element collapse_name_link("button");
                    emp::Collapse header_title_toggle(collapse_name_link, "card_collapse_" + group_name, true);
                    inline_elements << header_title_toggle.GetLinkDiv();
                    collapse_name_link.SetAttr("class", "btn btn-link float-left collapse_toggle setting_heading");
                    
                    // Prefab Collapse/toggle for card
                    collapse_name_link << "<h3>" << group->GetDesc() << "</h3>";
                    web::Element collapse_icon_link("button");
<<<<<<< HEAD
                    inline_elements << collapse_icon_link;
<<<<<<< HEAD
                    collapse_icon_link.SetAttr("data-toggle", "collapse").SetAttr("data-target", "#card_collapse_" + group_name);
                    collapse_icon_link.SetAttr("class", "btn btn-link float-right collapse_toggle").SetAttr("type", "button").SetAttr("aria-expanded", "true").SetAttr("aria-controls", "#card_body_" + group_name);

=======
                    collapse_icon_link.SetAttr(
                        "data-toggle", "collapse",
                        "data-target", "#card_collapse_" + group_name,
                        "class", "btn btn-link float-right collapse_toggle",
                        "type", "button",
                        "aria-expanded", "true",
                        "aria-controls", "#card_collapse_" + group_name // id passed to card constructor
                    );
<<<<<<< HEAD

>>>>>>> 1509465... added prefab module for comment box
                    // Toggle Icons
                    web::Element arrow_down("span");
                    collapse_icon_link << arrow_down;
                    arrow_down.SetAttr("class", "fa fa-angle-double-down");
                    web::Element arrow_up("span");
                    collapse_icon_link << arrow_up;
                    arrow_up.SetAttr("class", "fa fa-angle-double-up");
=======
=======
                    emp::Collapse header_icon_toggle(collapse_icon_link, "card_collapse_" + group_name, true);
                    inline_elements << header_icon_toggle.GetLinkDiv();
                    collapse_icon_link.SetAttr("class", "btn btn-link float-right collapse_toggle");
>>>>>>> 5f848a8... add prefab element to add collapibility to web elements
                    
                    // Prefab Icons
                    emp::FontAwesomeIcon arrow_down("fa-angle-double-down");
                    collapse_icon_link << arrow_down.GetDiv();
                    emp::FontAwesomeIcon arrow_up("fa-angle-double-up");
                    collapse_icon_link << arrow_up.GetDiv();
>>>>>>> c2cc336... prefab element for font awesome icons

<<<<<<< HEAD
                    web::Div card_collapse("card_collapse_" + group_name);
                    card << card_collapse;
                    card_collapse.SetAttr(
                        "class", "collapse show",
                        "data-parent", "card_" + group_name,
                        "aria-labelledby", "card_header_" + group_name
                    );
                    web::Div card_body("card_body_" + group_name);
                    card_collapse << card_body;
                    // make card true bootstrap cards
                    card.SetAttr("class", "card");
                    card_header.SetAttr("class", "card-header");
                    card_body.SetAttr("class", "card-body");

=======
                    prefab::Card card(prefab::Card::Collapse::OPEN);
=======
                    prefab::Card card("INIT_OPEN");
>>>>>>> 962b5db... added ConfigPanelInfo class back in
                    group_divs[group_name] << card;

                    // Header content
                    web::Div setting_heading;
                    card.AddHeaderContent(setting_heading);
                    setting_heading << "<h3>" + group->GetDesc() + "</h3>";
                    setting_heading.SetAttr("class", "setting_heading");
>>>>>>> 3305a14... change namespace to emp::prefab and inherit from web components for some prefab components

<<<<<<< HEAD
=======
                    // this is taken care of in card element
                    // TODO: decide what to do with data-parent and aria-labelledby
                    // web::Div card_collapse("card_collapse_" + group_name);
                    // card_collapse.SetAttr(
                    //     // "class", "collapse show",
                    //     "data-parent", "card_" + group_name,
                    //     "aria-labelledby", "card_header_" + group_name
                    // );
                    
>>>>>>> 0595c91... added card element

=======
>>>>>>> 5f848a8... add prefab element to add collapibility to web elements
                    for (size_t i = 0; i < group->GetSize(); i++) {
                        // std::cout << group->GetEntry(i)->GetType() << std::endl;
                        std::string name = group->GetEntry(i)->GetName();
                        if (Has(exclude, name)) {
                            continue;
                        }
                        std::string type = group->GetEntry(i)->GetType();
                        std::string value = group->GetEntry(i)->GetValue();
<<<<<<< HEAD
<<<<<<< HEAD

                        card_body << web::Element("form").SetCSS("width", "100%") << input_divs[name];
=======
                   
<<<<<<< HEAD
                        card.AddBodyContent(web::Element("form").SetCSS("width", "100%") << input_divs[name]);
>>>>>>> 0595c91... added card element

                        web::Div mobile_dropdown("mobile_dropdown_" + name);
                        mobile_dropdown.SetAttr("class", "d-md-none");
=======
=======

>>>>>>> 4e37ab7... Strip trailing whitespace
                        card.AddBodyContent(input_divs[name]);
>>>>>>> 6ccbb1f... fixed error in adding settings to card body

                        // Setting element label
                        web::Div setting_element(name + "_row");
                        input_divs[name] << setting_element;
                        setting_element.SetAttr("class", "setting_element");
                        web::Element title_span("span");
<<<<<<< HEAD
                        web::Element title("a");
                        setting_element << title_span << title;
                        title.SetAttr(
                            "data-toggle", "collapse",
                            "href", "#" + name + "_dropdown",
                            "class", "collapse_toggle",
                            "role", "button",
                            "aria-expanded", "false",
                            "aria-controls", "#" + name + "_dropdown"
                        );
=======
                        setting_element << title_span;
                        web::Element title("button");
                        title.SetAttr("class", "btn btn-link");
>>>>>>> 5f848a8... add prefab element to add collapibility to web elements

                        prefab::FontAwesomeIcon arrow_right_for_dropdown("fa-angle-double-right");
                        title << arrow_right_for_dropdown;
                        prefab::FontAwesomeIcon arrow_up_for_dropdown("fa-angle-double-up");
                        title << arrow_up_for_dropdown;
                        title << format_label_fun(name);
                        title_span.SetAttr("class", "title_area");
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD

                        // This makes the Comment box toggle when title is clicked
                        // TODO: make a class to create a toggle for two elements?
                        web::Div dropdown_target(name + "_dropdown");
                        input_divs[name] << dropdown_target;
                        dropdown_target.SetAttr("class", "collapse");

                        // Prefab Dropdown Box Version
=======
                        
                        // Prefab Dropdown Box Version 
>>>>>>> 5f848a8... add prefab element to add collapibility to web elements
                        emp::CommentBox box;
=======
                        arrow_right_for_dropdown.AddClass("toggle_icon_left_margin");
                        arrow_up_for_dropdown.AddClass("toggle_icon_left_margin");
=======
                        arrow_right_for_dropdown.AddClass("toggle_icon_right_margin");
                        arrow_up_for_dropdown.AddClass("toggle_icon_right_margin");
<<<<<<< HEAD
>>>>>>> 4f77c5a... added notes for users and removed print statements
                        
                        // Prefab Dropdown Box 
=======

                        // Prefab Dropdown Box
>>>>>>> 4e37ab7... Strip trailing whitespace
=======

                        arrow_right_for_dropdown.AddAttr("class", "toggle_icon_right_margin");
                        arrow_up_for_dropdown.AddAttr("class", "toggle_icon_right_margin");
                        
                        // Prefab Dropdown Box 

>>>>>>> a4fc814... added LoadingIcon tests and more CodeBlock tests
                        prefab::CommentBox box;
>>>>>>> 3305a14... change namespace to emp::prefab and inherit from web components for some prefab components
                        box.AddContent(group->GetEntry(i)->GetDescription());

                        // Prefab Collapse/toggle for setting element
                        prefab::CollapseCoupling title_toggle(title, box, false, name + "_dropdown");
                        input_divs[name] << title_toggle.GetTargetDiv(0);
                        title_span << title_toggle.GetControllerDiv(0);


                        if (Has(numeric_types, type)) {
<<<<<<< HEAD
                            // Seems more efficient to use web::Input, but it's not working
                            // TODO: need to define on change function to use built in Input
                            // emp::web::Input slider([this, name](std::string val){
                                                            //   std::cout << name << " " << val << " " <<config.Get(name) << std::endl;
                                                            //   config.Set(name, val);
                                                            //   std::cout << config.Get(name) << std::endl;
                                                            //   on_change_fun(val);},
                                                            // "range", format_label_fun(name), name + "_input_slider", true
                                                            // );
                            web::Element slider("input");
                            slider.SetAttr("type", "range");


                            web::Element number("input");
=======
                            // Empty div to keep elements aligned in mobile view
>>>>>>> 9738447... display appropriate setting descriptions and synchronize form when one input is changed
                            web::Element spacer("span");
                            setting_element << spacer;
                            spacer.SetAttr("class", "blank_div");

=======
>>>>>>> b7cb8b4... updated classes
                            const std::string name_input_slider = name + "_input_slider";
                            const std::string name_input_number = name + "_input_number";
                            const std::string name_input_mobile_slider = name + "_input_mobile_slider";
                            web::Input slider( [](std::string x){
                                std::cout << "empty slider function" << std::endl;},
                            "range", NULL, name_input_slider
                            );
                            setting_element << slider;

                            web::Input number([](std::string val){
                                std::cout << "empty number function" << std::endl;
                                },
                                "number", NULL, name_input_number
                                );
                            setting_element << number;
                            web::Input mobile_slider([](std::string val){
                                std::cout << "empty mobile slider function" << std::endl;
                            },
                                "range", NULL, name_input_mobile_slider
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
                                );
                            mobile_dropdown << "<hr>" << mobile_slider;
=======
                                );
                            std::cout << name << " ---- adding mobile content" << std::endl;
                            box.AddMobileContent("<hr>");
                            box.AddMobileContent(mobile_slider);
                            std::cout << "BACK to config panel after adding mobile content" << std::endl;
>>>>>>> 1509465... added prefab module for comment box
=======
                                ); 
=======
                                );
>>>>>>> 4e37ab7... Strip trailing whitespace
                            box.AddMobileContent("<hr>");
                            box.AddMobileContent(mobile_slider);
>>>>>>> 0595c91... added card element

                            // Set onchange behavior for inputs
                            slider.Callback(
                                [this,name, name_input_number, name_input_mobile_slider](std::string val){
                                config.Set(name, val);
                                SyncForm(val, name_input_number, name_input_mobile_slider);
                                });
                            number.Callback(
                                [this,name, name_input_slider, name_input_mobile_slider](std::string val){
                                config.Set(name, val);
<<<<<<< HEAD
                                std::cout << "slider callback: after setting new value" << std::endl;

=======
>>>>>>> 1509465... added prefab module for comment box
                                SyncForm(val, name_input_slider, name_input_mobile_slider);
                                });
                            mobile_slider.Callback(
                                [this,name, name_input_number, name_input_slider](std::string val){
                                config.Set(name, val);
                                SyncForm(val, name_input_number, name_input_slider);
                                });
                            // Set initial values
                            slider.Value(config.Get(name));
                            number.Value(config.Get(name));
                            mobile_slider.Value(config.Get(name));
                            slider.SetAttr("class", "input_slider");
                            number.SetAttr("class", "input_number");

                            // Attempt to have intelligent defaults
                            if (type == "double") {
<<<<<<< HEAD
                                SetDefaultRangeFloatingPoint(input_map[name], emp::from_string<double>(value));
                            } else if (type == "float") {
                                SetDefaultRangeFloatingPoint(input_map[name], emp::from_string<float>(value));
                            } else {
                                // TODO: Correctly handle all types (although I'm not sure it actually matters?)
                                SetDefaultRangeFixedPoint(input_map[name], emp::from_string<int>(value));
                            }

                        } else if (type == "bool") {
                            input_map[name] = emp::web::Input(
                                [this, name](std::string val){config.Set(name, val);
                                                              on_change_fun(val);},
                                "checkbox", format_label_fun(name), name + "_input_checkbox"
                            );
=======
                                SetDefaultRangeFloatingPoint(slider, emp::from_string<double>(value));
                                SetDefaultRangeFloatingPoint(number, emp::from_string<double>(value));
                                SetDefaultRangeFloatingPoint(mobile_slider, emp::from_string<double>(value));
                            } else if (type == "float") {
                                SetDefaultRangeFloatingPoint(slider, emp::from_string<float>(value));
                                SetDefaultRangeFloatingPoint(number, emp::from_string<float>(value));
                                SetDefaultRangeFloatingPoint(mobile_slider, emp::from_string<float>(value));
                            } else {
                                // TODO: Correctly handle all types (although I'm not sure it actually matters?)
                                SetDefaultRangeFixedPoint(slider, emp::from_string<int>(value));
                                SetDefaultRangeFixedPoint(number, emp::from_string<int>(value));
                                SetDefaultRangeFixedPoint(mobile_slider, emp::from_string<int>(value));
                            }

                        }
                        else if (type == "bool") {
                            // Bootstrap Toggle Switch (need at least v4.5.0)
                            emp::prefab::ToggleSwitch toggle_switch(
                                [this, name](std::string val){
                                  config.Set(name, val);
                                  DoOnChangeFun(val);
                                },
                                NULL,
                                emp::from_string<bool>(value),
                                name + "_input_checkbox"
                            );
                            setting_element << toggle_switch;
                            toggle_switch.AddAttr("class", "input_bool");

>>>>>>> 9738447... display appropriate setting descriptions and synchronize form when one input is changed
                        } else {
                            web::Input text_input(
                                [this, name](std::string val){
                                    config.Set(name, val);
                                    DoOnChangeFun(val);
                                },
                                "text", NULL, name + "_input_textbox"
                            );
                            setting_element << text_input;
                            text_input.SetAttr(
                                "class", "input_text",
                                "type", "text"
                            );
                            text_input.Value(config.Get(name));
                        }
<<<<<<< HEAD

                        // Dropdown bubble
                        // Should probably be modularlized in the future
                        web::Div drop_down(name + "_dropdown");
                        web::Div triangle(name + "_triangle_up");
                        web::Div content(name + "_content");
                        triangle.SetCSS("width", "0").SetCSS("height", "0").SetCSS("border-left", "12px solid transparent").SetCSS("border-right", "12px solid transparent").SetCSS("border-bottom", "12px solid #ede9e8").SetCSS("margin-left", "15px");
                        drop_down << triangle;
                        content.SetAttr("class", "p-3").SetCSS("font-style", "italic");
                        content << "hello world!";
                        content << mobile_dropdown;
                        drop_down << triangle;
                        drop_down << content;
                        content.SetBackground("#ede9e8");
                        drop_down.SetAttr("class", "collapse");

                        // input_map[name].Value(value);
                        input_divs[name] << setting_element;
                        input_divs[name] << drop_down;

                        // group_divs[group_name] << web::Element("form") << input_map[name];
                        card_body << web::Element("form").SetCSS("width", "100%") << input_divs[name];
>>>>>>> c0edc65... added css styling to form and dropdown box
                        group_divs[group_name] << card << card_body;

                    }
                    // settings_div.SetAttr("class", "col-lg-6 col-md-12"); makes all CSS go away!!!!
                    settings_div << group_divs[group_name];
=======
                    }
>>>>>>> 9738447... display appropriate setting descriptions and synchronize form when one input is changed
                }

            }

            web::Div & GetDiv() {return settings_div;}

    };
    }
}

#endif
