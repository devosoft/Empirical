#ifndef EMP_VALUE_BOX_HPP
#define EMP_VALUE_BOX_HPP

#include <cmath>

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
    ) : ValueControl(label, desc, value, web::Input(onChange, "text", ""), id) {
      mainCtrl.AddAttr("class", "form-control");
    }
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

    // Determine the default range by finding the next highest order of magnitude (base 10)
    inline static std::function<void(
      const std::string &,
      const std::string &,
      emp::web::Input &
    )> applyDefaultRange = [](
      const std::string & value,
      const std::string & type,
      emp::web::Input & in
    ) {
      if(type == "float" || type == "double") {
        double val = emp::from_string<double>(value);
        int max = (abs(val) <= 1) ? 1 : (int)pow(10, ceil(log10(abs(val))));
        int min = (val >= 0) ? 0 : -max;
        double step = max/100.0;
        in.Max(max);
        in.Min(min);
        in.Step(step);
      } else {
        int val = emp::from_string<int>(value);
        int max = (abs(val) <= 10) ? 10 : (int)pow(10, ceil(log10(abs(val))));
        int min = (val >= 0) ? 0 : -max;
        int step = (int)fmax(max/100.0, 1);
        in.Max(max);
        in.Min(min);
        in.Step(step);
      }
    };

    public:
    NumericValueControl(
      const std::string & label,
      const std::string & desc,
      const std::string & value,
      const std::string & type,
      const std::function<void(const std::string & val)> & onChange = [](const std::string & val) { ; },
      const std::string & id=""
    ) : ValueControl(label, desc, value, web::Input([](const std::string & val){ ; }, "range", ""), id) {
      mainCtrl.AddAttr("class", "form-range");
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
      number_box.AddAttr("class", "form-control");
      view << number_box;
      NumericValueControl::applyDefaultRange(value, type, mainCtrl);
      NumericValueControl::applyDefaultRange(value, type, number_box);
    }

    static void setDefaultRangeMaker(std::function<void(
      const std::string & value,
      const std::string & type,
      emp::web::Input & in)> det
    ) {
      applyDefaultRange = det;
    }
  };
}

#endif
