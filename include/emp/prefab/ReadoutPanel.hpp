/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file ReadoutPanel.hpp
 *  @brief UI framework for live statistic readouts.
 */

#ifndef EMP_PREFAB_READOUTPANEL_HPP_INCLUDE
#define EMP_PREFAB_READOUTPANEL_HPP_INCLUDE

#include "../tools/string_utils.hpp"
#include "../web/Animate.hpp"
#include "../web/Div.hpp"

#include "Card.hpp"
#include "ValueBox.hpp"

namespace emp::prefab {

  namespace internal {
    /**
     * Shared pointer held by instances of ReadoutPanel class representing
     * the same conceptual ReadoutPanel DOM object.
     * Contains state that should persist while ReadoutPanel DOM object
     * persists.
     */
    class ReadoutPanelInfo : public CardInfo {
      // Holds the divs in the panel that are to be
      // redrawn since they contain live objects.
      emp::vector<emp::web::Div> live_divs;
    public:
      /**
       * Construct a shared pointer to manage ReadoutPanel state.
       * @param in_id HTML ID of ReadoutPanel div
       */
      ReadoutPanelInfo(const std::string & in_id="") : CardInfo(in_id) { ; }

      /**
       * Add a div to the list of divs redrawn every refresh period.
       * @param liv a div (probably containing some Live value)
       */
      void AddLiveDiv(emp::web::Div & liv) {
        live_divs.push_back(liv);
      }

      /**
       * @return a vector of divs redrawn every refresh period.
       */
      emp::vector<emp::web::Div> & GetLiveDivs() {
        return live_divs;
      }

    };
  }
  /**
   * Use a ReadoutPanel to display a collection of related live values
   * in a Card.
   */
  class ReadoutPanel : public Card {
  private:
    /**
     * Get shared info pointer, cast to ReadoutPanel-specific type.
     * Necessary to access info's list of live divs.
     *
     * @return cast pointer
     */
    internal::ReadoutPanelInfo * Info() {
      return dynamic_cast<internal::ReadoutPanelInfo *>(info);
    }

    /**
     * Get shared info pointer, cast to const ReadoutPanel-specific type.
     * Necessary to access info's list of live divs.
     *
     * @return cast pointer
     */
    const internal::ReadoutPanelInfo * Info() const {
      return dynamic_cast<internal::ReadoutPanelInfo *>(info);
    }
  public:
    /**
     * @param group_name name for this collection of values, displayed in card header
     * @param refresh_time the time in milliseconds between refreshes to the live values
     * @param state initial state of the card, one of STAITC, INIT_OPEN, or INIT_CLOSED
     * @param show_glyphs whether the underlying card should show toggle icons in card header
     * @param id a user defined ID for ReadoutPanel div (default is emscripten generated)
     */
    ReadoutPanel(const std::string & group_name,
      const double refresh_milliseconds=250,
      const std::string & state="INIT_OPEN",
      const bool & show_glyphs=true,
      const std::string & id=""
    ) : ReadoutPanel(
      group_name,
      refresh_milliseconds,
      state,
      show_glyphs,
      new internal::ReadoutPanelInfo(id)
    ) { ; }

  protected:
    // The div to hold the LiveValueDisplays for when content is added.
    Div values{emp::to_string(GetID(), "_values")};

    /**
     * A protected constructor for a ReadoutPanel which sets up the necessary
     * structures for it to function. All other constructors should delegate
     * to this one first before causing subsequent modifications.
     *
     * @param group_name name for this collection of values, displayed in card
     * header
     * @param refresh_milliseconds the minimum time in milliseconds between
     * refreshes to the live values
     * @param state initial state of the card, one of STAITC, INIT_OPEN, or
     * INIT_CLOSED
     * @param show_glyphs whether the underlying card should show toggle icons
     * in card header
     * @param info_ref a pointer to the underlying ReadoutPanelInfo object for
     * this ReadoutPanel or a pointer to a derived info object (simulating inheritance)
     */
    ReadoutPanel(const std::string & group_name,
      double refresh_milliseconds,
      const std::string & state,
      const bool & show_glyphs,
      internal::ReadoutPanelInfo * in_info
    ) : Card(state, show_glyphs, in_info) {

      // Best to cast to Card in case we decide to overload the stream
      // operator to do something special for this class later
      static_cast<Card>(*this) << values;
      AddHeaderContent(group_name);
      values.AddAttr("class", "display_group");

      auto & live_divs = Info()->GetLiveDivs();
      // Animation is referenced by this component's ID
      AddAnimation(GetID(), [
        elapsed_milliseconds = 0, refresh_milliseconds, &live_divs
      ](double stepTime) mutable {
        // Accumulate steps, then redraw after enough time has elapsed
        elapsed_milliseconds += stepTime;
        if (elapsed_milliseconds > refresh_milliseconds) {
          elapsed_milliseconds -= refresh_milliseconds;
          for(emp::web::Div & div : live_divs) {
            div.Redraw();
          }
        }
        // Might not be necessary, but if elapsed time got to 2x
        // the refresh period, redraws are being severly delayed by
        // something. Setting to zero in this case has the effect of
        // dropping frames to prevent choking execution with redraws.
        if (elapsed_milliseconds > refresh_milliseconds) {
          elapsed_milliseconds = 0;
        }
      });

      auto * main = & this->Animate(GetID());
      main->Start(); // Start the animation
      if (state != "STATIC") {
        // TODO: make this double-click safe
        SetOnToggle([main](){
          main->ToggleActive();
        });
      }

    }

    /// A helper function to formate IDs generated for subcomponents
    inline static std::string FormatName(const std::string & name) {
      return to_lower(join(slice(name, ' '), "_"));
    }

  public:

    /**
     * Adds a LiveValueDisplay to this component and adds the value's
     * parent div to a list of divs to be redrawn at the refresh rate.
     * @param name the name for this value.
     * @param desc a description for this value.
     * @param value_getter a function that will return the string for this value.
     *
     * @return the readout panel for chaining calls
     */
    template<typename VALUE_TYPE>
    ReadoutPanel & AddValue(
      const std::string & name,
      const std::string & desc,
      VALUE_TYPE && value
    ) {
      const std::string vd_name(emp::to_string(GetID(), "_", FormatName(name)));
      LiveValueDisplay lvd(name, desc, std::forward<VALUE_TYPE>(value), false, vd_name);
      values << lvd;
      emp::web::Div view(lvd.GetView());
      Info()->AddLiveDiv(view);
      return (*this);
    }
    /**
     * A version of AddValue that can take multiple name, description, value tuples
     * for ease of use when adding lots of values.
     */
    template<typename VALUE_TYPE, typename... OTHER_VALUES>
    ReadoutPanel & AddValues(
      const std::string & name,
      const std::string & desc,
      VALUE_TYPE && value,
      OTHER_VALUES && ... others
    ) {
      AddValue(name, desc, std::forward<VALUE_TYPE>(value));
      return AddValues(std::forward<OTHER_VALUES>(others)...);
    }
    ReadoutPanel & AddValues() {
      return (*this);
    }


  };
}

#endif // #ifndef EMP_PREFAB_READOUTPANEL_HPP_INCLUDE
