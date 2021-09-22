/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file  ToggleButtonGroup.hpp
 *  @brief ToggleButtonGroups maintain two button elements intended to represent two
 *  mutually exclusive states.
 */

#ifndef EMP_TOGGLE_BUTTON_GROUP_HPP
#define EMP_TOGGLE_BUTTON_GROUP_HPP

#include <functional>
#include <utility>

#include "emp/tools/string_utils.hpp"
#include "emp/web/Div.hpp"
#include "emp/web/Element.hpp"
#include "emp/web/Input.hpp"
#include "emp/prefab/ButtonGroup.hpp"

namespace emp::prefab {
namespace internal {
  using on_toggle_t = std::function<void(bool)>;
  /**
   * Shared pointer held by instances of ToggleButtonGroup class representing
   * the same conceptual ToggleButtonGroup DOM object.
   * Contains state that should persist while ToggleButtonGroup DOM object
   * persists.
   */
  class ToggleButtonGroupInfo : public web::internal::DivInfo {
    on_toggle_t callback; // A callback to be called when the component changes states
    bool is_active; // Whether the toggle is in the activated or deactivated state

    public:
    ToggleButtonGroupInfo(const std::string & in_id)
    : web::internal::DivInfo(in_id), callback([](bool){ ; }), is_active(false) { ; }

    /**
     * Set the function to be called when the component toggles
     * @param cb a callback function that accepts a boolean indicating
     * whether the toggle is active or inactive
     */
    void UpdateCallback(const on_toggle_t & cb) {
      callback = cb;
    }

    /**
     * Get the function to be called when the component toggles from activated
     * to deactivated.
     * @return a callback function
     */
    const on_toggle_t & GetCallback() const {
      return callback;
    }

    /**
     * Determines whether the toggle is in the active state
     * @return boolean
     */
    bool IsActive() const {
      return is_active;
    }

    /**
     * Sets this toggle to activated.
     */
    void SetActive() {
      is_active = true;
    }

    /**
     * Sets this toggle to deactivated.
     */
    void SetInactive() {
      is_active = false;
    }
  };
} // namespace internal

  /**
   * Use a ToggleButtonGroup to create a control with two labeled, visually
   * distinct states. Choose whether the button should display cassette-style
   * with two separate buttons or as a single button that flip-flops state.
   *
   * State can be accessed procedurally via IsActive() or in an event driven
   * manner by setting a callback via SetCallback().
   */
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
     * Protected constructor for a ToggleButton group. For internal use only.
     * See the prefab/README.md for more information on this design pattern.
     *
     * @param activate_indicator a string, FontAwesomeIcon or other component
     * indicating that the first button actives this toggle
     * @param deactivate_indicator a string, FontAwesomeIcon or other component
     * indicating that the second button deactivates this toggle
     * @param activate_style a bootstrap style (primary, secondary, etc) for
     * the first button
     * @param deactivate_style a bootstrap style (primary, secondary, etc) for
     * the second button
     * @param info_ref shared pointer containing presistent state
     */
    template<typename LABEL1_TYPE, typename LABEL2_TYPE>
    ToggleButtonGroup(
      LABEL1_TYPE && activate_indicator,
      LABEL2_TYPE && deactivate_indicator,
      const std::string & activate_style,
      const std::string & deactivate_style,
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

      auto & on_toggle = Info()->GetCallback();

      activate_label.AddAttr(
        "class", "btn",
        "class", emp::to_string("btn-outline-", activate_style)
      );
      activate_label.OnClick([tog=*this, &handle_toggle=on_toggle]() mutable {
        tog.SetActive();
        handle_toggle(true);
      });
      // OnClick used due to a strange bug(?) in Javascript in which the input
      // radios only fire their onchange function once. Probably due to some
      // Bootstrap/jQuery weirdness. This is a last minute work around ¯\_(ツ)_/¯.

      web::Input activate_radio(
        [](std::string){ ; },
        "radio", "", emp::to_string(GetID(), "_activate_radio"),
        false, false
      );
      activate_label << activate_radio;
      activate_label << std::forward<LABEL1_TYPE>(activate_indicator);

      deactivate_label.AddAttr(
        "class", "active",
        "class", "btn",
        "class", emp::to_string("btn-outline-", deactivate_style)
      );
      deactivate_label.OnClick([tog=*this, &handle_toggle=on_toggle]() mutable {
        tog.SetInactive();
        handle_toggle(false);
      });

      web::Input deactivate_radio(
        [](std::string){ ; },
        "radio", "", emp::to_string(GetID(), "_deactivate_radio"),
        false, true
      );
      deactivate_label << deactivate_radio;
      deactivate_label << std::forward<LABEL2_TYPE>(deactivate_indicator);
    }

    public:
    /**
     * @param activate_indicator a string, FontAwesomeIcon or other component
     * indicating that the first button activates this toggle
     * @param deactivate_indicator a string, FontAwesomeIcon or other component
     * indicating that the second button deactivates this toggle
     * @param activate_style a bootstrap style (primary, secondary, etc) for
     * the first button
     * @param deactivate_style a bootstrap style (primary, secondary, etc) for
     * the second button
     * @param in_id HTML ID of ToggleButtonGroup div
     */
    template<typename LABEL1_TYPE, typename LABEL2_TYPE>
    ToggleButtonGroup(
      LABEL1_TYPE && activate_indicator,
      LABEL2_TYPE && deactivate_indicator,
      const std::string & activate_style="success",
      const std::string & deactivate_style="warning",
      const std::string & in_id=""
    ) : ToggleButtonGroup(
      std::forward<LABEL1_TYPE>(activate_indicator),
      std::forward<LABEL2_TYPE>(deactivate_indicator),
      activate_style, deactivate_style,
      new internal::ToggleButtonGroupInfo(in_id)
    ) { ; }

    /**
     * Determines whether the toggle is activated or deactivated.
     * @return boolean
     */
    bool IsActive() const {
      return Info()->IsActive();
    }

    /**
     * Sets the state of the toggle to active.
     */
    void SetActive() {
      Info()->SetActive();
    }

    /**
     * Sets the state of the toggle to inactive.
     */
    void SetInactive() {
      Info()->SetInactive();
    }

    /**
     * Set the function to be called when the component toggles
     * @param cb a void callback function that accepts a boolean indicating
     * whether the toggle is active or inactive
     */
    ToggleButtonGroup & SetCallback(const on_toggle_t & cb) {
      Info()->UpdateCallback(cb);
      return (*this);
    }

    /**
     * Change styling from cassette style (buttons side-by-side) to single
     * button style so that button will swap between the two indicators.
     */
    ToggleButtonGroup & Compress() {
      AddAttr("class", "hide_inactive");
      return (*this);
    }

    /**
     * Add a grayscale filter to further emphasize the current state of the toggle.
     */
    ToggleButtonGroup & Grayout() {
      AddAttr("class", "grayout");
      return (*this);
    }
  };
} // namespace emp::prefab

#endif // #ifndef EMP_TOGGLE_BUTTON_GROUP_HPP
