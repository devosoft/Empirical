#ifndef EMP_UI_TABLE_H
#define EMP_UI_TABLE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Table widget
//
//  Tables have two helper widgets: TableRow and TableData.
//
//  A Table is composed of row x col cells
//  TableData is represented in its upper-left most cell, but may be muliple cells wide/tall.
//  Cells representing TableData are "Control Cells"; other cells are "Helper Cells".
//


#include <vector>

#include "UI_base.h"

namespace emp {
namespace UI {

  // Forward-declare the internal Table*_detail classes
  namespace internal { class TableData_detail; class TableRow_detail; class Table_detail; };
  
  // Specify the Table class for use in return values in the Table_detail definitions below.
  using TableData = internal::Widget_wrap<internal::TableData_detail>;
  using TableRow = internal::Widget_wrap<internal::TableRow_detail>;
  using Table = internal::Widget_wrap<internal::Table_detail, int, int>;

  namespace internal {
    class TableData_detail : public Widget_base {
    protected:
      int pos_id;   // Where is the Control Cell for this data field?
      int cols;     // How many columns wide is this TableData?
      int rows;     // How many rows deep is this TableData?
      bool header;  // Is this TableData a header (<th> vs <td>)?

      void WriteHTML(std::ostream & os) { ; }
    public:
      TableData_detail() {
        obj_ext = "__td";    // Make sure table row has own CSS.
      }
      ~TableData_detail() { ; }
    };
    
    class TableRow_detail : public Widget_base{
    protected:
      std::vector<TableData> data;  // detail object for each cell in this row.

      void WriteHTML(std::ostream & os) { ; }
    public:
      TableRow_detail() {
        obj_ext = "__tr";    // Make sure table row has own CSS.
      }
      ~TableRow_detail() { ; }

      TableRow & Cols(int c) { data.resize(c); return (TableRow &) *this; }
    };

    class Table_detail : public Widget_base {
    protected:
      int col_count;    // How big is this table?
      int row_count;
      
      std::vector<TableRow> rows;   // detail object for each row.
     
      // int GetCol(int id) const { return id % col_count; }
      // int GetRow(int id) const { return id / col_count; }
      // int GetID(int row, int col) { return row * col_count + col; }

      void WriteHTML(std::ostream & os) {
        // @CAO fill out...
      }

    public:
      Table_detail(int r, int c) : col_count(c), row_count(r), rows(r) {
        emp_assert(c > 0 && r > 0);           // Ensure that we have rows and columns!
        obj_ext = "__t";                      // Make sure table has own CSS.
        for (auto & row : rows) row.Cols(c);  // Set all rows to correct number of columns.
      }
      ~Table_detail() { ; }

      int GetNumCols() const { return col_count; }
      int GetNumRows() const { return row_count; }
      int GetNumCells() const { return col_count*row_count; }
      
      Table & Cols(int c) { col_count = c; return (Table &) *this; }
      Table & Rows(int r) { row_count = r; return (Table &) *this; }
    };

  };

};
};

#endif
