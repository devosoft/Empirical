/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file  ButtonGroup.hpp
 *  @brief ButtonGroups add styling to compactly display a group of buttons and
 *  provides methods useful for moving buttons between groups.
 */

#ifndef EMP_BUTTON_GROUP_HPP
#define EMP_BUTTON_GROUP_HPP

#include "emp/web/Div.hpp"

namespace emp::prefab {
  /**
   * A ButtonGroup is a container with styling specifically to display buttons.
   * It also provides methods for moving buttons from one group into another
   * allowing the user to combine groups.
   *
   * Use a ButtonGroup to place buttons of a similar role into the same
   * container or to save space by placing buttons without gaps between them.
   */
  class ButtonGroup : public web::Div {

    protected:
    /**
     * A protected contructor for a ButtonGroup for internal use only. See the
     * prefab/README.md for more information on this design pattern.
     *
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
     * Removes buttons from the ButtonGroup passed in and appends them in order
     * to this button group group.
     *
     * @param btn_group a button group
     */
    ButtonGroup & TakeChildren(ButtonGroup && btn_group) {
      *this << btn_group.Children();
      btn_group.Clear();
      return (*this);
    }
  };
} // namespace emp::prefab

#endif // #ifndef EMP_BUTTON_GROUP_HPP
