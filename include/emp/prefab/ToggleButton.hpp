#ifndef EMP_TOGGLE_BUTTON_HPP
#define EMP_TOGGLE_BUTTON_HPP

namespace emp::prefab {

  namespace internal {

    class ToggleButtonInfo : public DivInfo {
      bool active;
      public:
      ToggleButtonInfo(std::string & in_id) : DivInfo(in_id) {;}
    }
  }

  class ToggleButton : public ButtonGroup {

    protected:
    ToggleButton(DivInfo * info_ref) : Div(info_ref) {

    }

    public:
    ToggleButton(std::string & in_id) : Div(new DivInfo(in_id)) { ; }

    operator bool() const

    }
  };


}

#endif
