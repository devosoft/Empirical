/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/EditableText.hpp
 * @brief Plain-text editing with explicit commit/cancel and configurable blur behavior.
 * 
 * Use example:
 *
 *   emp::web::EditableText name("Untitled", SaveName, "Edit name", "name");
 *   doc << name;
 *
 * Here doc is a web::Document or another container. SaveName receives committed
 * text as const std::string &; the third argument names the control accessibly.
 * Double-click, Enter, or F2 begins editing. Enter commits, Escape cancels, and
 * blur commits by default. Pass EditableText::Blur::Cancel after the optional
 * classes argument to cancel on blur instead. Unchanged text does not invoke the callback.
 * GetText() returns the last committed text; displayed content is plain text.
 * 
 * A common use case is displaying a filename that the user can double-click to rename.
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_EDITABLE_TEXT_HPP_GUARD
#define INCLUDE_EMP_WEB_EDITABLE_TEXT_HPP_GUARD

#include "../tools/String.hpp"

#include "Widget.hpp"

namespace emp::web {
  class EditableText : public internal::WidgetFacet<EditableText> {
  public:
    enum class Blur { Commit, Cancel };
    class EditableInfo : public internal::WidgetInfo {
    public:
      std::string text;
      Blur blur;
      std::function<void(const std::string &)> on_commit;
      JSCallback callback;
      EditableInfo(const std::string & id, std::string value,
                   std::function<void(const std::string &)> fn, Blur behavior)
        : WidgetInfo(id), text(std::move(value)), blur(behavior), on_commit(std::move(fn)),
          callback([this](std::string value) {
            if (value == text) { return; }
            text = std::move(value);
            // A commit may remove this widget.
            auto fn = on_commit;
            auto committed = text;
            if (fn) { fn(committed); }
          }) {}
      void GetHTML(std::stringstream & html) override {
        html << "<div id=\"" << id << "\">" << emp::MakeWebSafe(text) << "</div>";
      }
      void TriggerJS() override {
        MAIN_THREAD_EM_ASM({
          const node = document.getElementById(UTF8ToString($0));
          if (!node) return;
          let editing = false; let original = "";
          node.contentEditable = 'false';
          node.setAttribute('aria-readonly', 'true');
          const begin = () => {
            if (editing) return;
            editing = true; original = node.textContent;
            node.contentEditable = 'plaintext-only';
            node.setAttribute('aria-readonly', 'false'); node.focus();
            const range = document.createRange(); range.selectNodeContents(node);
            const selection = window.getSelection();
            selection.removeAllRanges(); selection.addRange(range);
          };
          const finish = commit => {
            if (!editing) return;
            editing = false;
            if (!commit) node.textContent = original;
            node.contentEditable = 'false'; node.setAttribute('aria-readonly', 'true');
            if (commit) emp.Callback($1, node.textContent);
          };
          node.ondblclick = event => { event.stopPropagation(); begin(); };
          node.onkeydown = event => {
            if (event.isComposing) return;
            if (event.key === 'Enter' || (!editing && event.key === 'F2')) {
              event.preventDefault(); if (editing) finish(true); else begin();
            } else if (event.key === 'Escape' && editing) {
              event.preventDefault(); finish(false);
            }
          };
          node.onblur = () => finish(!!$2);
        }, id.c_str(), callback.GetID(), blur == Blur::Commit);
      }
    };
    using INFO_TYPE = EditableInfo;
    EditableText(const std::string & text,
                 const std::function<void(const std::string &)> & on_commit,
                 const std::string & accessible_name, const std::string & id = "",
                 const std::string & classes = "", Blur blur = Blur::Commit)
      : WidgetFacet(id) {
      info = new EditableInfo(id, text, on_commit, blur);
      AddClass(classes);
      SetAttr("tabindex", "0"); SetAttr("role", "textbox");
      SetAttr("aria-label", accessible_name);
    }
    const std::string & GetText() const { return static_cast<EditableInfo *>(info)->text; }
  };
}

#endif  // INCLUDE_EMP_WEB_EDITABLE_TEXT_HPP_GUARD
