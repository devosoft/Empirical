/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file  ControlPanel.hpp
 *  @brief A ControlPanel manages a container for buttons and an Animate instance
 *  that can run a callback function every frame and redraw a list of Widgets
 *  based on frames or milliseconds elapsed and a specified rate.
 *
 *  @todo The default control panel should eventually contain a select element
 *  to choose the rate unit and a numeric input for the rate. The two rates
 *  should be independent so that changing the unit to e.g. FRAMES will change
 *  the input to the FRAMES rate.
 */

#ifndef EMP_CONTROL_PANEL_HPP
#define EMP_CONTROL_PANEL_HPP

#include <functional>
#include <string>
#include <utility>
#include <unordered_map>

#include "../base/optional.hpp"
#include "../base/vector.hpp"
#include "../datastructs/DisjointVariant.hpp"
#include "../prefab/ButtonGroup.hpp"
#include "../prefab/FontAwesomeIcon.hpp"
#include "../prefab/ToggleButtonGroup.hpp"
#include "../tools/string_utils.hpp"
#include "../web/Animate.hpp"
#include "../web/Element.hpp"
#include "../web/Div.hpp"

namespace emp::prefab {

class ControlPanel;

/**
 * A RefreshChecker is a functor that accepts an Animate reference
 * and returns a boolean indicating whether components should be redrawn
 * based on an internal "rate" for redrawing.
 */
class MillisecondRefreshChecker {

  int refresh_rate{};
  int elapsed_milliseconds{};

  public:

  explicit MillisecondRefreshChecker(const int refresh_rate_=0)
  : refresh_rate(refresh_rate_)
  {}

  bool ShouldRefresh(const web::Animate & anim) {
    elapsed_milliseconds += anim.GetStepTime();
      if (elapsed_milliseconds > refresh_rate) {
        elapsed_milliseconds -= refresh_rate;
        if (elapsed_milliseconds > refresh_rate) elapsed_milliseconds = 0;
        return true;
      }
      return false;
  }

  std::string Describe() const {
    return emp::to_string("Every ", refresh_rate, " Milliseconds");
  }

};

/**
 * A RefreshChecker is a functor that accepts an Animate reference
 * and returns a boolean indicating whether components should be redrawn
 * based on an internal "rate" for redrawing.
 */
class FrameRefreshChecker {

  int refresh_rate{};

  public:

  explicit FrameRefreshChecker(const int refresh_rate_=0)
  : refresh_rate(refresh_rate_)
  {}

  bool ShouldRefresh(const web::Animate & anim) {
    return anim.GetFrameCount() % refresh_rate;
  }

  std::string Describe() const {
    return emp::to_string("Every ", refresh_rate, " Updates");
  }

};

/**
 * A RefreshChecker is a functor that accepts an Animate reference
 * and returns a boolean indicating whether components should be redrawn
 * based on an internal "rate" for redrawing.
 */
class DynamicRefreshChecker {

  std::function<bool(const web::Animate &)> checker;
  std::string description;

  public:

  explicit DynamicRefreshChecker(
    const std::function<bool(const web::Animate &)> checker_
      =[](const web::Animate &){ return true; },
    const std::string& description_="Custom Refresh"
  ) : checker(checker_)
  , description(description_)
  {}

  bool ShouldRefresh(const web::Animate & anim) {
    return checker(anim);
  }

  std::string Describe() const { return description; }

};

namespace internal {

  /**
   * Shared pointer held by instances of ControlPanel class representing
   * the same conceptual ControlPanel DOM object.
   * Contains state that should persist while ControlPanel DOM object
   * persists.
   */
  class ControlPanelInfo : public web::internal::DivInfo {
    friend ControlPanel;

    // ControlPanelInfo holds contains two specific instances of a
    // RefreshChecker for milliseconds and frames to keep independent rates
    // for both.
    emp::DisjointVariant<
      emp::prefab::MillisecondRefreshChecker,
      emp::prefab::FrameRefreshChecker,
      emp::prefab::DynamicRefreshChecker
    > checkers;

