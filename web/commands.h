#ifndef EMP_WEB_COMMANDS_H
#define EMP_WEB_COMMANDS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This file defines a set of command-defining classes that can be fed into widgets
//  using the << operator.
//

namespace emp {
namespace web {

  class Close : public internal::WidgetCommand {
  private:
    std::string close_id;
  public:
    Close(const std::string & id) : close_id(id) { ; }

    bool Trigger(internal::WidgetInfo & w) const override {
      if (w.id == close_id) {  // Test if this is the element we need to close.
        w.append_ok = false;
        return true;
      }
      return false;
    }

    const std::string & GetID() const { return close_id; }
  };

}
}

#endif
