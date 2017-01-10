//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines a set of command-defining classes that can be fed into widgets
//  using the << operator.
//
//  Currently Close is the only command available; it will prevent the specified
//  DOM id from being added to further.

#ifndef EMP_WEB_COMMANDS_H
#define EMP_WEB_COMMANDS_H

namespace emp {
namespace web {

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

}
}

#endif
