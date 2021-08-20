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
    using checker_func_t = std::function<bool(const web::Animate &)>;
    /**
     * Shared pointer held by instances of ControlPanel class representing
     * the same conceptual ControlPanel DOM object.
     * Contains state that should persist while ControlPanel DOM object
     * persists.
     */
    class ControlPanelInfo : public web::internal::DivInfo {

      // The unit for rate of refresh
      std::string refresh_unit;

      // A list of refresh rates with current value associated with current unit
      std::map<std::string, int> refresh_rates{
        {"MILLISECONDS", 100}, {"FRAMES", 5}
      };

      // Map units to cool refresh checkers
      const std::map<std::string, const checker_func_t> refresh_checkers{
        { "MILLISECONDS",
          [elapsed_milliseconds = 0, this]
          (const web::Animate & anim) mutable {
            int rate = this->refresh_rates[this->refresh_unit];
            elapsed_milliseconds += anim.GetStepTime();
            if (elapsed_milliseconds > rate) {
              elapsed_milliseconds -= rate;
              if (elapsed_milliseconds > rate) elapsed_milliseconds = 0;
              return true;
              // If this is weird check out explanation of pattern in ReadoutPanel
            }
            return false;
          }},
        { "FRAMES",
          [this](const web::Animate & anim) {
            return anim.GetFrameCount() % this->refresh_rates[this->refresh_unit];
          }
        }
      };

      // The current redraw checker function
      checker_func_t do_redraw;

      // A list of widget that should be redraw when do_redraw return true
      emp::vector<web::Widget> refresh_list;

      // A function to run every frame (as fast as possible)
      std::function<void()> simulation;

      public:
      /**
       * Construct a shared pointer to manage ControlPanel state.
       *
       * @param in_id HTML ID of ConfigPanel div
       */
      ControlPanelInfo(const std::string & in_id="")
      : DivInfo(in_id),
      refresh_unit("MILLISECONDS"),
      do_redraw(refresh_checkers.at(refresh_unit)),
      simulation([](){ ; })  { ; }

      /**
       * Get a reference to the redraw checker function
       * @return a redraw checker (void(const Animate &) function)
       */
      const checker_func_t & GetRedrawChecker() const {
        return do_redraw;
      }

      /**
       * Set the simulation for this control panel
       * @param sim the function to be run every frame (as fast as possible)
       */
      void SetSimulation(const std::function<void()> & sim) {
        simulation = sim;
      }

      /**
       * Get the simulation for this control panel
       * @return the function to be run every frame (as fast as possible)
       */
      const std::function<void()> & GetSimulation() const {
        return simulation;
      }

      /**
       * Set the refresh rate units for this control panel
       * @param unit either "MILLISECONDS" or "FRAMES"
       */
      void SetUnit(const std::string & unit) {
        refresh_unit = unit;
        do_redraw = refresh_checkers.at(refresh_unit);
      }

      /**
       * Set the refresh rate for this control panel
       * @param rate the number of milliseconds or frames between refreshes
       */
      void SetRate(const int & rate) {
        refresh_rates[refresh_unit] = rate;
      }

      /**
       * Get the refresh list for this control panel
       * @return a list of Widgets that will be refreshed every update period
       */
      emp::vector<web::Widget> & GetRefreshList() {
        return refresh_list;
      }
    };
  }
  /**
   * Use the ConfigPanel class to add a play/pause toggle button and a step
   * button to your application. You can add a simulation to be run, web
   * components to be redrawn, and more Buttons or ButtonGroups to add more
   * functionality.
   */
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
    /**
     * The protected contructor for a Control panel that sets up the state
     * and event handlers
     * @param refresh_mode units of "MILLISECONDS" or "FRAMES"
     * @param refresh_rate the number of milliseconds or frames between refreshes
     * @param in_info info object associated with this component
     */
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
    step{
      [](){ ; },
      "<span class=\"fa fa-step-forward\" aria-hidden=\"true\"></span>",
      emp::to_string(GetID(), "_step")
    }
    {
      AddAttr(
        "class", "btn-toolbar",
        "class", "space_groups",
        "role", "toolbar",
        "aria-label", "Toolbar with simulation controls"
      );
      SetRefreshRate(refresh_rate, refresh_mode);

      static_cast<Div>(*this) << button_line;
      button_line << toggle_run;
      button_line << step;

      AddAnimation(GetID(),
        [&run_sim=GetSimulation(),
          &refresh_list=Info()->GetRefreshList(),
          &do_redraw=Info()->GetRedrawChecker()]
        (const web::Animate & anim) mutable {
          // Run the simulation function every frame
          run_sim();
          // Redraw widgets according to a rule
          if(do_redraw(anim)) {
            for (auto & wid : refresh_list) {
              wid.Redraw();
            }
          }
        }
      );

      toggle_run.SetCallback(
        [&anim=Animate(GetID()), step=web::Button(step)]
        (bool is_active) mutable {
          if (is_active) {
            anim.Start();
          } else {
            anim.Stop();
          }
        }
      );

      step.SetCallback([&anim=Animate(GetID())]() {
        anim.Step();
      });
    }

    public:
    /**
     * Contructor for a Control panel.
     * @param refresh_mode units of "MILLISECONDS" or "FRAMES"
     * @param refresh_rate the number of milliseconds or frames between refreshes
     * @param in_id HTML ID of control panel div
     */
    ControlPanel(
      const std::string & refresh_mode,
      const int & refresh_rate,
      const std::string & in_id="")
    : ControlPanel(
      refresh_mode,
      refresh_rate,
      new internal::ControlPanelInfo(in_id)
    ) { ; }

    /**
     * Set the simulation for this control panel
     * @param sim the function to be run every frame (as fast as possible)
     */
    ControlPanel & SetSimulation(const std::function<void()> & sim) {
      Info()->SetSimulation(sim);
      return *this;
    }

    /**
     * Get the simulation for this control panel
     * @return the function to be run every frame (as fast as possible)
     */
    const std::function<void()> & GetSimulation() const {
      return Info()->GetSimulation();
    }

    /**
     * Set the refresh rate units for this control panel.
     * @param unit either "MILLISECONDS" or "FRAMES"
     * @note rates are independent for "MILLISECONDS" and "FRAMES"
     */
    ControlPanel & SetRefreshUnit(const std::string & units) {
      Info()->SetUnit(units);
      return *this;
    }

    /**
     * Set the refresh rate for this control panel for the current unit.
     * @param rate period in frames or milliseconds
     * @note rates are independent for "MILLISECONDS" and "FRAMES".
     */
    void SetRefreshRate(const int & rate) {
      Info()->SetRate(rate);
    }

    /**
     * Set the refresh rate for this control panel.
     * @param rate the number of milliseconds or frames between refreshes
     * @param unit either "MILLISECONDS" or "FRAMES"
     * @note rates are independent for "MILLISECONDS" and "FRAMES"
     */
    void SetRefreshRate( const int & rate, const std::string & units) {
      Info()->SetUnit(units);
      Info()->SetRate(rate);
    }

    /**
     * Adds a Widget to a list of widgets redrawn at the specified refresh rate.
     * @param area a widget
     */
    void AddToRefreshList(Widget & area) {
      Info()->GetRefreshList().push_back(area);
    }

    /**
     * Stream operator to add a component to the control panel.
     *
     * Some special behavior: Buttons and ToggleButtonGroups will be added
     * to the last ButtonGroup added to keep related components together.
     * If you want to start a new group, just stream in a new ButtonGroup.
     * @param in_val a component to be added to the control panel
     */
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
