#ifndef EMP_CONFIG_WEB_INTERFACE_H
#define EMP_CONFIG_WEB_INTERFACE_H

#include "../config/config.h"
#include "../web/Div.h"
#include "../web/Element.h"
#include "../web/Input.h"
#include <emscripten.h>   // to use EM_JS
#include "../web/JSWrap.h" // to use EM_ASM

#include <map>
#include <set>
#include "../tools/set_utils.h"
#include "../tools/string_utils.h"

namespace emp {

    class ConfigPanel {
        private:
            inline static std::set<std::string> numeric_types = {"int", "double", "float", "uint32_t", "uint64_t", "size_t"};
            Config & config;
            web::Div settings_div;
            std::set<std::string> exclude;
            std::map<std::string, web::Div> group_divs;
            std::map<std::string, web::Input> input_map;
            std::function<void(const std::string & val)> on_change_fun = [](const std::string & val){;};
            std::function<std::string(std::string val)> format_label_fun = [](std::string name){
                emp::vector<std::string> sliced = slice(name, '_');
                return to_titlecase(join(sliced, " "));
            };
            void SyncForm(std::string val, std::string input1, std::string input2){
                std::cout << "Begin SyncForm" << std::endl;
                emp::web::Input div1(settings_div.Find(input1));
                div1.Value(val);
                emp::web::Input div2(settings_div.Find(input2));
                div2.Value(val);
                std::cout << "SyncForm: after updating other two divs" << std::endl;

                div1.Redraw();
                div2.Redraw();
                std::cout << "SyncForm: after redraw" << std::endl;
            }
        public:
            ConfigPanel(Config & c, const std::string & div_name = "settings_div")
                : config(c), settings_div(div_name) {;}

            void SetOnChangeFun(std::function<void(const std::string & val)> fun) {on_change_fun = fun;}

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

                    // Setting card setup
                    web::Div card("card_" + group_name);
                    group_divs[group_name] << card;
                    web::Div card_header("card_header_" + group_name);
                    card << card_header;
                    web::Element collapse_link("a");
                    card_header << collapse_link;
                    collapse_link.SetAttr("data-toggle", "collapse").SetAttr("href", "#card_body_" + group_name);
                    collapse_link << "<h3>" << group->GetDesc() << "</h3>";


                    web::Div card_body("card_body_" + group_name);
                    card << card_body;
                    card_body.SetAttr("class", "collapse").SetAttr("class", "show");
                    // make card true bootstrap cards
                    card.SetAttr("class", "card");
                    card_header.SetAttr("class", "card-header");
                    card_body.SetAttr("class", "card-body");



