/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/DOMState.hpp
 * @brief Opt-in DOM state capture and keyed content patching, using stable element IDs.
 *
 * Use example:
 *
 *   emp::web::DOMState saved;
 *   saved.Capture("panel");
 *   RebuildPanel();
 *   saved.Restore("panel");
 *
 * Keep element IDs stable across rebuilds. DOMStatePolicy selects which focus,
 * scroll, disclosure, resize, and form states to preserve; form values are opt-in.
 * Resize preservation defaults to elements marked data-emp-preserve-size.
 * PatchHTML(root_id, trusted_html, policy) patches the contents of a root while
 * preserving matching keyed nodes and selected state. Escape user text before
 * passing HTML; this operation updates the DOM, not a Widget's child collection.
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_DOMSTATE_HPP_GUARD
#define INCLUDE_EMP_WEB_DOMSTATE_HPP_GUARD

#include "init.hpp"

namespace emp::web {
  struct DOMStatePolicy {
    bool focus = true;
    bool scroll = true;
    bool disclosures = true;
    bool resized = true;
    bool form_values = false;
    std::string selector = "[id]";
    std::string resize_selector = "[data-emp-preserve-size]";
    int Flags() const {
      return focus | (scroll << 1) | (disclosures << 2) | (resized << 3) | (form_values << 4);
    }
  };

  class DOMState {
    size_t token = 0;
  public:
    DOMState() = default;
    DOMState(const DOMState &) = delete;
    DOMState & operator=(const DOMState &) = delete;
    ~DOMState() { Clear(); }
    void Clear() {
      MAIN_THREAD_EM_ASM({ emp_i.domStates?.delete($0); }, token);
      token = 0;
    }
    bool Capture(const std::string & root_id, const DOMStatePolicy & policy = {}) {
      Clear();
      token = MAIN_THREAD_EM_ASM_INT({
        const root = document.getElementById(UTF8ToString($0));
        if (!root) return 0;
        const states = emp_i.domStates || (emp_i.domStates = new Map());
        const token = emp_i.nextDomState = (emp_i.nextDomState || 0) + 1;
        states.set(token, emp.CaptureDOMState(root, $1, UTF8ToString($2), UTF8ToString($3)));
        return token;
      }, root_id.c_str(), policy.Flags(), policy.selector.c_str(), policy.resize_selector.c_str());
      return token != 0;
    }
    void Restore(const std::string & root_id) const {
      MAIN_THREAD_EM_ASM({
        emp.RestoreDOMState(document.getElementById(UTF8ToString($0)), emp_i.domStates?.get($1));
      }, root_id.c_str(), token);
    }
  };

  /// Patch trusted HTML content inside root; never supply unescaped user text as HTML.
  /// This is an explicit DOM operation, not a mutation of a Widget's C++ child collection.
  inline bool PatchHTML(const std::string & root_id, const std::string & html,
                        const DOMStatePolicy & policy = {}) {
    return MAIN_THREAD_EM_ASM_INT({
      return emp.PatchHTML(document.getElementById(UTF8ToString($0)), UTF8ToString($1),
        $2, UTF8ToString($3), UTF8ToString($4));
    }, root_id.c_str(), html.c_str(), policy.Flags(), policy.selector.c_str(),
       policy.resize_selector.c_str());
  }
}

#endif  // INCLUDE_EMP_WEB_DOMSTATE_HPP_GUARD
