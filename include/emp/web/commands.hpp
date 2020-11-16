/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  commands.hpp
 *  @brief A set of command-defining classes that can be fed into widgets using the << operator.
 */

#ifndef EMP_WEB_COMMANDS_H
#define EMP_WEB_COMMANDS_H

#include "Widget.hpp"

namespace emp {
namespace web {

  /// The Close command will prevent the specified widget from accepting further appends
  /// (and instead revert to trying parents)
  class Close : public internal::WidgetCommand {
  private:
    std::string close_id;
  public:
    Close(const std::string & id) : close_id(id) { ; }

    bool Trigger(internal::WidgetInfo & w) const override {
      if (w.id == close_id) {  // Test if this is the element we need to close.
        Widget(&w).PreventAppend();
        return true;
      }
      return false;
    }

    const std::string & GetID() const { return close_id; }
  };

  /// The PrintStr command will print a pre-specified string to a widget.
  class PrintStr : public internal::WidgetCommand {
  private:
    std::string str;
  public:
    PrintStr(const std::string & _s) : str(_s) { ; }

    bool Trigger(internal::WidgetInfo & w) const override {
      w.Append(str);
      return true;
    }

    const std::string & GetStr() const { return str; }
  };

  /// Pre-define emp::endl to insert a "<br>" and thus acting like a newline.
  static const PrintStr endl("<br>");

}
}

#endif
