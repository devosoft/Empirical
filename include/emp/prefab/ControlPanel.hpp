/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file  ControlPanel.hpp
 *  @brief A ControlPanel manages ...
 */

#ifndef EMP_CONTROL_PANEL_HPP
#define EMP_CONTROL_PANEL_HPP

#include <functional>
#include <unordered_map>
#include <utility>

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

class ControlPanel;

namespace internal {

  /**
   * Shared pointer held by instances of ControlPanel class representing
   * the same conceptual ControlPanel DOM object.
   * Contains state that should persist while ControlPanel DOM object
   * persists.
   */
  class ControlPanelInfo : public web::internal::DivInfo {
    friend ControlPanel;
    /**
     * A RefreshChecker is a functor that accepts an Animate reference
     * and returns a boolean indicating whether components should be redrawn
     * based on an internal "rate" for redrawing.
     */
    class RefreshChecker {
      protected:
      int rate;
      public:
      RefreshChecker(int r) { rate = r; }
      virtual bool operator()(const web::Animate &) {
        return true;
      }
      void SetRate(int r) {
        rate = r;
      }
      int GetRate() const { return rate; }
    };

    // ControlPanelInfo holds contains two specific instances of a
    // RefreshChecker for milliseconds and frames to keep rates independent
    // for each.
    class : public RefreshChecker {
      using RefreshChecker::RefreshChecker;
      int elapsed_milliseconds = 0;
      bool operator()(const web::Animate & anim) {
        elapsed_milliseconds += anim.GetStepTime();
          if (elapsed_milliseconds > rate) {
            elapsed_milliseconds -= rate;
            if (elapsed_milliseconds > rate) elapsed_milliseconds = 0;
            return true;
          }
          return false;
      }
    } millisecond_refresher{100};
    class : public RefreshChecker {
      using RefreshChecker::RefreshChecker;
      bool operator()(const web::Animate & anim) {
        return anim.GetFrameCount() % rate;
      }
    } frame_refresher{5};

    // The current redraw checker function
    RefreshChecker should_redraw;

    // A list of widget that should be redrawn when do_redraw return true
    emp::vector<web::Widget> refresh_list;

    // A function to run every frame (as fast as possible)
    std::function<void()> step_callback;

    /**
     * Construct a shared pointer to manage ControlPanel state.
     *
     * @param in_id HTML ID of ConfigPanel div
     */
    ControlPanelInfo(const std::string & in_id="")
    : DivInfo(in_id),
    should_redraw(millisecond_refresher),
    step_callback([](){ ; })  { ; }

    /**
     * Get a constant reference to the redraw checker function
     * @return a redraw checker
     */
    const RefreshChecker & GetRefreshChecker() const {
      return should_redraw;
    }

    /**
     * Get a reference to the redraw checker function
     * @return a redraw checker
     */
    RefreshChecker & GetRefreshChecker() {
      return should_redraw;
    }

    /**
     * Gets a void callback function that will be called every frame (as often as possible).
     * @return the void callback function to advance the state of some simulation or process
     * by one update every call.
     */
    const std::function<void()> & GetStepCallback() const {
      return step_callback;
    }

    /**
     * Get the refresh list for this control panel
     * @return a list of Widgets that will be refreshed every update period
     */
    emp::vector<web::Widget> & GetRefreshList() {
      return refresh_list;
    }

    public:
    /**
     * Set the redraw checker function
     * @param check a redraw checker
     */
    void SetRefreshChecker(const RefreshChecker & check) {
      should_redraw = check;
    }

    /**
     * Set the redraw checker function to milliseconds instance
     */
    void SetRefreshCheckerMilliseconds() {
      should_redraw = millisecond_refresher;
    }

    /**
     * Set the redraw checker function to frames instance
     */
    void SetRefreshCheckerFrames() {
      should_redraw = frame_refresher;
    }

