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
      friend TableRow_detail;
    protected:
      int pos_id;   // Where is the Control Cell for this data field?
      int cols;     // How many columns wide is this TableData?
      int rows;     // How many rows deep is this TableData?
      bool header;  // Is this TableData a header (<th> vs <td>)?

      void WriteHTML(std::ostream & os) {
        os << (header ? "<th id=" : "<td id=\"")
           << div_id << obj_ext << "\">&nbsp;"
           << (header ? "</th>" : "</td>");
      }
      void UpdateCSS() {
        std::string obj_id = div_id + obj_ext;
        css_info.Apply(obj_id);
      }
    public:
      TableData_detail() {
        obj_ext = "td";    // Make sure table row has own CSS.
      }
      ~TableData_detail() { ; }
    };
    
    class TableRow_detail : public Widget_base{
      friend Table_detail;
    protected:
      std::vector<TableData> data;  // detail object for each cell in this row.

      void WriteHTML(std::ostream & os) {
        os << "<tr id=\"" << div_id << obj_ext << "\">";
        for (auto & datum : data) datum.WriteHTML(os);
        os << "</tr>";
      }
      void UpdateCSS() {
        std::string obj_id = div_id + obj_ext;
        css_info.Apply(obj_id);
        for (auto & datum : data) datum.UpdateCSS();
      }
    public:
      TableRow_detail() {
        obj_ext = "tr";    // Make sure table row has own CSS.
      }
      ~TableRow_detail() { ; }

      TableRow & Cols(int c) { data.resize(c); return (TableRow &) *this; }

      // Apply to all cells in row.  (@CAO: Can we use fancier jquery here?)
      template <typename SETTING_TYPE>
      TableRow & CellsCSS(const std::string & setting, SETTING_TYPE && value) {
        for (auto & datum : data) datum.CSS(setting, value);
        return (TableRow &) *this;
      }
        
      // Apply to specific cell in row.
      template <typename SETTING_TYPE>
      TableRow & CellCSS(int col_id, const std::string & setting, SETTING_TYPE && value) {
        data[col_id].CSS(setting, value);
        return (TableRow &) *this;
      }

      // NOTE: Regular CSS applied to a TableRow will modify row's own CSS.
    };

    class Table_detail : public Widget_base {
    protected:
      int row_count;    // How big is this table?
      int col_count;

      int cur_row;      // Which row/col is currently active?
      int cur_col;
      
      std::vector<TableRow> rows;   // detail object for each row.
     
      // A table's state determines how some operations work.
      enum state_t { TABLE, ROW, CELL };
      state_t state;

      void WriteHTML(std::ostream & os) {
        os << "<table id=\"" << div_id << obj_ext << "\">";
        for (auto & row : rows) row.WriteHTML(os);
        os << "</table>";
      }
      void UpdateCSS() {
        std::string obj_id = div_id + obj_ext;
        css_info.Apply(obj_id);
        for (auto & row : rows) row.UpdateCSS();
      }

      static bool OverrideCSS() { return true; } // Let tables control their own CSS function.
    public:
      Table_detail(int r, int c) : row_count(r), col_count(c), rows(r), state(TABLE) {
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
      
      // Apply to target row.
      template <typename SETTING_TYPE>
      Table & CSS(int row_id, const std::string & setting, SETTING_TYPE && value) {
        emp_assert(row_id >= 0 && row_id < row_count);
        rows[row_id].CSS(setting, value);
        return (Table &) *this;
      }
        
      // Apply to target cell.
      template <typename SETTING_TYPE>
      Table & CSS(int row_id, int col_id, const std::string & setting, SETTING_TYPE && value) {
        emp_assert(row_id >= 0 && row_id < row_count);
        emp_assert(col_id >= 0 && col_id < row_count);
        rows[row_id].CSS(setting, value);
        return (Table &) *this;
      }
        


      // Apply to all rows.  
      template <typename SETTING_TYPE>
      Table & RowsCSS(const std::string & setting, SETTING_TYPE && value) {
        // (@CAO: Can we use fancier jquery here?)
        for (auto & row : rows) row.CSS(setting, value);
        return (Table &) *this;
      }
        
      // Apply to all rows.  (@CAO: Can we use fancier jquery here?)
      template <typename SETTING_TYPE>
      Table & CellsCSS(const std::string & setting, SETTING_TYPE && value) {
        for (auto & row : rows) row.CellsCSS(setting, value);
        return (Table &) *this;
      }

    };

  };

};
};

#endif
