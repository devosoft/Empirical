#ifndef EMP_CONFIG_PANEL_BUG_H
#define EMP_CONFIG_PANEL_BUG_H

#include "../config/config.h"
#include "../web/Div.h"
#include "../web/Element.h"
#include "../web/Input.h"

#include <map>
#include <set>
#include "../tools/set_utils.h"
#include "../tools/string_utils.h"

// Using prefab elements
#include "CommentBox.h"
#include "Card.h"
#include "Collapse.h"
#include "FontAwesomeIcon.h"

/**
* This config panel will not render on the web.  When setting the initial value for the first slider,
* it throws this exception: uncaught exception: abort(2). Build with -s ASSERTIONS=1 for more info.
* Emscripten notes on exception: https://emscripten.org/docs/porting/Debugging.html#debugging-assertions
*
* Methods called when setting value: Value -> UpdateValue -> DoChange -> callback -> SynchForm -> Value
*
* Note: This is not an issue when the prefab comment box element is added to the map of input_divs immediately
* after creation. (uncomment line 109 and comment line 188)
* All description boxes will be expanded.
*/

namespace emp {

class CP_Bug {
    private:
      inline static std::set<std::string> numeric_types = {"int", "double", "float", "uint32_t", "uint64_t", "size_t"};
      Config & config;
      web::Div settings_div;
      std::set<std::string> exclude;
      std::map<std::string, web::Div> group_divs;
      std::map<std::string, web::Div> input_divs;
      std::function<void(const std::string & val)> on_change_fun = [](const std::string & val){;};
      std::function<std::string(std::string val)> format_label_fun = [](std::string name){
          emp::vector<std::string> sliced = slice(name, '_');
          return to_titlecase(join(sliced, " "));
      };
      void SyncForm(std::string val, std::string input1, std::string input2){
        std::cout << "---- SyncForm() -----\n";
        emp::web::Input div1(settings_div.Find(input1));
        div1.Value(val);
        emp::web::Input div2(settings_div.Find(input2));
        div2.Value(val);
        div1.Redraw();
        div2.Redraw();
        std::cout << "END SyncForm()\n";
      }
    public:
      CP_Bug(Config & c, const std::string & div_name = "settings_div")
        : config(c), settings_div(div_name) {;}

      void SetOnChangeFun(std::function<void(const std::string & val)> fun) {on_change_fun = fun;}

      void ExcludeConfig(std::string setting) {
        exclude.insert(setting);
      }

      void Setup(const std::string & id_prefix = "settings_") {
        for (auto group : config.GetGroupSet()) {
          // std::cout << "GROUP: " << group->GetName() << std::endl;
          std::string group_name = group->GetName();
          group_divs[group_name] = web::Div(id_prefix + group_name);
          settings_div << group_divs[group_name];

          // Prefab Card
          prefab::Card card(prefab::Card::Collapse::OPEN);
          group_divs[group_name] << card;

          // Card header content
          web::Div setting_heading;
          card.AddHeaderContent(setting_heading);
          setting_heading << "<h3>" + group->GetDesc() + "</h3>";
          setting_heading.SetAttr("class", "setting_heading");

          for (size_t i = 0; i < group->GetSize(); i++) {
            // std::cout << group->GetEntry(i)->GetType() << std::endl;
            std::string name = group->GetEntry(i)->GetName();
            if (Has(exclude, name)) {
              continue;
            }
            std::string type = group->GetEntry(i)->GetType();
            std::string value = group->GetEntry(i)->GetValue();

            card.AddBodyContent(input_divs[name]);

            // Setting element label
            web::Div setting_element(name + "_row");
            input_divs[name] << setting_element;
            setting_element.SetAttr("class", "setting_element");
            web::Element title_span("span");
            setting_element << title_span;
            web::Element title("button");
            title_span << title;
            title.SetAttr("class", "btn btn-link");
            title << format_label_fun(name);
            title_span.SetAttr("class", "title_area");

            // Prefab Dropdown Box
            prefab::CommentBox box;
            // input_divs[name] << box;  // TODO: uncomment this line
            box.AddContent(group->GetEntry(i)->GetDescription());

            if (Has(numeric_types, type)) {
              // Empty div to keep elements aligned in mobile view
              web::Element spacer("span");
              setting_element << spacer;
              spacer.SetAttr("class", "blank_div");

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
              std::cout << name << " ---- adding mobile content" << std::endl;
              box.AddMobileContent("<hr>");
              box.AddMobileContent(mobile_slider);
              std::cout << "BACK to config panel after adding mobile content" << std::endl;

              // Set onchange behavior for inputs
              slider.Callback(
                [this,name, name_input_number, name_input_mobile_slider](std::string val){
                config.Set(name, val);
                SyncForm(val, name_input_number, name_input_mobile_slider);
                });
              number.Callback(
                [this,name, name_input_slider, name_input_mobile_slider](std::string val){
                config.Set(name, val);
                SyncForm(val, name_input_slider, name_input_mobile_slider);
                });
              mobile_slider.Callback(
                [this,name, name_input_number, name_input_slider](std::string val){
                config.Set(name, val);
                SyncForm(val, name_input_number, name_input_slider);
                });
              // Set initial values
              slider.Value(config.Get(name)); // TODO: It never gets past this line!
              number.Value(config.Get(name));
              mobile_slider.Value(config.Get(name));
              slider.SetAttr("class", "input_slider");
              number.SetAttr("class", "input_number");

            }
            else if (type == "bool") {
              emp::web::Input bool_input(
                [this, name](std::string val){config.Set(name, val);
                                                on_change_fun(val);},
                "checkbox", NULL, name + "_input_checkbox"
              );
              setting_element << bool_input;

            } else {
              emp::web::Input text_input(
                [this, name](std::string val){config.Set(name, val);
                                                on_change_fun(val);},
                "text", NULL, name + "_input_textbox"
              );
              setting_element << text_input;
              text_input.SetAttr(
                "class", "input_text",
                "type", "text"
              );
              text_input.Value(config.Get(name));
            }
            input_divs[name] << box;    // TODO: Comment this line
          }
        }

      }

        web::Div & GetDiv() {return settings_div;}

};

}

#endif