                    for (size_t i = 0; i < group->GetSize(); i++) {
                        // std::cout << group->GetEntry(i)->GetType() << std::endl;
                        std::string name = group->GetEntry(i)->GetName();
                        if (Has(exclude, name)) {
                            continue;
                        }
                        std::string type = group->GetEntry(i)->GetType();
                        std::string value = group->GetEntry(i)->GetValue();

                        card_body << web::Element("form").SetCSS("width", "100%") << input_divs[name];

<<<<<<< HEAD
<<<<<<< HEAD
                        if (Has(numeric_types, type)) {
                            input_map[name] = emp::web::Input(
                                [this, name](std::string val){
                                                              std::cout << name << " " << val << " " <<config.Get(name) << std::endl;
                                                              config.Set(name, val);
                                                              std::cout << config.Get(name) << std::endl;
                                                              on_change_fun(val);},
                                "range", format_label_fun(name), name + "_input_slider", true
                            );
=======
                        web::Div mobile_dropdown("mobile_dropdown_" + name);
                        mobile_dropdown.SetAttr("class", "d-md-none");

=======
                        // Setting element label
>>>>>>> 9738447... display appropriate setting descriptions and synchronize form when one input is changed
                        web::Div setting_element(name + "_row");
                        input_divs[name] << setting_element;
                        setting_element.SetCSS("width", "100%");
                        web::Element title_span("span");
                        web::Element title("a");
                        setting_element << title_span << title;
                        title.SetAttr("data-toggle", "collapse").SetAttr("href", "#" + name + "_dropdown");
                        title << format_label_fun(name);
                        title_span.SetCSS("width", "38%").SetCSS("display", "inline-block");

                        // Dropdown bubble
                        // Should probably be modularlized in the future
                        web::Div drop_down(name + "_dropdown");
                        input_divs[name] << drop_down;
                        web::Div triangle(name + "_triangle_up");
                        drop_down << triangle;
                        web::Div content(name + "_content");
                        drop_down << content;
                        triangle.SetCSS("width", "0").SetCSS("height", "0").SetCSS("border-left", "12px solid transparent").SetCSS("border-right", "12px solid transparent").SetCSS("border-bottom", "12px solid #ede9e8").SetCSS("margin-left", "15px");
                        content.SetAttr("class", "p-3").SetCSS("font-style", "italic");
                        content << group->GetEntry(i)->GetDescription();
                        content.SetBackground("#ede9e8");
                        drop_down.SetAttr("class", "collapse");
                        drop_down.SetCSS("width", "auto").SetCSS("margin-bottom", "10px");
                        // Portion of dropdown only visible on small screens
                        web::Div mobile_dropdown("mobile_dropdown_" + name);
                        content << mobile_dropdown;
                        mobile_dropdown.SetAttr("class", "d-md-none");

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
                            spacer.SetCSS("width", "38%").SetAttr("class", "d-inline-block d-md-none");

                            const std::string name_input_slider = name + "_input_slider";
                            const std::string name_input_number = name + "_input_number";
                            const std::string name_input_mobile_slider = name + "_input_mobile_slider";
                            emp::web::Input slider( [](std::string x){
                                std::cout << "empty slider function" << std::endl;},
                            "range", NULL, name_input_slider
                            );
                            setting_element << slider;

                            emp::web::Input number([](std::string val){
                                std::cout << "empty number function" << std::endl;
                                },
                                "number", NULL, name_input_number
                                );
                            setting_element << number;
                            emp::web::Input mobile_slider([](std::string val){
                                std::cout << "empty mobile slider function" << std::endl;
                            },
                                "range", NULL, name_input_mobile_slider
                                );
                            mobile_dropdown << "<hr>" << mobile_slider;

                            // Set onchange behavior for inputs
                            slider.Callback(
                                [this,name, name_input_number, name_input_mobile_slider](std::string val){
                                config.Set(name, val);
                                std::cout << "slider callback: after setting new value" << std::endl;

                                SyncForm(val, name_input_number, name_input_mobile_slider);
                                });
                            number.Callback(
                                [this,name, name_input_slider, name_input_mobile_slider](std::string val){
                                config.Set(name, val);
                                std::cout << "slider callback: after setting new value" << std::endl;

                                SyncForm(val, name_input_slider, name_input_mobile_slider);
                                });
                            mobile_slider.Callback(
                                [this,name, name_input_number, name_input_slider](std::string val){
                                config.Set(name, val);
                                std::cout << "slider callback: after setting new value" << std::endl;

                                SyncForm(val, name_input_number, name_input_slider);
                                });
                            // Set initial values
                            slider.Value(config.Get(name));
                            number.Value(config.Get(name));
                            mobile_slider.Value(config.Get(name));
                            slider.SetCSS("width", "38%").SetCSS("margin-right", "10px").SetAttr("class", "d-none d-md-inline-block");
                            number.SetCSS("width", "18%").SetCSS("right", "10px");

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
                            emp::web::Input bool_input(
                                [this, name](std::string val){config.Set(name, val);
                                                              on_change_fun(val);},
                                "checkbox", NULL, name + "_input_checkbox"
                            );
                            setting_element << bool_input;

>>>>>>> 9738447... display appropriate setting descriptions and synchronize form when one input is changed
                        } else {
                            emp::web::Input text_input(
                                [this, name](std::string val){config.Set(name, val);
                                                               on_change_fun(val);},
                                "text", NULL, name + "_input_textbox"
                            );
                            setting_element << text_input;
                            text_input.Value("none");
                            text_input.SetAttr("type", "text");
                            text_input.SetCSS("width", "39%");
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

#endif
