#ifndef EMP_CONFIG_WEB_INTERFACE_HPP
#define EMP_CONFIG_WEB_INTERFACE_HPP

#include <map>
#include <set>

#include "../datastructs/set_utils.hpp"
#include "../tools/string_utils.hpp"

#include "../config/config.hpp"
#include "../web/Div.hpp"
#include "../web/Element.hpp"
#include "../web/Input.hpp"

// Prefab elements
#include "Card.hpp"
#include "CommentBox.hpp"
#include "FontAwesomeIcon.hpp"
#include "Collapse.hpp"
#include "ToggleSwitch.hpp"

/*
 * TODO: Find a way for input callbacks to remain active
 * after a ConfigPanel object goes out of scope.
 */

namespace emp {
namespace prefab {

  namespace internal {
      /**
       * Shared pointer held by instances of ConfigPanel class representing
       * the same conceptual ConfigPanel DOM object.
       * Contains state that should persist while ConfigPanel DOM object
       * persists.
       */
      class ConfigPanelInfo : public web::internal::DivInfo {

      public:
        using on_change_fun_t = std::function<void(const std::string & val)>;

      private:
        on_change_fun_t on_change_fun{ [](const std::string & val) { ; } };

      public:
        /**
         * Construct a shared pointer to manage ConfigPanel state.
         *
         * @param in_id HTML ID of ConfigPanel div
         */
        ConfigPanelInfo( const std::string & in_id="" )
        : web::internal::DivInfo(in_id) { ; }

        /**
         * Get current on-update callback for a ConfigPanel.
         *
         * @return current callback function handle
         */
        on_change_fun_t & GetOnChangeFun() { return on_change_fun; }

        /**
         * Set on-update callback for a ConfigPanel.
         *
         * @param fun callback function handle
         */
        void SetOnChangeFun(const on_change_fun_t & fun) {
          on_change_fun = fun;
        }

      };

  }

  /**
   * Use the ConfigPanel class to easily add a dynamic configuration
   * panel to your web app. Users can interact with the config panel
   * by updating values.
   */
  class ConfigPanel : public web::Div {
    public:
      using on_change_fun_t = internal::ConfigPanelInfo::on_change_fun_t;

    private:
      /**
       * Type of shared pointer shared among instances of ConfigPanel
       * representing the same conceptual DOM element.
       */
      using INFO_TYPE = internal::ConfigPanelInfo;

      /**
       * Get shared info pointer, cast to ConfigPanel-specific type.
       *
       * @return cast pointer
       */
      INFO_TYPE * Info() {
        return dynamic_cast<INFO_TYPE *>(info);
      }

      /**
       * Get shared info pointer, cast to const ConfigPanel-specific type.
       *
       * @return cast pointer
       */
      const INFO_TYPE * Info() const {
        return dynamic_cast<INFO_TYPE *>(info);
      }

      inline static std::set<std::string> numeric_types = {"int", "double", "float", "uint32_t", "uint64_t", "size_t"};
      Config & config;
      web::Div settings_div;
      std::set<std::string> exclude;
      std::map<std::string, web::Div> group_divs;
      std::map<std::string, web::Div> input_divs;

      std::function<std::string(std::string val)> format_label_fun = [](std::string name) {
        emp::vector<std::string> sliced = slice(name, '_');
        return to_titlecase(join(sliced, " "));
      };

      /**
       * For ints and doubles, there are three inputs:
       * a number input and two slider inputs.
       * SyncForm updates two inputs with the new value in the third input.
       *
       * @param val new value to be update the inputs with
       * @param input1 ID of one input that needs its value updated
       * @param input2 ID of the second input that needs its value updated
       */
      void SyncForm(const std::string val, const std::string input1, const std::string input2) {
          emp::web::Input div1(settings_div.Find(input1));
          div1.Value(val);
          emp::web::Input div2(settings_div.Find(input2));
          div2.Value(val);
          div1.Redraw();
          div2.Redraw();
      }

      /**
       * Get current on-update callback.
       *
       * @return current callback function handle
       */
      on_change_fun_t& GetOnChangeFun() {
          return Info()->GetOnChangeFun();
      };

