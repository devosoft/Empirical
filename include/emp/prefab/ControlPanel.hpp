#ifndef EMP_CONTROL_PANEL_HPP
#define EMP_CONTROL_PANEL_HPP

#include "emp/base/optional.hpp"
#include "emp/base/vector.hpp"

#include "emp/prefab/ButtonGroup.hpp"
#include "emp/prefab/FontAwesomeIcon.hpp"
#include "emp/prefab/ToggleButtonGroup.hpp"

#include "emp/tools/string_utils.hpp"

#include "emp/web/Animate.hpp"
#include "emp/web/Element.hpp"
#include "emp/web/Div.hpp"

namespace emp::prefab {

  namespace internal {

    class ControlPanelInfo : public web::internal::DivInfo {

      std::string refresh_unit = "MILLISECONDS";
      std::map<std::string, int> refresh_rates{
        {"MILLISECONDS", 100}, {"FRAMES", 5}
      };

      emp::vector<web::Widget> refresh_list;
      std::function<void()> simulation;

      public:
      ControlPanelInfo(const std::string & in_id="")
      : DivInfo(in_id), simulation([](){ ; }) { ; }

      void SetSimulation(std::function<void()> & sim) {
        simulation = sim;
      }

      const std::function<void()> & GetSimulation() const {
        return simulation;
      }

      void SetUnit(const std::string & unit) {
        refresh_unit = unit;
      }

      const std::string & GetUnit() const {
        return refresh_unit;
      }

      void SetRate(const int & rate) {
        refresh_rates[refresh_unit] = rate;
      }

      const int & GetRate() {
        return refresh_rates[refresh_unit];
      }

      emp::vector<web::Widget> & GetRefreshList() {
        return refresh_list;
      }
    };
  }

  class ControlPanel : public web::Div {

    /**
     * Get shared info pointer, cast to ControlPanel-specific type.
     * @return cast pointer
     */
    internal::ControlPanelInfo * Info() {
      return dynamic_cast<internal::ControlPanelInfo *>(info);
    }

    /**
     * Get shared info pointer, cast to const ControlPanel-specific type.
     * @return cast pointer
     */
    const internal::ControlPanelInfo * Info() const {
      return dynamic_cast<internal::ControlPanelInfo *>(info);
    }

    ToggleButtonGroup toggle_run;
    Div button_line;
    web::Button step;

    protected:
    ControlPanel(
      const std::string & refresh_mode,
      const int & refresh_rate,
      web::internal::DivInfo * in_info
    ) : web::Div(in_info),
    toggle_run{
      FontAwesomeIcon{"fa-play"}, FontAwesomeIcon{"fa-pause"},
      "success", "warning",
      true, false,
      emp::to_string(GetID(), "_run_toggle")
    },
    button_line(ButtonGroup{emp::to_string(GetID(), "_core")}),
    step{[](){ ; }, "", emp::to_string(GetID(), "_step")}
    {
      AddAttr(
        "class", "btn-toolbar",
        "class", "space_groups",
        "role", "toolbar",
        "aria-label", "Toolbar with simulation controls"
      );
      SetUnit(refresh_mode);
      SetRate(refresh_rate);

      step.SetAttr("class", "success");
      step << FontAwesomeIcon{"fa-step-forward"};
      button_line << toggle_run;
      static_cast<Div>(*this) << button_line;

      AddAnimation(GetID(),
        [elapsed_milliseconds = 0,
          &run_sim=GetSimulation(),
          &refresh_list=Info()->GetRefreshList(),
          &unit=GetUnit(),
          &rate=GetRate()]
        (const web::Animate & anim) mutable {

          // Run the simulation function every frame
          run_sim();

          if (unit == "FRAMES") {
            // Units of frames means redraw every <rate> # of frames
            if (anim.GetFrameCount() % rate) {
              for (emp::web::Widget & wid : refresh_list) {
                wid.Redraw();
              }
            }
          } else {
            // Units of milliseconds means redraw every <rate> # of milliseconds
            elapsed_milliseconds += anim.GetStepTime();
            if (elapsed_milliseconds > rate) {
              elapsed_milliseconds -= rate;
              for (emp::web::Widget & wid : refresh_list) {
                wid.Redraw();
              }
            }
            // see ReadoutPanel for explanation of this pattern
            if (elapsed_milliseconds > rate) elapsed_milliseconds = 0;
          }
        }
      );

      toggle_run.SetCallback(
        [&anim=Animate(GetID()), step=web::Button(step)]
        (bool set_active) mutable {
          if (set_active) {
            anim.Start();
            step.SetAttr("disabled", true);
          } else {
            anim.Stop();
            step.SetAttr("disabled", true);
          }
        }
      );

      step.SetCallback([&anim=Animate(GetID())]() {
        anim.Step();
      });
    }

    public:
    ControlPanel(
      const std::string & refresh_mode,
      const int & refresh_rate,
      const std::string & in_id="")
    : ControlPanel(
      refresh_mode,
      refresh_rate,
      new internal::ControlPanelInfo(in_id)
    ) { ; }

    void SetSimulation(std::function<void()> & sim) {
      Info()->SetSimulation(sim);
    }

    const std::function<void()> & GetSimulation() const { return Info()->GetSimulation(); }

    void SetUnit(const std::string & units) { Info()->SetUnit(units); }

    const std::string & GetUnit() const { return Info()->GetUnit(); }

    void SetRate(const int & val) { Info()->SetRate(val); }

    const int & GetRate() { return Info()->GetRate(); }

    void AddToRefreshList(Widget & area) {
      Info()->GetRefreshList().push_back(area);
    }

    template <typename IN_TYPE>
    ControlPanel & operator<<(IN_TYPE && in_val) {
      // Took soooo long to figure out but if in_val is a r-value ref
      // IN_TYPE is just the TYPE. If it's l-value then it's TYPE &.
      // std::decay and forward help handle both.
      if constexpr(std::is_same<typename std::decay<IN_TYPE>::type, web::Button>::value ||
        std::is_same<typename std::decay<IN_TYPE>::type, ToggleButtonGroup>::value) {
        button_line << std::forward<IN_TYPE>(in_val);
      } else if constexpr(std::is_same<typename std::decay<IN_TYPE>::type, ButtonGroup>::value) {
        button_line = std::forward<ButtonGroup>(in_val);
        static_cast<Div>(*this) << button_line;
      } else {
        static_cast<Div>(*this) << std::forward<IN_TYPE>(in_val);
      }
      return (*this);
    }
  };
}

#endif
