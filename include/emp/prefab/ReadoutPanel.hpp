#ifndef EMP_READOUT_PANEL_HPP
#define EMP_READOUT_PANEL_HPP

#include "../tools/string_utils.hpp"
#include "../web/Div.hpp"

#include "Card.hpp"
#include "ValueBox.hpp"

namespace emp::prefab {

  namespace internal {
    class ReadoutPanelInfo : public CardInfo {
      emp::vector<emp::web::Div> live_divs;
    public:
      ReadoutPanelInfo(const std::string & in_id="") : CardInfo(in_id) { ; }

      void AddLiveDiv(emp::web::Div & holder) {
        live_divs.push_back(holder);
      }

      emp::vector<emp::web::Div> & GetLiveDivs() {
        return live_divs;
      }

    };
  }

  class ReadoutPanel : public Card {
  private:
    /**
     * Get shared info pointer, cast to ReadoutPanel-specific type.
     * Necessary to access info's liveHolder's list of divs.
     *
     * @return cast pointer
     */
    internal::ReadoutPanelInfo * Info() {
      return dynamic_cast<internal::ReadoutPanelInfo *>(info);
    }

    /**
     * Get shared info pointer, cast to const ReadoutPanel-specific type.
     * Necessary to access info's liveHolder's list of divs.
     *
     * @return cast pointer
     */
    const internal::ReadoutPanelInfo * Info() const {
      return dynamic_cast<internal::ReadoutPanelInfo *>(info);
    }

  protected:
    // The div to hold the
    Div data_collection{emp::to_string(GetID(), "_data")};

    ReadoutPanel(const std::string & group_name,
      double refresh_rate,
      const std::string & state,
      const bool & show_glyphs,
      internal::ReadoutPanelInfo * info_ref
    ) : Card(state, show_glyphs, info_ref) {

      // Best to cast to Card in case we decide to overload the stream
      // operator to do something special for this class
      static_cast<Card>(*this) << data_collection;
      AddHeaderContent(group_name);
      data_collection.AddAttr("class", "display_group");

      auto & live_divs = Info()->GetLiveDivs();
      // Animation is referenced by this component's ID
      AddAnimation(GetID(), [
        elapsed = 0, refresh_rate, &live_divs
      ](double stepTime) mutable {
        // Accumulate steps, then redraw after enough time has elapsed
        elapsed += stepTime;
        if (elapsed > refresh_rate) {
          elapsed -= refresh_rate;
          for(emp::web::Div & div : live_divs) {
            div.Redraw();
          }
        }
        // If we are running a full 2x slower than the refresh rate
        // we'll have to skip frames to prevent overflow
        if (elapsed > refresh_rate) {
          elapsed = 0;
        }
      });
      Animate(GetID()).Start(); // Start the animation
    }

  public:
    ReadoutPanel(const std::string & group_name,
      double refresh_rate=250,
      const std::string & state="INIT_OPEN",
      const bool & show_glyphs=true,
      const std::string & id=""
    ) : ReadoutPanel(
      group_name,
      refresh_rate,
      state,
      show_glyphs,
      new internal::ReadoutPanelInfo(id)
    ) { ; }

    using string_getter_t = std::function<std::string()>;
    /**
     * Adds a LiveValueDisplay to this component and adds the value's
     * parent div to a list of divs to be redrawn at the refresh rate.
     * @param name the name for this value.
     * @param desc a description for this value.
     * @param value_getter a function that will return the string for this value.
     *
     * @return the readout panel for chaining calls
     */
    ReadoutPanel & AddValue(
      const std::string & name,
      const std::string & desc,
      const string_getter_t & value_getter
    ) {
      const std::string vd_name(emp::to_string(GetID(), "_", name));
      LiveValueDisplay ldv(name, desc, value_getter, false, vd_name);
      data_collection << ldv;
      Div view(ldv.GetView());
      Info()->AddLiveDiv(view);
      return (*this);
    }
  };
}

#endif