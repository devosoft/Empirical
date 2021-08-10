#ifndef EMP_CONTROL_PANEL_HPP
#define EMP_CONTROL_PANEL_HPP

#include "emp/base/optional.hpp"
#include "emp/prefab/ButtonGroup.hpp"
#include "emp/prefab/ToggleButtonGroup.hpp"
#include "emp/prefab/FontAwesomeIcon.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/web/Div.hpp"

namespace emp::prefab {

  class ControlPanel : public web::Div {

    ToggleButtonGroup play_pause_toggle;
    optional<web::Div> active_group;

    protected:
    ControlPanel(web::internal::DivInfo * in_info) : web::Div(in_info),
    play_pause_toggle(ToggleButtonGroup{
      FontAwesomeIcon{"fa-play"}, FontAwesomeIcon{"fa-pause"},
      "success", "warning",
      true, false, emp::to_string(GetID(), "_play_pause")
    })
    {
      AddAttr(
        "class", "btn-toolbar",
        "class", "space_groups",
        "role", "toolbar",
        "aria-label", "Toolbar with simulation controls"
      );
      active_group = ButtonGroup{};
      *active_group << play_pause_toggle;
      static_cast<Div>(*this) << *active_group;
    }

    public:
    ControlPanel(const std::string & in_id="")
    : ControlPanel(new web::internal::DivInfo(in_id)) { ; }

    template <typename IN_TYPE>
    ControlPanel & operator<<(IN_TYPE && in_val) {
      // Took soooo long to figure out but if in_val is a r-value ref
      // IN_TYPE is just the TYPE. If it's l-value then it's TYPE &.
      // std::decay and forward help handle both.
      if constexpr(std::is_same<typename std::decay<IN_TYPE>::type, ButtonGroup>::value) {
        active_group.emplace(std::forward<ButtonGroup>(in_val));
        static_cast<Div>(*this) << *active_group;
      } else {
        if(!active_group.has_value()) {
          active_group = ButtonGroup{};
          static_cast<Div>(*this) << *active_group;
        }
        *active_group << std::forward<IN_TYPE>(in_val);
      }
      return (*this);
    }
  };
}

#endif
