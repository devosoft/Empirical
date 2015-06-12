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
      HTML << "<table id=\"" << div_id << obj_ext << "\">";

      // Loop through all of the rows in the table.
      for (auto & row : rows) {
        HTML << "<tr id=\"" << row.GetDivID() << row.GetObjExt() << "\">";

        // Loop through each cell in this row.
        for (auto & datum : row.GetCells()) {
          HTML << (datum.IsHeader() ? "<th id=" : "<td id=\"")
               << datum.GetDivID() << datum.GetObjExt() << "\">";
          if (datum.GetChildID() >= 0) {
            Element * element = children[datum.GetChildID()];
            const std::string & tag = element->GetWrapperTag();
            HTML << "<" << tag << " id=\"" << element->GetName() << "\"></" << tag << ">\n";
          }
          HTML << (datum.IsHeader() ? "</th>" : "</td>");
        }

        HTML << "</tr>";
      }

      HTML << "</table>";
    }
    void UpdateCSS() {
      Table::UpdateCSS();
    }

    Element & GetCurSlate() {
      // Determine if the current cell has a slate already.
      auto & cur_cell = GetCurCell();
      int child_id = cur_cell.GetChildID();

      // If not, generate one (and let the cell know).
      if (child_id < 0) {
        child_id = (int) children.size();
        std::string child_name = cur_cell.GetDivID() + "s";
        children.push_back( BuildElement(Slate(child_name), this) );
        cur_cell.SetChildID(child_id);
      }

      // Return the element, now that we know we have it.
      return *(children[child_id]);
    }

  public:
    ElementTable(const Table & in_table, Element * in_parent)
      : Element(in_table.GetDivID(), in_parent), Table(in_table) {
      ;
    }
    ~ElementTable() { ; }

    virtual bool IsTable() const override { return true; }

    // Mask some "Get" methods to facilitate appending (have them return ElementTable type)
    ElementTable & GetCell(int r, int c) {  Table::GetCell(r,c);  return (ElementTable &) *this; }
    ElementTable & GetRow(int r) { Table::GetRow(r);  return (ElementTable &) *this; }
    ElementTable & GetTable() { Table::GetTable(); return (ElementTable &) *this; }
    

    // When appending children to a table, forward to an internal slate.
    Element & Append(const std::string & in_text) { return GetCurSlate().Append(in_text); }
    Element & Append(const std::function<std::string()> & fun) { return GetCurSlate().Append(fun); }
    Element & Append(emp::UI::Button info) { return GetCurSlate().Append(info); }
    Element & Append(emp::UI::Image info) { return GetCurSlate().Append(info); }
    Element & Append(emp::UI::Table info) { return GetCurSlate().Append(info); }
    Element & Append(emp::UI::Text info) { return GetCurSlate().Append(info); }
    Element & Append(emp::UI::Slate info) { return GetCurSlate().Append(info); }

    Element & Append(const emp::UI::GetCell & cell) {
      this->GetCell(cell.row, cell.col);
      return *this;
    }

    Element & Append(const emp::UI::GetRow & row) {
      this->GetRow(row.row);
      return *this;
    }
  };

};
};

#endif
