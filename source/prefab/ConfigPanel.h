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

namespace emp {

    class ConfigPanel {
        private:
            inline static std::set<std::string> numeric_types = {"int", "double", "float", "uint32_t", "uint64_t", "size_t"};
            Config & config;
            web::Div settings_div;
            std::set<std::string> exclude;
            std::map<std::string, web::Div> group_divs;
            // std::map<std::string, web::Input> input_map;
            std::map<std::string, web::Div> input_divs;
            std::function<void(const std::string & val)> on_change_fun = [](const std::string & val){;};
            std::function<std::string(std::string val)> format_label_fun = [](std::string name){
                emp::vector<std::string> sliced = slice(name, '_');
                return to_titlecase(join(sliced, " "));
            };
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
                    // group_divs[group_name] << "<h3 style='text-align: center'>" << group->GetDesc() << "</h3>"; 

                    web::Div card("card_" + group_name);
                    web::Div card_header("card_header_" + group_name);
                    web::Element collapse_link("a");
                    collapse_link.SetAttr("data-toggle", "collapse").SetAttr("href", "#card_body_" + group_name);
                    collapse_link << "<h3>" << group->GetDesc() << "</h3>";
                    card_header << collapse_link;
                    card << card_header;

                    
                    web::Div card_body("card_body_" + group_name);
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

                        web::Div setting_element(name + "_row");
                        web::Element title("a");
                        title.SetAttr("data-toggle", "collapse").SetAttr("href", "#" + name + "_dropdown");
                        title << format_label_fun(name);
                        setting_element << title;

                        if (Has(numeric_types, type)) {
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
                            number.SetAttr("type", "number");
                            setting_element << slider;
                            setting_element << number;
                            // NOTE: adding an id to these slider or number breaks it!

                            // Attempt to have intelligent defaults
                            // TODO: uncomment this area after onchange function is defined
                            // if (type == "double") {
                                // SetDefaultRangeFloatingPoint(input_map[name], emp::from_string<double>(value));
                            // } else if (type == "float") {
                                // SetDefaultRangeFloatingPoint(input_map[name], emp::from_string<float>(value));
                            // } else {
                                // TODO: Correctly handle all types (although I'm not sure it actually matters?)
                                // SetDefaultRangeFixedPoint(input_map[name], emp::from_string<int>(value));
                            // }

                        } 
                        else if (type == "bool") {
                        //     input_map[name] = emp::web::Input(
                        //         [this, name](std::string val){config.Set(name, val);
                        //                                       on_change_fun(val);},
                        //         "checkbox", format_label_fun(name), name + "_input_checkbox"
                        //     );
                            web::Element bool_input("input");
                            bool_input.SetAttr("type", "checkbox");
                            setting_element << bool_input;

                        } else {
                        //     input_map[name] = emp::web::Input(
                        //         [this, name](std::string val){config.Set(name, val);
                        //                                        on_change_fun(val);},
                        //         "text", format_label_fun(name), name + "_input_textbox"
                        //     );
                            web::Element text_input("input");
                            text_input.SetAttr("type", "text");
                            setting_element << text_input;
                        }

                        // Dropdown bubble
                        // Should probably be modularlized in the future
                        web::Div drop_down(name + "_dropdown");
                        web::Div triangle(name + "_triangle_up");
                        web::Div content(name + "_content");
                        triangle.SetCSS("width", "0").SetCSS("height", "0").SetCSS("border-left", "12px solid transparent").SetCSS("border-right", "12px solid transparent").SetCSS("border-bottom", "12px solid #ede9e8").SetCSS("margin-left", "15px");
                        drop_down << triangle;
                        content << "hello world!";
                        drop_down << triangle;
                        drop_down << content;
                        content.SetBackground("#ede9e8");
                        drop_down.SetAttr("class", "collapse");

                        // input_map[name].Value(value); 
                        input_divs[name] << setting_element;
                        input_divs[name] << drop_down;

                        // group_divs[group_name] << web::Element("form") << input_map[name];
                        card_body << web::Element("form") << input_divs[name];
                        group_divs[group_name] << card << card_body;

                    }
                    settings_div << group_divs[group_name];
                }

            }

            web::Div & GetDiv() {return settings_div;}

    };

}

#endif
