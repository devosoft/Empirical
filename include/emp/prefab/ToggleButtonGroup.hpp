#ifndef EMP_TOGGLE_BUTTON_GROUP_HPP
#define EMP_TOGGLE_BUTTON_GROUP_HPP


#include "emp/tools/string_utils.hpp"
#include "emp/web/Div.hpp"
#include "emp/web/Element.hpp"
#include "emp/web/Input.hpp"
#include "emp/prefab/ButtonGroup.hpp"

namespace emp::prefab {

  namespace internal {
    using on_toggle_t = std::function<void(bool)>;

    class ToggleButtonGroupInfo : public web::internal::DivInfo {
      on_toggle_t callback;
      bool active;

      public:
      ToggleButtonGroupInfo(const std::string & in_id)
      : web::internal::DivInfo(in_id), callback([](bool){ ; }), active(false) { ; }

      void UpdateCallback(const on_toggle_t & cb) {
        callback = cb;
      }

      const on_toggle_t & GetCallback() const {
        return callback;
      }

      bool IsActive() const {
        return active;
      }

      void SetActive(const bool & val) {
        active = val;
      }
    };
  }

  class ToggleButtonGroup : public ButtonGroup {
    using on_toggle_t = internal::on_toggle_t;

    /**
     * Get shared info pointer, cast to ToggleButton-specific type.
     *
     * @return cast pointer
     */
    internal::ToggleButtonGroupInfo * Info() {
      return dynamic_cast<internal::ToggleButtonGroupInfo *>(info);
    }

    /**
     * Get shared info pointer, cast to const ToggleButton-specific type.
     *
     * @return cast pointer
     */
    const internal::ToggleButtonGroupInfo * Info() const {
      return dynamic_cast<internal::ToggleButtonGroupInfo *>(info);
    }

    protected:
    /**
     * @param activate_indicator a string, FontAwesomeIcon or other component
     * indicating that the first button actives this toggle
     * @param deactivate_indicator a string, FontAwesomeIcon or other component
     * indicating that the second button deactivates this toggle
     * @param activate_style a bootstrap style (primary, secondary, etc) for
     * the first button
     * @param deactivate_style a bootstrap style (primary, secondary, etc) for
     * the second button
     * @param cassette_style whether the toggle should display in cassette style
     * (both buttons visible) or do a swap on toggle (one button visible)
     * @param grayout in cassette mode, whether buttons should be
     * grayed out to further emphasize the current state
     */
    template<typename L1_TYPE, typename L2_TYPE>
    ToggleButtonGroup(
      L1_TYPE && activate_indicator,
      L2_TYPE && deactivate_indicator,
      const std::string & activate_style,
      const std::string & deactivate_style,
      const bool & cassette_style,
      const bool & grayout,
      web::internal::DivInfo * info_ref
    ) : ButtonGroup(info_ref)
    {
      AddAttr(
        "class", "btn-group-toggle", "data-toggle", "buttons"
      );

      web::Element activate_label("label", emp::to_string(GetID(), "_activate"));
      web::Element deactivate_label("label", emp::to_string(GetID(), "_deactivate"));
      *this << activate_label;
      *this << deactivate_label;

      if (!cassette_style) {
        AddAttr("class", "hide_inactive");
      } else if (grayout) {
        AddAttr("class", "grayout");
      }

      auto & on_toggle = GetCallback();

      activate_label.AddAttr(
        "class", "btn",
        "class", emp::to_string("btn-outline-", activate_style)
      );
      activate_label.OnClick([tog=*this, &handle_toggle=on_toggle]() mutable {
        tog.SetActive(true);
        handle_toggle(true);
      });

      web::Input activate_radio(
        [](std::string){ ; },
        "radio", "", emp::to_string(GetID(), "_activate_radio"),
        false, false
      );
      activate_label << activate_radio;
      activate_label << std::forward<L1_TYPE>(activate_indicator);

      deactivate_label.AddAttr(
        "class", "active",
        "class", "btn",
        "class", emp::to_string("btn-outline-", deactivate_style)
      );
      deactivate_label.OnClick([tog=*this, &handle_toggle=on_toggle]() mutable {
        tog.SetActive(false);
        handle_toggle(false);
      });

      web::Input deactivate_radio(
        [](std::string){ ; },
        "radio", "", emp::to_string(GetID(), "_deactivate_radio"),
        false, true
      );
      deactivate_label << deactivate_radio;
      deactivate_label << std::forward<L2_TYPE>(deactivate_indicator);
    }

    public:
    template<typename L1_TYPE, typename L2_TYPE>
    ToggleButtonGroup(
      L1_TYPE && activate_indicator,
      L2_TYPE && deactivate_indicator,
      const std::string & activate_style="success",
      const std::string & deactivate_style="warning",
      const bool & cassette_style=true,
      const bool & grayout=false,
      const std::string & in_id=""
    ) : ToggleButtonGroup(
      std::forward<L1_TYPE>(activate_indicator),
      std::forward<L2_TYPE>(deactivate_indicator),
      activate_style, deactivate_style,
      cassette_style, grayout,
      new internal::ToggleButtonGroupInfo(in_id)
    ) { ; }

    bool IsActive() const {
      return Info()->IsActive();
    }

    void SetActive(const bool & val) {
      Info()->SetActive(val);
    }

    const on_toggle_t & GetCallback() const {
      return Info()->GetCallback();
    }

    ToggleButtonGroup & SetCallback(const on_toggle_t & cb) {
      Info()->UpdateCallback(cb);
      return (*this);
    }
  };
}

#endif
