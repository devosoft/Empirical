/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 * @file include/emp/prefab/FormField.hpp
 * @brief A labeled control with optional help, units, and accessible feedback.
 */
#pragma once

#ifndef INCLUDE_EMP_PREFAB_FORM_FIELD_HPP_GUARD
#define INCLUDE_EMP_PREFAB_FORM_FIELD_HPP_GUARD

#include "../tools/String.hpp"
#include "../web/Element.hpp"
#include "../web/Input.hpp"

namespace emp::prefab {
  class FormField : public web::Div {
    web::Widget control;
    web::Element feedback;
  public:
    template <typename CONTROL>
    FormField(CONTROL input, const std::string & label, const std::string & help = "",
              const std::string & units = "", const std::string & id = "",
              const std::string & classes = "")
      : Div(id, classes), control(input), feedback("span", GetID() + "_feedback") {
      web::Element caption("label", GetID() + "_label");
      caption.SetAttr("for", input.GetID());
      caption << emp::MakeWebSafe(label);
      *this << caption;
      *this << input;
      std::string described = feedback.GetID();
      if (!units.empty()) {
        web::Element unit("span", GetID() + "_units");
        unit << emp::MakeWebSafe(units);
        described += " " + unit.GetID();
        *this << unit;
      }
      if (!help.empty()) {
        web::Element description("span", GetID() + "_help");
        description << emp::MakeWebSafe(help);
        described += " " + description.GetID();
        *this << description;
      }
      input.AddAttr("aria-describedby", described);
      feedback.SetAttr("role", "status");
      *this << feedback;
    }

    FormField & SetValidationError(const std::string & message, bool report = false) {
      feedback.ClearChildren();
      feedback << emp::MakeWebSafe(message);
      if (control.IsInput()) { web::Input(control).SetValidationError(message, report); }
      return *this;
    }
    FormField & ClearValidationError() { return SetValidationError(""); }
    web::Element & GetFeedback() { return feedback; }
  };
}

#endif  // INCLUDE_EMP_PREFAB_FORM_FIELD_HPP_GUARD
