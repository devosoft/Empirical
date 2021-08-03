#ifndef EMP_TOGGLE_BUTTON_HPP
#define EMP_TOGGLE_BUTTON_HPP

#include "emp/web/Element.hpp"
#include "emp/web/Input.hpp"
#include "emp/tools/string_utils.hpp"

namespace emp::prefab {

  namespace internal {

    class ToggleButtonInfo : public web::internal::DivInfo {
      bool active;

      public:
      ToggleButtonInfo(const std::string & in_id) :
      web::internal::DivInfo(in_id) {;}

      bool IsActive() const {
        return active;
      }

      void SetActive() {
        active = true;
      }

      void SetInactive() {
        active = false;
      }
    }
  }

  class ToggleButton : public ButtonGroup {

    /**
     * Get shared info pointer, cast to ToggleButton-specific type.
     *
     * @return cast pointer
     */
    internal::ToggleButtonInfo * Info() {
      return dynamic_cast<internal::ToggleButtonInfo *>(info);
    }

    /**
     * Get shared info pointer, cast to const ToggleButton-specific type.
     *
     * @return cast pointer
     */
    const internal::ToggleButtonInfo * Info() const {
      return dynamic_cast<internal::ToggleButtonInfo *>(info);
    }

    emp::web::Input activeRadioBtn;
    emp::web::Input inactiveRadioBtn;

    emp::web::Element activeLabel;
    emp::web::Element inactiveLabel;

    protected:
    ToggleButton(web::internal::DivInfo * info_ref)
    : ButtonGroup(info_ref),
    activeRadioBtn(
      [](){;}, "radio", "", emp::to_string(GetID(),"_active_radio")
    ), inactiveRadioBtn(
      [](){;}, "radio", "", emp::to_string(GetID(),"_inactive_radio"), false, true
    ), activeLabel(
      "label", emp::to_string(GetID(),"_active_label")
    ), inactiveLabel(
      "label", emp::to_string(GetID(),"_inactive_label")
    ) {
      *this << activeRadioBtn;
      *this << activeLabel;
      *this << inactiveRadioBtn;
      *this << inactiveLabel;
      activeRadioBtn.SetAttr(
        "class", "btn-check", "name", emp::to_string(GetID(),"_radios"),
        "autocomplete", "off"
      );
      inactiveRadioBtn.SetAttr(
        "class", "btn-check", "name", emp::to_string(GetID(),"_radios"),
        "autocomplete", "off"
      );
      activeLabel.SetAttr(
        "class", "btn", "for", emp::to_string(GetID(),"_active_label")
      );
      inactiveLabel.SetAttr(
        "class", "btn", "for", emp::to_string(GetID(),"_inactive_label")
      );
    }

    public:
    ToggleButton(std::string & in_id="")
    : ButtonGroup(new internal::ToggleButtonInfo(in_id)) { ; }

    bool IsActive() const {
      return Info()->IsActive();
    }
  };


}

#endif
