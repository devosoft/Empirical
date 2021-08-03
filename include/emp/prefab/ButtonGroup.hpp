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
     * Plus operator joins two button groups into one. Useful for joining
     * multiple default constructed groups together if it makes sense.
     * LHS ButtonGroup takes RHS ButtonGroup's children.
     *
     * @param btn_group a button group
     */
    ButtonGroup & operator+(const ButtonGroup & rhs) {
      *this << rhs.Children();
      return (*this);
    }
  };
}

#endif
