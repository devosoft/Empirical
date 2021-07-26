#ifndef EMP_READOUT_PANEL_HPP
#define EMP_READOUT_PANEL_HPP

#include "../tools/string_utils.hpp"
#include "../web/Div.hpp"

#include "Card.hpp"
#include "ValueBox.hpp"

namespace emp::prefab {

  namespace internal {
    class ReadoutPanelInfo : public CardInfo {
      using string_getter_t = std::function<std::string()>;
      using check_pair_t = std::pair<string_getter_t, emp::web::Div>;
      emp::vector<check_pair_t> refreshPairs;
    public:
      ReadoutPanelInfo(const std::string & in_id="") : CardInfo(in_id) { ; }


    };
  }

  class ReadoutPanel : public Card {
  protected:
    Div data_holder{emp::to_string(GetID(), "_data")};

    ReadoutPanel(const std::string & group_name,
      const std::string & state,
      const bool & show_glyphs,
      internal::ReadoutPanelInfo * info_ref
    ) : Card(state, show_glyphs, info_ref) {
      static_cast<Card>(*this) << data_holder;
      AddHeaderContent(group_name);
      data_holder.AddAttr("class", "display_group");

      // Do other stuff here

    }
  public:
    ReadoutPanel(const std::string & group_name,
      const std::string & state="INIT_OPEN",
      const bool & show_glyphs=true,
      const std::string & id=""
    ) : ReadoutPanel(
      group_name,
      state,
      show_glyphs,
      new internal::ReadoutPanelInfo(id)
    ) { ; }

    using string_getter_t = std::function<std::string()>;

    ReadoutPanel & AddValue(
      const std::string & name,
      const std::string & desc,
      const string_getter_t & value_getter
    ) {
      const std::string vd_name(emp::to_string(GetID(), "_", name));
      LiveValueDisplay ldv(name, desc, value_getter, false, vd_name);
      data_holder << ldv;
      Div view(ldv.GetView());
      this->AddAnimation(name, [](){;}, view);
      this->Animate(name).Start();
      return (*this);
    }
  };
}

#endif