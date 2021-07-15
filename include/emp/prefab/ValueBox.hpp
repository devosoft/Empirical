#pragma once

#include "../datastructs/set_utils.hpp"
#include "../tools/string_utils.hpp"

#include "../config/config.hpp"
#include "../web/Div.hpp"
#include "../web/Element.hpp"
#include "../web/Input.hpp"

#include "Collapse.hpp"
#include "CommentBox.hpp"
#include "FontAwesomeIcon.hpp"
#include "ToggleSwitch.hpp"

namespace emp::prefab {

  const int min_slider_width = 100;

  class ValueBox : public web::Div {

    public:
    CommentBox description;
    Div view;

    ValueBox(
      const std::string & label,
      const std::string & desc,
      const std::string & id=""
    ) : Div(id) {
      web::Element drop_button("button");
      drop_button.AddAttr("class", "btn btn-link");

      FontAwesomeIcon arrow_right("fa-angle-double-right");
      arrow_right.AddAttr("class", "toggle_icon_right_margin");
      FontAwesomeIcon arrow_up("fa-angle-double-up");
      arrow_up.AddAttr("class", "toggle_icon_right_margin");
      drop_button << arrow_right;
      drop_button << arrow_up;
      drop_button << label;

      description.AddContent(desc);
      CollapseCoupling(drop_button, description, false);
      *this << drop_button;
      *this << description;
      *this << view;
      this->AddAttr("class", "value_box");
      view.AddAttr("class", "value_view");
      description.AddAttr("class", "value_description");
    }
  };

  class ValueDisplay : public ValueBox {

    public:
    ValueDisplay(
      const std::string & label,
      const std::string & value,
      const std::string & desc,
      const std::string & id=""
    ) : ValueBox(label, desc, id) {
      view << value;
    }
  };

  class ValueControl : public ValueBox {
    public:
    web::Input mainCtrl;
    ValueControl(
      const std::string & label,
      const std::string & desc,
      const std::string & inital_value,
      web::Input input,
      const std::string & id=""
    ) : ValueBox(label, desc, id), mainCtrl(input) {
      view << mainCtrl;
      mainCtrl.Value(inital_value);
    }
  };

  class TextValueControl : public ValueControl {
    public:
    TextValueControl(
      const std::string & label,
      const std::string & desc,
      const std::string & value,
      const std::function<void(const std::string & val)> & onChange = [](std::string val) { ; },
      const std::string & id=""
    ) : ValueControl(label, desc, value, web::Input(onChange, "text", ""), id) { ; }
  };

  class BoolValueControl : public ValueBox {
    public:
    BoolValueControl(
      const std::string & label,
      const std::string & desc,
      const bool is_checked,
      const std::function<void(const std::string & val)> & onChange = [](const std::string & val) { ; },
      const std::string & id=""
    ) : ValueBox(label, desc, id) {
      prefab::ToggleSwitch toggle(onChange, "", is_checked);
      view << toggle;
    }
  };

  class NumericValueControl : public ValueControl {
    public:
    NumericValueControl(
      const std::string & label,
      const std::string & desc,
      const std::string & value,
      const std::function<void(const std::string & val)> & onChange = [](const std::string & val) { ; },
      const std::string & id=""
    ) : ValueControl(label, desc, value, web::Input([](const std::string & val){ ; }, "range", ""), id) {
      web::Input temp(mainCtrl);
      web::Input number_box([slider=temp, onChange](const std::string & val) mutable {
        // Lambdas must be marked mutable since .Value is not a const function
        // Warning: referenced components/functions must be captured by value at
        // this lowest level or dangling references (and broken components) result!
        slider.Value(val);
      }, "number", "");
      mainCtrl.Callback([number_box, onChange](const std::string & val) mutable {
        onChange(val);
        number_box.Value(val);
      });
      number_box.Value(value);
      view << number_box;
    }
  };
}