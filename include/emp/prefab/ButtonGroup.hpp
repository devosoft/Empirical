#ifndef EMP_BUTTON_GROUP_HPP
#define EMP_BUTTON_GROUP_HPP

namespace emp::prefab {

  class ButtonGroup : public emp::web::Div {

    public:
    ButtonGroup(const std::string & in_id="") : Div(in_id) {
      SetAttr("class", "btn-group");
    }

  };

  // /**
  //  * Overridden stream operator causes a button group's contents to be
  //  * appended to existing one rather than having nested button groups. Other
  //  * streamed components should be nested like usual.
  //  * @param btn_group a button group
  //  */
  // template<> ButtonGroup & ButtonGroup::operator<<(ButtonGroup && btn_group) {
  //     static_cast<Div>(*this) << btn_group.Children();
  //     return (*this);
  // }
}



#endif
