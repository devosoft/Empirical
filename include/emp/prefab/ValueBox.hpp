/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file ValueBox.hpp
 *  @brief UI subcomponent for ConfigPanel and ReadoutPanel.
 */

#ifndef EMP_PREFAB_VALUEBOX_HPP_INCLUDE
#define EMP_PREFAB_VALUEBOX_HPP_INCLUDE

#include <cmath>

#include "../datastructs/set_utils.hpp"
#include "../tools/string_utils.hpp"

#include "../web/Div.hpp"
#include "../web/Element.hpp"
#include "../web/Input.hpp"

#include "Collapse.hpp"
#include "CommentBox.hpp"
#include "FontAwesomeIcon.hpp"
#include "ToggleSwitch.hpp"

namespace emp::prefab {

  /**
   * ValueBox is the base class for a component to show information.
   * At its core it has a label, a description, and a value to display.
   *
   * ValueBoxes and derived classes placed in component with the "settings_group"
   * or "display_group" class will align to a grid.
   */
  class ValueBox : public web::Div {

    protected:

    CommentBox description{emp::to_string(GetID(), "_descr")};
    Div view{emp::to_string(GetID(), "_view")};

    public:
    /**
     * @param label name for this value
     * @param desc a more detailed description of what the value means
     * @param id user defined ID for ValueBox Div (default is emscripten generated)
     */
    ValueBox(
      const std::string & label,
      const std::string & desc,
      const std::string & id=""
    ) : Div(id) {
      web::Element drop_button("button", emp::to_string(GetID(), "_label"));
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

    /**
     * Returns the view div containing live values to animate
     * (use this in an Animate instance).
     *
     * @return the view div
     */
    Div & GetView() {
      return view;
    }
  };

  /**
   * Use a ValueDisplay component to display a labeled, static value with
   * a nice description of what this value means.
   *
   * We suggest adding the "display_group" class to the enclosing tag around
   * multiple ValueDisplays to align labels and values along a common grid.
   */
  class ValueDisplay : public ValueBox {

    public:
    /**
     * @param label name for this value
     * @param desc a more detailed description of what the value means
     * @param value the piece of information or data being displayed
     * @param id user defined ID for ValueDisplay div (default is emscripten generated)
     */
    ValueDisplay(
      const std::string & label,
      const std::string & desc,
      const std::string & value,
      const std::string & id=""
    ) : ValueBox(label, desc, id) {
      view << value;
    }
  };

  /**
   * Use a LiveValueDisplay component to display a labeled value which will
   * change over the course of a simulation with a nice description of what
   * this value means. Internally, it manages an Animator to redraw the
   * view at the fastest possible framerate unless independent=false.
   *
   * We suggest adding the "display_group" class to the enclosing tag around
   * multiple LiveValueDisplays to align labels and values along a common grid.
   */
  class LiveValueDisplay : public ValueBox {

    public:
    /**
     * @param label name for this value
     * @param desc a more detailed description of what the value means
     * @param value a value to display or function returning a string
     * @param independent should this component be independently animated? Or will
     * some other component manage its animation?
     * @param id user defined ID for LiveValueDisplay div (default is emscripten generated)
     */
    template<typename T>
    LiveValueDisplay(
      const std::string & label,
      const std::string & desc,
      T && value,
      const bool & independent=true,
      const std::string & id=""
    ) : ValueBox(label, desc, id) {
      view << web::Live(std::forward<T>(value));
      if (independent) {
        this->AddAnimation(GetID(), [](){;}, view);
        this->Animate(GetID()).Start();
      }
    }
  };

  /**
   * ValueControl is the base class for components the user should be
   * able to interact with. The view will contain the Input object for
   * controlling the value.
   */
  class ValueControl : public ValueBox {
    protected:
    web::Input mainCtrl;
    public:
    /**
     * @param label name for this value
     * @param desc a more detailed description of what the value means
     * @param initial_value the initial value
     * @param input Input component that user can interact with
     * @param id user defined ID for ValueControl div (default is emscripten generated)
     */
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

  /**
   * Use a TextValueControl to display a boolean value with a label,
   * description, and a text input to manipulate the value.
   */
  class TextValueControl : public ValueControl {
    public:
    /**
     * @param label name for this value
     * @param desc a more detailed description of what the value means
     * @param value the initial value
     * @param onChange function to be called when the user changes this value
     * @param id user defined ID for TextValueControl div (default is emscripten generated)
     */
    TextValueControl(
      const std::string & label,
      const std::string & desc,
      const std::string & value,
      const std::function<void(const std::string &)> & onChange = [](const std::string &) { ; },
      const std::string & id=""
    ) : ValueControl(label, desc, value, web::Input(onChange, "text", ""), id) {
      mainCtrl.AddAttr("class", "form-control");
    }
  };

  /**
   * Use a BoolValueControl to display a boolean value with a label,
   * description, and a switch to toggle the value.
   */
  class BoolValueControl : public ValueBox {
    public:
    /**
     * @param label name for this value
     * @param desc a more detailed description of what the value means
     * @param value the initial value
     * @param onChange function to be called when the user changes this value
     * @param id user defined ID for BoolValueControl div (default is emscripten generated)
     */
    BoolValueControl(
      const std::string & label,
      const std::string & desc,
      const bool is_checked,
      const std::function<void(const std::string &)> & onChange = [](const std::string &) { ; },
      const std::string & id=""
    ) : ValueBox(label, desc, id) {
      prefab::ToggleSwitch toggle(onChange, "", is_checked);
      view << toggle;
    }
  };

  /**
   * Use a NumericValueControl to display a number with a label,
   * description, synchronized slider and number box to change the
   * value.
   */
  class NumericValueControl : public ValueControl {

    using range_setter_t = std::function<void(
      const std::string &, // value
      const std::string &, // type
      emp::web::Input &
    )>;

    // Determine the default range by finding the next highest order of magnitude (base 10)
    inline static range_setter_t applyDefaultRange = [](
      const std::string & value,
      const std::string & type,
      emp::web::Input & in
    ) {
      if(type == "float" || type == "double") {
        const double val = emp::from_string<double>(value);
        const int max = (abs(val) <= 1) ? 1 : static_cast<int>(pow(10, ceil(log10(abs(val)))));
        const int min = (val >= 0) ? 0 : -max;
        const double step = max/100.0;
        in.Max(max);
        in.Min(min);
        in.Step(step);
      } else {
        const int val = emp::from_string<int>(value);
        const int max = (abs(val) <= 10) ? 10 : static_cast<int>(pow(10, ceil(log10(abs(val)))));
        const int min = (val >= 0) ? 0 : -max;
        const int step = (int)fmax(max/100.0, 1);
        in.Max(max);
        in.Min(min);
        in.Step(step);
      }
    };

    public:
    /**
     * @param label name for this value
     * @param desc a more detailed description of what the value means
     * @param type the numeric type ('float', 'double' or 'int')
     * @param value the initial value
     * @param onChange function to be called when the user changes this value
     * @param id user defined ID for NumericValueControl div (default is emscripten generated)
     */
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
        // Note: referenced components/functions must be captured by value at
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

    /**
     * @param det a function that based on the value and type provided sets
     * a slider input's min, max and step values appropriately.
     */
    static void setDefaultRangeMaker(const range_setter_t & det) {
      applyDefaultRange = det;
    }
  };
}

#endif // #ifndef EMP_PREFAB_VALUEBOX_HPP_INCLUDE
