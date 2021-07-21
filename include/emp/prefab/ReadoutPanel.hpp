#ifndef EMP_READOUT_PANEL_HPP
#define EMP_READOUT_PANEL_HPP

#include "../tools/string_utils.hpp"
#include "../web/Div.hpp"

#include "Card.hpp"
#include "ValueBox.hpp"

namespace emp::prefab {
  class ReadoutPanel : public Card {
    protected:
    Div data_holder{emp::to_string(GetID(), "_data")};

    public:
    ReadoutPanel(const std::string & group_name,
      const std::string & state="INIT_OPEN",
      const bool & show_glyphs=true,
      const std::string & id=""
    ) : Card(state, show_glyphs, id) {
      static_cast<Card>(*this) << data_holder;
      AddHeaderContent(group_name);
      data_holder.AddAttr("class", "display_group");
    }

    template<typename IN_TYPE>
    ReadoutPanel & AddValue(
      const std::string & name,
      IN_TYPE && value,
      const std::string & desc
    ) {
      const std::string vd_name(emp::to_string(GetID(), "_", name));
      data_holder << LiveValueDisplay(name, std::forward<IN_TYPE>(value), desc, vd_name);
      return (*this);
    }

    /**
     * Add a LiveValueDisplay to the data holder portion of this readout
     *
     * @param in_val a LiveValueDisplay to add to the readout
     */
    ReadoutPanel & operator<<(LiveValueDisplay && in_val) {
      data_holder << std::forward<LiveValueDisplay>(in_val);
      return (*this);
    }
  };
}

#endif