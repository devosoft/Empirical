/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/Details.hpp
 * @brief Native disclosure widget; browsers provide keyboard and accessibility behavior.
 *
 * Use example:
 *
 *   emp::web::Details advanced("Advanced settings", "advanced");
 *   advanced << "Additional controls go here.";
 *   doc << advanced;
 *
 * Here doc is a web::Document or another container. The label becomes the native
 * summary, and streamed content appears inside the disclosure. Optional classes
 * and an initial open flag follow the ID. Use SetOpen() and IsOpen() to control
 * or inspect the disclosure, and GetSummary() to customize its summary element.
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_DETAILS_HPP_GUARD
#define INCLUDE_EMP_WEB_DETAILS_HPP_GUARD

#include "../tools/String.hpp"

#include "Element.hpp"

namespace emp::web {
  class Details : public Element {
    Element summary;
  public:
    Details(const std::string & label, const std::string & id = "",
            const std::string & classes = "", bool open = false)
      : Element("details", id, classes), summary("summary", GetID() + "_summary") {
      summary << emp::MakeWebSafe(label);
      *this << summary;
      SetOpen(open);
    }
    Element & GetSummary() { return summary; }
    Details & SetOpen(bool open = true) {
      if (open) { SetAttr("open", "open"); }
      else {
        Info()->extras.RemoveAttr("open");
        if (IsActive()) {
          MAIN_THREAD_EM_ASM({
            const node = document.getElementById(UTF8ToString($0));
            if (node) node.open = false;
          }, GetID().c_str());
        }
      }
      return *this;
    }
    bool IsOpen() const {
      if (!IsActive()) { return Info()->extras.HasAttr("open"); }
      return MAIN_THREAD_EM_ASM_INT({
        return !!document.getElementById(UTF8ToString($0))?.open;
      }, GetID().c_str());
    }
  };
}

#endif  // INCLUDE_EMP_WEB_DETAILS_HPP_GUARD
