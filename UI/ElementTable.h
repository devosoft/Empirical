#ifndef EMP_UI_ELEMENT_TABLE_H
#define EMP_UI_ELEMENT_TABLE_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Manage a section of the current web page
//

#include <functional>
#include <string>
#include <vector>

#include <emscripten.h>

#include "../tools/DynamicStringSet.h"

#include "Element.h"

namespace emp {
namespace UI {

  class ElementTable : public Element, public Table {
  private:
    void UpdateHTML() {
      HTML.str("");       // Clear the current HTML
      Table::WriteHTML(HTML);
    }
    void UpdateCSS() {
      Table::UpdateCSS();
    }

  public:
    ElementTable(const Table & in_table, Element * in_parent)
      : Element(in_table.GetDivID(), in_parent), Table(in_table) {
      ;
    }
    ~ElementTable() { ; }

    virtual bool IsTable() const override { return true; }
  };

};
};

#endif