      /**
       * Run on-update callback.
       *
       * @param val TODO what is this?
       */
      void DoOnChangeFun(const std::string & val) {
          Info()->GetOnChangeFun()(val);
      };

      /**
       * Add toggle glyphs and title for setting
       *
       * @param name setting's name
       * @param setting_element Div for all of setting's contents
       * @param title Button for setting's glyphs and title
       */
      void AddSettingLabel(
        const std::string & name,
        web::Div & setting_element,
        web::Element & title
      ) {
        input_divs[name] << setting_element;
        setting_element.SetAttr("class", "setting_element");
        title.AddAttr("class", "btn btn-link");

        prefab::FontAwesomeIcon arrow_right_for_dropdown("fa-angle-double-right");
        title << arrow_right_for_dropdown;
        prefab::FontAwesomeIcon arrow_up_for_dropdown("fa-angle-double-up");
        title << arrow_up_for_dropdown;
        title << format_label_fun(name);

        arrow_right_for_dropdown.AddAttr("class", "toggle_icon_right_margin");
        arrow_up_for_dropdown.AddAttr("class", "toggle_icon_right_margin");
      }

      /**
       * Add numeric specific inputs to setting element.
       *
       * @param name setting's name
       * @param setting_element Div for all of setting's contents
       * @param box CommentBox for setting's dropdown section
       * @param type string representing the settting's type (int, double, float)
       * @param value string representing the numerical value of the setting
       */
      void AddNumericSetting(
        const std::string & name,
        web::Div & setting_element,
        prefab::CommentBox & box,
        const std::string & type,
        const std::string & value
      ){
        const std::string name_input_slider = name + "_input_slider";
        const std::string name_input_number = name + "_input_number";
        const std::string name_input_mobile_slider = name + "_input_mobile_slider";
        web::Input slider( [](std::string x) {
          std::cout << "empty slider function" << std::endl;},
          "range",
          "",
          name_input_slider
        );
        setting_element << slider;

        web::Input number([](std::string val) {
          std::cout << "empty number function" << std::endl;
          },
          "number",
          "",
          name_input_number
        );
        setting_element << number;
        web::Input mobile_slider([](std::string val) {
          std::cout << "empty mobile slider function" << std::endl;
          },
          "range",
          "",
          name_input_mobile_slider
        );
        box.AddMobileContent("<hr>");
        box.AddMobileContent(mobile_slider);

        // Set onchange behavior for inputs
        slider.Callback(
          [this,name, name_input_number, name_input_mobile_slider](std::string val) {
          config.Set(name, val);
          SyncForm(val, name_input_number, name_input_mobile_slider);
          });
        number.Callback(
          [this,name, name_input_slider, name_input_mobile_slider](std::string val) {
          config.Set(name, val);
          SyncForm(val, name_input_slider, name_input_mobile_slider);
          });
        mobile_slider.Callback(
          [this,name, name_input_number, name_input_slider](std::string val) {
          config.Set(name, val);
          SyncForm(val, name_input_number, name_input_slider);
          });
        // Set initial values
        slider.Value(config.Get(name));
        number.Value(config.Get(name));
        mobile_slider.Value(config.Get(name));
        slider.SetAttr("class", "input_slider");
        number.SetAttr("class", "input_number");


        SetDefaultNumericInputs(type, value, slider, number, mobile_slider);
      }

