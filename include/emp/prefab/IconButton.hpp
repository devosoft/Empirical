/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 * @file include/emp/prefab/IconButton.hpp
 * @brief Accessible icon buttons using ordinary web::Button behavior.
 */
#pragma once

#ifndef INCLUDE_EMP_PREFAB_ICON_BUTTON_HPP_GUARD
#define INCLUDE_EMP_PREFAB_ICON_BUTTON_HPP_GUARD

#include "../tools/String.hpp"
#include "../web/Button.hpp"

namespace emp::prefab {
  class IconButton : public web::Button {
  public:
    /// icon is a plain-text symbol or an image URL when image is true. No stylesheet is required.
    IconButton(const std::function<void()> & callback, const std::string & icon,
               const std::string & accessible_name, const std::string & label = "",
               const std::string & id = "", const std::string & classes = "",
               bool image = false, const std::string & icon_classes = "")
      : Button(callback, "", id, classes) {
      emp_assert(!accessible_name.empty());
      const auto safe_icon = emp::MakeWebSafe(icon);
      const auto safe_class = emp::MakeWebSafe(icon_classes);
      SetLabel(image
        ? emp::to_string("<img alt=\"\" aria-hidden=\"true\" class=\"", safe_class,
                         "\" src=\"", safe_icon, "\">", emp::MakeWebSafe(label))
        : emp::to_string("<span aria-hidden=\"true\" class=\"", safe_class, "\">",
                         safe_icon, "</span>", emp::MakeWebSafe(label)));
      SetAttr("type", "button");
      SetAttr("aria-label", accessible_name);
      SetTitle(accessible_name);
    }
  };
}

#endif  // INCLUDE_EMP_PREFAB_ICON_BUTTON_HPP_GUARD
