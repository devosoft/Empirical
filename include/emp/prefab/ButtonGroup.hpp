#ifndef EMP_BUTTON_GROUP_HPP
#define EMP_BUTTON_GROUP_HPP

#include "emp/web/Div.hpp"

namespace emp::prefab {
  /**
   * Use a ButtonGroup to place buttons of a similar role into the same
   * container or to save space by placing buttons without gaps between them.
   */
  class ButtonGroup : public web::Div {

    protected:
    /**
     * The protected contructor for a ButtonGroup.
     * @param info_ref shared pointer containing presistent state
     */
    ButtonGroup(web::internal::DivInfo * info_ref) : web::Div(info_ref) {
      SetAttr("class", "btn-group");
    }

    public:
    /**
     * Constructor for a ButtonGroup.
     * @param in_id HTML ID of ButtonGroup div
     */
    ButtonGroup(const std::string & in_id="")
    : ButtonGroup(new web::internal::DivInfo(in_id)) { ; }

    /**
     * A function useful for joining two button groups together into one unit.
     * Removes buttons from the ButtonGroup passed in and adds them to this
     * button group group.
     * @param btn_group a button group
     */
    ButtonGroup & TakeChildren(ButtonGroup & btn_group) {
      *this << btn_group.Children();
      btn_group.Clear();
      return (*this);
    }
  };
}

#endif