      /**
       * Sets the desktop slider, mobile slider, and number
       * input for numeric settings to the default value
       * provided by the linked config file.
       *
       * @param type string representing the settting's type (int, double, float)
       * @param value string representing the numerical value of the setting
       * @param slider reference setting's desktop slider input
       * @param number reference setting's number input
       * @param mobile_slider reference setting's mobile slider input
       */
      void SetDefaultNumericInputs(
        const std::string & type,
        const std::string & value,
        web::Input & slider,
        web::Input & number,
        web::Input & mobile_slider
      ){
        // Attempt to have intelligent defaults
        if (type == "double") {
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

      /**
       * Add prefab ToggleSwitch to boolean setting element.
       *
       * @param name setting's name
       * @param setting_element Div for all of setting's contents
       * @param value string representing the numerical value of the setting
       */
      void AddBoolSetting(
        const std::string & name,
        web::Div & setting_element,
        const std::string & value
      ){
        // Bootstrap Toggle Switch
        emp::prefab::ToggleSwitch toggle_switch(
          [this, name](std::string val) {
            config.Set(name, val);
            DoOnChangeFun(val);
          },
          "",
          emp::from_string<bool>(value),
          name + "_input_checkbox"
        );
        setting_element << toggle_switch;
        toggle_switch.AddAttr("class", "input_bool");
      }

      /**
       * Add text input box to text setting element.
       *
       * @param name setting's name
       * @param setting_element Div for all of setting's contents
       */
      void AddTextSetting(
        const std::string & name,
        web::Div & setting_element
      ){
        web::Input text_input(
          [this, name](std::string val) {
            config.Set(name, val);
            DoOnChangeFun(val);
          },
          "text", "", name + "_input_textbox"
        );
        setting_element << text_input;
        text_input.SetAttr(
          "class", "input_text",
          "type", "text"
        );
        text_input.Value(config.Get(name));
      }


    public:
      /// @param c config panel to construct prefab ConfigPanel for
      ConfigPanel(
        Config & c,
        const std::string & div_name = "settings_div"
      ) : config(c)
      { info = new internal::ConfigPanelInfo(div_name); }

      /**
       * Sets on-update callback for a ConfigPanel.
       *
       * @param fun callback function handle
       */
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

      void SetDefaultRangeFixedPoint(web::Input & input, const int val) {
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

      void ExcludeConfig(const std::string setting) {
        exclude.insert(setting);
      }

      /**
       * Arranges config panel based configuration pass to constructor
       *
       * @param id_prefix string appended to id for each setting group Div
       */
      void Setup(const std::string & id_prefix = "settings_") {
        for (auto group : config.GetGroupSet()) {
          std::string group_name = group->GetName();
          group_divs[group_name] = web::Div(id_prefix + group_name);
          settings_div << group_divs[group_name];

          // Prefab Card
          prefab::Card card("INIT_OPEN");
          group_divs[group_name] << card;

          // Header content
          web::Div setting_heading;
          card.AddHeaderContent(setting_heading);
          setting_heading << "<h3>" + group->GetDesc() + "</h3>";
          setting_heading.SetAttr("class", "setting_heading");

          for (size_t i = 0; i < group->GetSize(); i++) {
            std::string name = group->GetEntry(i)->GetName();
            if (Has(exclude, name)) {
              continue;
            }
            std::string type = group->GetEntry(i)->GetType();
            std::string value = group->GetEntry(i)->GetValue();

            card.AddBodyContent(input_divs[name]);

            // Setting element label
            web::Div setting_element(name + "_row");
            web::Element title("button");
            title.SetAttr("class", "title_area");
            AddSettingLabel(name, setting_element, title);

            // Prefab Dropdown Box
            prefab::CommentBox box;
            box.AddContent(group->GetEntry(i)->GetDescription());

            // Prefab Collapse/toggle for setting element
            prefab::CollapseCoupling title_toggle(title, box, false, name + "_dropdown");
            input_divs[name] << title_toggle.GetTargetDiv(0);
            setting_element << title_toggle.GetControllerDiv(0);

            /*
             * There are 3 categories of settings that can be
             * added to a Config Panel: numeric, boolean, and text values.
             * Each category has unique components that should be
             * added for a setting of that type.
             *
             * Numeric values (ints, doubles, floats) should have a slider
             * for the desktop and mobile view as well as a numerical
             * input for the user to type their value.
             *
             * Boolean values should have a prefab toggle switch.
             *
             * Text values should have a textbox for the user to
             * type their input.
             */
            if (Has(numeric_types, type)) {
              AddNumericSetting(name, setting_element, box, type, value);
            } else if (type == "bool") {
              AddBoolSetting(name, setting_element, value);
            } else {
              AddTextSetting(name, setting_element);
            }
          }
        }
      }

      /// @return Div containing the entire config panel
      web::Div & GetConfigPanelDiv() { return settings_div; }

  };
}
}

#endif