    // A list of widget that should be redrawn when do_redraw return true
    emp::vector<web::Widget> refresh_list;

    // A void callback function to run every frame (as fast as possible)
    std::function<void()> step_callback;

    /**
     * Construct a shared pointer to manage ControlPanel state.
     *
     * @param in_id HTML ID of ConfigPanel div
     */
    ControlPanelInfo(const std::string & in_id="")
    : DivInfo(in_id),
    step_callback([](){ ; })  { ; }

    /**
     * Get a constant reference to the redraw checker function
     * @return a redraw checker
     */
    bool ShouldRefresh(const emp::web::Animate& anim) {
      return checkers.Visit([&anim](auto&& active_checker){
        return active_checker.ShouldRefresh( anim );
      });
    }

    /**
     * Configure a checker but do not activate it.
     */
    template<typename Checker>
    void ConfigureRefreshChecker(Checker&& checker) {
      checkers.AssignToElement<Checker>( std::forward<Checker>(checker) );
    }

    /**
     * Specify which refresh checker to use.
     */
    template<typename T>
    void ActivateRefreshChecker() { checkers.Activate<T>(); }


    /**
     * Calls the void callback function that will be called every frame (as often as possible).
     */
    void CallStepCallback() const { step_callback(); }

    /**
     * Get the refresh list for this control panel
     * @return a list of Widgets that will be refreshed every update period
     */
    emp::vector<web::Widget> & GetRefreshList() {
      return refresh_list;
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
     * @param checker refresh checker to use
     * (i.e., emp::MillisecondRefreshChecker, emp::FrameRefreshChecker, or emp::DynamicRefreshChecker)
     * @param in_info info object associated with this component
     */
    template< typename RefreshChecker >
    ControlPanel(
      RefreshChecker&& checker,
      web::internal::DivInfo * in_info
    ) : web::Div(in_info),
    toggle_run{
      FontAwesomeIcon{"fa-play"}, FontAwesomeIcon{"fa-pause"},
      "success", "warning",
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
      SetRefreshChecker(std::forward<RefreshChecker>(checker) );
      step.AddAttr(
        "class", "btn",
        "class", "btn-success"
      );

      static_cast<Div>(*this) << button_line;
      button_line << toggle_run;
      button_line << step;

      AddAnimation(GetID(),
        [info=Info()](const web::Animate & anim) mutable {
          // Run the simulation function every frame
          info->CallStepCallback();
          // Redraw widgets according to a rule
          if (info->ShouldRefresh(anim)) {
            for (auto & widget : info->GetRefreshList()) {
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
     * @param refresh_checker object to control refresh rate
     * @param in_id HTML ID of control panel div
     */
    template<typename RefreshChecker>
    ControlPanel(
      RefreshChecker&& refresh_checker,
      const std::string & in_id=""
    ) : ControlPanel(
      std::forward<RefreshChecker>( refresh_checker ),
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
     * Set which refresh checker this panel will use.
     */
    template <typename Checker>
    void ActivateRefreshChecker() { Info()->ActivateRefreshChecker<Checker>(); }

    /**
     * Configure a particular refresh checker associated with this panel
     * but do not activate it.
     * @param checker configured checker to copy into panel.
     */
    template <typename Checker>
    void ConfigureRefreshChecker( Checker&& checker ) {
      Info()->ConfigureRefreshChecker<Checker>(
        std::forward<Checker>(checker)
      );
    }

    /**
     * Configure a particular refresh checker associated with this panel
     * and activate it.
     * @param checker configured checker to copy into panel.
     */
    template <typename Checker>
    void SetRefreshChecker( Checker&& checker ) {
      ConfigureRefreshChecker<Checker>( std::forward<Checker>(checker) );
      ActivateRefreshChecker<Checker>();
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
