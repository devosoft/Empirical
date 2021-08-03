#ifndef EMP_BUTTON_GROUP_HPP
#define EMP_BUTTON_GROUP_HPP

namespace emp::prefab {

  class ButtonGroup : public web::Div {

    protected:
    ButtonGroup(web::internal::DivInfo * info_ref) : web::Div(info_ref) {
      SetAttr("class", "btn-group");
    }

    public:
    ButtonGroup(const std::string & in_id="")
    : ButtonGroup(new web::internal::DivInfo(in_id)) { ; }

    /**
     * A function useful for joining two button groups together into one unit.
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
