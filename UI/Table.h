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
      friend TableRow_detail; friend Table_detail;
    protected:
      int colspan;  // How many columns wide is this TableData?
      int rowspan;  // How many rows deep is this TableData?
      int child_id; // Which child element is this cell associated with?
      bool header;  // Is this TableData a header (<th> vs <td>)?
      bool masked;  // Is this cell masked by another cell?

      void UpdateCSS() {
        std::string obj_id = div_id + obj_ext;
        css_info.Apply(obj_id);
      }
    public:
      TableData_detail() : colspan(1), rowspan(1), child_id(-1), header(false), masked(false) {
        obj_ext = "td";    // Make sure table row has own CSS.
      }
      ~TableData_detail() { ; }

      int GetColSpan() const { return colspan; }
      int GetRowSpan() const { return rowspan; }
      int GetChildID() const { return child_id; }

      bool IsHeader() const { return header; }
      bool IsMasked() const { return masked; }

      void SetColSpan(int cs) { colspan = cs; }
      void SetRowSpan(int rs) { rowspan = rs; }
      void SetChildID(int cid) { child_id = cid; }
      void SetHeader(bool h=true) { header = h; }
      void SetMasked(bool m=true) { masked = m; }
    };
    
    class TableRow_detail : public Widget_base{
      friend Table_detail;
    protected:
      std::vector<TableData> data;  // detail object for each cell in this row.

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

      TableRow & SetCols(int c) { data.resize(c); return (TableRow &) *this; }

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

      std::vector<TableData> & GetCells() { return data; }

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

      void UpdateCSS() {
        std::string obj_id = div_id + obj_ext;
        css_info.Apply(obj_id);
        for (auto & row : rows) row.UpdateCSS();
      }

      TableData_detail & GetCurCell() { return rows[cur_row].data[cur_col]; }

    public:
      Table_detail(int r, int c) : row_count(r), col_count(c), rows(r), state(TABLE) {
        emp_assert(c > 0 && r > 0);              // Ensure that we have rows and columns!
        obj_ext = "__t";                         // Make sure table has own CSS.
        for (auto & row : rows) row.SetCols(c);  // Set all rows to correct number of columns.
      }
      ~Table_detail() { ; }

      int GetNumCols() const { return col_count; }
      int GetNumRows() const { return row_count; }
      int GetNumCells() const { return col_count*row_count; }

      int GetCurRow() const { return cur_row; }
      int GetCurCol() const { return cur_col; }

      bool InTableState() const { return state == TABLE; }
      bool InRowState() const { return state == ROW; }
      bool InCellState() const { return state == CELL; }


      Table & Cols(int c) { col_count = c; return (Table &) *this; }
      Table & Rows(int r) { row_count = r; return (Table &) *this; }
      
      Table & GetCell(int r, int c) {
        cur_row = r; cur_col = c;
        state = CELL;
        return (Table &) *this;
      }
      Table & GetRow(int r) {
        cur_row = r; cur_col = 0;
        state = ROW;
        return (Table &) *this;
      }
      Table & GetTable() {
        // Leave row and col where they are.
        state = TABLE;
        return (Table &) *this;
      }

      // Apply to appropriate component based on current state.
      template <typename SETTING_TYPE>
      Table & CSS(const std::string & setting, SETTING_TYPE && value) {
        CSS(setting, value);
        return (Table &) *this;
      }

      // Function to override if a widget wants to be able to redirect CSS calls.
      template <typename SETTING_TYPE>      
      bool RedirectCSS(const std::string & setting, SETTING_TYPE && value) {
        switch (state) {
        case TABLE:
          css_info.Set(setting, value);
          break;
        case ROW:
          rows[cur_row].CSS(setting, value);
          break;
        case CELL:
          rows[cur_row].data[cur_col].CSS(setting, value);
          break;
        };

        return true;
      }
      

      // Apply to target row.
      template <typename SETTING_TYPE>
      Table & RowCSS(int row_id, const std::string & setting, SETTING_TYPE && value) {
        emp_assert(row_id >= 0 && row_id < row_count);
        rows[row_id].CSS(setting, value);
        return (Table &) *this;
      }
       
      // Apply to target cell.
      template <typename SETTING_TYPE>
      Table & CellCSS(int row_id, int col_id, const std::string & setting, SETTING_TYPE && value) {
        emp_assert(row_id >= 0 && row_id < row_count);
        emp_assert(col_id >= 0 && col_id < row_count);
        rows[row_id].CSS(setting, value);
        return (Table &) *this;
      }
        
      // Apply to all rows.  (@CAO: Should we use fancier jquery here?)
      template <typename SETTING_TYPE>
      Table & RowsCSS(const std::string & setting, SETTING_TYPE && value) {
        for (auto & row : rows) row.CSS(setting, value);
        return (Table &) *this;
      }
        
      // Apply to all rows.  (@CAO: Should we use fancier jquery here?)
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
