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
#include "ValueBox.hpp"

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
        using on_change_fun_t = std::function<void(const std::string &, const std::string &)>;

      private:
        on_change_fun_t on_change_fun{ [](const std::string & name, const std::string & val) { ; } };

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
      inline static std::function<std::string(std::string val)> format_label = [](std::string name) {
        return to_titlecase(join(slice(name, '_'), " "));
      };

      bool init_open;

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
      void DoOnChangeFun(const std::string & name, const std::string & val) {
          Info()->GetOnChangeFun()(name, val);
      };

    public:
      /// @param c config panel to construct prefab ConfigPanel for
      ConfigPanel(
        Config & config,
        bool _init_open = true,
        const std::string & div_name = ""
      ) : init_open(_init_open) {
        info = new internal::ConfigPanelInfo(div_name);
        this->AddAttr("class", "config_main");
        std::string this_id = GetID();

        on_change_fun_t & onChangeRef = GetOnChangeFun();

        for (auto group : config.GetGroupSet()) {
          std::string group_name = group->GetName();
          std::string pretty_group_name = format_label(group_name);
          std::string group_base = emp::to_string(this_id, "_", group_name);

          Card group_card(init_open ? "INIT_OPEN" : "INIT_CLOSED", true, group_base);
          group_card.AddHeaderContent(pretty_group_name);
          (*this) << group_card;
          group_card.AddAttr("class", "config_card");
          Div settings;
          settings.AddAttr("class", "settings_group");
          group_card << settings;
          for (size_t i = 0; i < group->GetSize(); i++) {
            auto setting = group->GetEntry(i);
            std::string name = setting->GetName();
            std::string pretty_name = format_label(name);
            std::string type = setting->GetType();
            std::string desc = setting->GetDescription();
            std::string value = setting->GetValue();
            std::string setting_base = emp::to_string(this_id, "_", name);

            auto handleChange = [name, &config, &handleChange = onChangeRef](const std::string & val) {
              handleChange(name, val);
              config.Set(name, val);
            };

            if (Has(numeric_types, type)) {
              settings << NumericValueControl(
                pretty_name, desc, value, handleChange, setting_base
              );
            } else if (type == "std::string") {
              settings << TextValueControl(
                pretty_name, desc, value, handleChange, setting_base
              );
            } else if (type == "bool") {
              settings << BoolValueControl(
                pretty_name, desc, emp::from_string<bool>(value), handleChange, setting_base
              );
            } else {
              settings << ValueDisplay(pretty_name, desc, value, setting_base);
            }
          }
        }
        Div controls{emp::to_string(GetID(), "_", "controls")};
        controls.AddAttr("class", "config_controls");

        web::Button reset_button{ [&config](){
          std::stringstream ss;
          config.WriteUrlQueryString(ss);
          const std::string tmp(ss.str());
          const char* cstr = tmp.c_str();
          EM_ASM({
            window.location.href = UTF8ToString($0);
          }, cstr);
         }, "Reset with changes", emp::to_string(GetID(), "_", "reset")};

        reset_button.SetAttr("class", "btn btn-danger");
        controls << reset_button;
        (*this) << controls;
      }

      /**
       * Sets on-update callback for a ConfigPanel.
       *
       * @param fun callback function handle
       */
      void SetOnChangeFun(const on_change_fun_t& fun) {
        Info()->SetOnChangeFun(fun);
      }

      /**
       * Excludes a setting or group of settings, recommend using ExcludeSetting
       * or ExcludeGroup instead
       *
       * @param setting The name of a single setting that should not be
       * displayed in the config panel
       * @note renamed to ExcludeSetting
       */
      void ExcludeConfig(const std::string & setting) {
        ExcludeSetting(setting);
      }

      /**
       * Excludes a specific setting from the config panel
       *
       * @param setting name of the setting that should not be
       * displayed in the config panel
       */
      void ExcludeSetting(const std::string & setting) {
        ((Div)this->Find(emp::to_string(GetID(), "_", setting))).AddAttr("class", "excluded");
      }

      /**
       * Excludes an entire group of settings from the config panel
       *
       * @param setting_group name of the group that should not be
       * displayed in the config panel
       */
      void ExcludeGroup(const std::string & setting_group) {
        ((Div)this->Find(emp::to_string(GetID(), "_", setting_group))).AddAttr("class", "excluded");
      }

      /**
       * Arranges config panel based configuration pass to constructor
       *
       * @param id_prefix string appended to id for each setting (unusued)
       * @deprecated No longer necessary for config panel to function.
       * This function was a work around to try and fix a bug.
       */
      [[deprecated("Prefer construction of ConfigPanel after config values have been set")]]
      void Setup(Config & config, const std::string & id_prefix = "") {
        *this = ConfigPanel(config, init_open, id_prefix);
      }

      /** @return Div containing the entire config panel
       *  @deprecated Can directly stream this component
       */
      [[deprecated("Can directly stream this component into another")]]
      web::Div & GetConfigPanelDiv() { return (*this); }

  };
}
}

#endif