    /**
     * Sets a void callback function that will be called every frame (as often as possible).
     * @param step the void callback function to advance the state of some simulation or process
     * by one update every call.
     */
    void SetStepCallback(const std::function<void()> & step) {
      step_callback = step;
    }
  };
} // namspace internal

  /**
   * Use the ConfigPanel class to add a play/pause toggle button and a step
   * button to your application. You can add a simulation to be run, web
   * components to be redrawn, and more Buttons or ButtonGroups to add more
   * functionality to the control panel.
   */
  class ControlPanel : public web::Div {
    public:
    using RefreshChecker = internal::ControlPanelInfo::RefreshChecker;

    private:
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

    ToggleButtonGroup toggle_run; // The main toggle to stop/start animation
    ButtonGroup button_line; // Current button group to which buttons are streamed
    web::Button step; // Button to step forward animation

    protected:
    /**
     * The protected contructor for a Control panel that sets up the state
     * and event handlers. For internal use only. See the prefab/README.md for
     * more information on this design pattern.
     *
     * @param refresh_mode units of "MILLISECONDS" or "FRAMES"
     * @param refresh_rate the number of milliseconds or frames between refreshes
     * @param in_info info object associated with this component
     */
    ControlPanel(
      const int & refresh_rate,
      const std::string & refresh_unit,
      web::internal::DivInfo * in_info
    ) : web::Div(in_info),
    toggle_run{
      FontAwesomeIcon{"fa-play"}, FontAwesomeIcon{"fa-pause"},
      "success", "warning",
      true, false,
      emp::to_string(GetID(), "_main_toggle")
    },
    button_line(ButtonGroup{emp::to_string(GetID(), "_main")}),
    step{
      [](){ ; },
      "<span class=\"fa fa-step-forward\" aria-hidden=\"true\"></span>",
      emp::to_string(GetID(), "_main_step")
    }
    {
      AddAttr(
        "class", "btn-toolbar",
        "class", "space_groups",
        "role", "toolbar",
        "aria-label", "Toolbar with simulation controls"
      );
      SetRefreshRate(refresh_rate, refresh_unit);
      step.AddAttr(
        "class", "btn",
        "class", "btn-success"
      );

      static_cast<Div>(*this) << button_line;
      button_line << toggle_run;
      button_line << step;

      AddAnimation(GetID(),
        [&step_simulation=Info()->GetStepCallback(),
          &refresh_list=Info()->GetRefreshList(),
          &should_redraw=Info()->GetRefreshChecker()]
        (const web::Animate & anim) mutable {
          // Run the simulation function every frame
          step_simulation();
          // Redraw widgets according to a rule
          if (should_redraw(anim)) {
            for (auto & widget : refresh_list) {
              widget.Redraw();
            }
          }
        }
      );

      toggle_run.SetCallback(
        [&anim=Animate(GetID()), step=web::Button(step)]
        (const bool & is_active) mutable {
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
     * @param refresh_rate the number of milliseconds or frames between refreshes
     * @param refresh_mode units of "MILLISECONDS" or "FRAMES"
     * @param in_id HTML ID of control panel div
     */
    ControlPanel(
      const int & refresh_rate,
      const std::string & refresh_unit,
      const std::string & in_id="")
    : ControlPanel(
      refresh_rate,
      refresh_unit,
      new internal::ControlPanelInfo(in_id)
    ) { ; }

    /**
     * Sets a void callback function that will be called every frame (as often as possible).
     * @param step the void callback function to advance the state of some simulation or process
     * by one update every call.
     */
    ControlPanel & SetStepCallback(const std::function<void()> & step) {
      Info()->SetStepCallback(step);
      return *this;
    }

    /**
     * Set the refresh rate units for this control panel.
     * @param unit either "MILLISECONDS" or "FRAMES"
     * @note rates are independent for "MILLISECONDS" and "FRAMES" so changing
     * units may also change the rate.
     */
    ControlPanel & SetRefreshUnit(const std::string & units) {
      if (units == "MILLISECONDS") {
        Info()->SetRefreshCheckerMilliseconds();
      } else if (units == "FRAMES") {
        Info()->SetRefreshCheckerFrames();
      }
      return *this;
    }

    /**
     * Set the refresh rate for this control panel for the current unit.
     * @param rate period in frames or milliseconds
     * @note rates are independent for "MILLISECONDS" and "FRAMES".
     */
    void SetRefreshRate(const int & rate) {
      Info()->GetRefreshChecker().SetRate(rate);
    }

    /**
     * Set the refresh rate for this control panel.
     * @param rate the number of milliseconds or frames between refreshes
     * @param unit either "MILLISECONDS" or "FRAMES"
     * @note rates are independent for "MILLISECONDS" and "FRAMES".
     */
    void SetRefreshRate(const int & rate, const std::string & units) {
      SetRefreshUnit(units);
      SetRefreshRate(rate);
    }
    /**
     * Give the control panel a custom method to determine what frames to
     * refresh on.
     * @param check an instance of a RefreshChecker
     */
    ControlPanel & SetRefreshChecker(const RefreshChecker & check) {
      Info()->SetRefreshChecker(check);
      return *this;
    }

    /**
     * Gets the current refresh rate for the control panel.
     * @return the current refresh rate
     */
    int GetRate() const {
      return Info()->GetRefreshChecker().GetRate();
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
        button_line = std::forward<IN_TYPE>(in_val);
        static_cast<Div>(*this) << button_line;
      } else {
        static_cast<Div>(*this) << std::forward<IN_TYPE>(in_val);
      }
      return (*this);
    }
  };
} // namespace emp::prefab

#endif // #ifndef EMP_CONTROL_PANEL_HPP
