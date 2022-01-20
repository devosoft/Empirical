/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file config_web_interface.hpp
 *  @brief TODO.
 */

#ifndef EMP_CONFIG_CONFIG_WEB_INTERFACE_HPP_INCLUDE
#define EMP_CONFIG_CONFIG_WEB_INTERFACE_HPP_INCLUDE

#include <map>
#include <set>

#include "../datastructs/set_utils.hpp"
#include "../tools/string_utils.hpp"
#include "../web/Div.hpp"
#include "../web/Element.hpp"
#include "../web/Input.hpp"

#include "config.hpp"

namespace emp {

  class ConfigWebUI {
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
    public:
      ConfigWebUI(Config & c, const std::string & div_name = "settings_div")
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

        for (auto group : config.group_set) {
          // std::cout << "GROUP: " << group->GetName() << std::endl;
          std::string group_name = group->GetName();
          group_divs[group_name] = web::Div(id_prefix + group_name);
          group_divs[group_name] << "<h3 style='text-align: center'>" << group->GetDesc() << "</h3>";
          for (size_t i = 0; i < group->GetSize(); i++) {
            // std::cout << group->GetEntry(i)->GetType() << std::endl;
            std::string name = group->GetEntry(i)->GetName();
            if (Has(exclude, name)) {
              continue;
            }
            std::string type = group->GetEntry(i)->GetType();
            std::string value = group->GetEntry(i)->GetValue();

            if (Has(numeric_types, type)) {
              input_map[name] = emp::web::Input(
                [this, name](std::string val){
                                std::cout << name << " " << val << " " <<config.Get(name) << std::endl;
                                config.Set(name, val);
                                std::cout << config.Get(name) << std::endl;
                                on_change_fun(val);},
                "range", format_label_fun(name), name + "_input_slider", true
              );

              // Attempt to have intelligent defaults
              if (type == "double") {
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
            } else {
              input_map[name] = emp::web::Input(
                [this, name](std::string val){config.Set(name, val);
                                 on_change_fun(val);},
                "text", format_label_fun(name), name + "_input_textbox"
              );

            }

            input_map[name].Value(value);

            group_divs[group_name] << web::Element("form") << input_map[name];

          }
          settings_div << group_divs[group_name];
        }

      }

      web::Div & GetDiv() {return settings_div;}

  };

}

#endif // #ifndef EMP_CONFIG_CONFIG_WEB_INTERFACE_HPP_INCLUDE
