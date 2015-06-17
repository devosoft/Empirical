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

#include "Widget.h"

namespace emp {
namespace UI {

  class TableRow;
  class Table;

  class TableData : public internal::Widget<TableData> {
    friend TableRow; friend Table;
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
    TableData() : colspan(1), rowspan(1), child_id(-1), header(false), masked(false) {
      obj_ext = "td";    // Make sure table row has own CSS.
    }
    ~TableData() { ; }

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
    
    static std::string TypeName() { return "TableData"; }

    bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
      bool ok = true;
      if (verbose) {
        ss << prefix << "Scanning: emp::TableData with child_id = " << child_id << std::endl;
      }

      if (child_id >= 0 && masked == true) {
        ss << "Warning: Masked row has contents!" << std::endl;
        ok = false;
      }
      
      return ok;
    }
    
  };
    
  class TableRow : public internal::Widget<TableRow>{
    friend Table;
  protected:
    std::vector<TableData> data;  // detail object for each cell in this row.

    void UpdateCSS() {
      std::string obj_id = div_id + obj_ext;
      css_info.Apply(obj_id);
      for (auto & datum : data) datum.UpdateCSS();
    }
  public:
    TableRow() { obj_ext = "tr"; }
    ~TableRow() { ; }

    int GetSize() { return (int) data.size(); }

    TableRow & SetCols(int c) { data.resize(c); return *this; }

    // Apply to all cells in row.  (@CAO: Can we use fancier jquery here?)
    template <typename SETTING_TYPE>
    TableRow & CellsCSS(const std::string & setting, SETTING_TYPE && value) {
      for (auto & datum : data) datum.CSS(setting, value);
      return *this;
    }
        
    // Apply to specific cell in row.
    template <typename SETTING_TYPE>
    TableRow & CellCSS(int col_id, const std::string & setting, SETTING_TYPE && value) {
      data[col_id].CSS(setting, value);
      return *this;
    }

    std::vector<TableData> & GetCells() { return data; }

    // NOTE: Regular CSS applied to a TableRow will modify row's own CSS.

    static std::string TypeName() { return "TableRow"; }

    virtual bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
      bool ok = true;
      if (verbose) {
        ss << prefix << "Scanning: emp::TableRow" << std::endl;
      }

      for (auto & cell : data) ok = ok && cell.OK(ss, verbose, prefix+"  ");
      
      return ok;
    }
  };

  class Table : public internal::Widget<Table> {
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

    TableData & GetCurCell() { return rows[cur_row].data[cur_col]; }

  public:
    Table(int r, int c, const std::string & in_name="")
      : Widget(in_name)
      , row_count(r), col_count(c), rows(r), state(TABLE) {
      emp_assert(c > 0 && r > 0);              // Ensure that we have rows and columns!
      obj_ext = "__t";                         // Make sure table has own CSS.
      for (auto & row : rows) row.SetCols(c);  // Set all rows to correct number of columns.
    }
    ~Table() { ; }

    int GetNumCols() const { return col_count; }
    int GetNumRows() const { return row_count; }
    int GetNumCells() const { return col_count*row_count; }
    
    int GetCurRow() const { return cur_row; }
    int GetCurCol() const { return cur_col; }
    
    bool InStateTable() const { return state == TABLE; }
    bool InStateRow() const { return state == ROW; }
    bool InStateCell() const { return state == CELL; }
    

    Table & Rows(int r) {
      rows.resize(r);                                                 // Resize rows.
      for (int i = row_count; i < r; i++) rows[i].SetCols(col_count); // Initialize new rows.
      row_count = r;                                                  // Store new size.
      return *this;
    }
    Table & Cols(int c) {
      col_count = c;                                    // Store new size.
      for (auto & row : rows) row.SetCols(col_count);   // Make sure all rows have new col_count
      return *this;
    }
      
    Table & GetCell(int r, int c) {
      cur_row = r; cur_col = c;
      state = CELL;
      return *this;
    }
    Table & GetRow(int r) {
      cur_row = r; cur_col = 0;
      state = ROW;
      return *this;
    }
    Table & GetTable() {
      // Leave row and col where they are.
      state = TABLE;
      return *this;
    }

    // Apply to appropriate component based on current state.
    template <typename SETTING_TYPE>      
    Table & CSS(const std::string & setting, SETTING_TYPE && value) {
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
      
      return *this;
    }
    
    // Allow the row and column span of the current cell to be adjusted.
    Table & SetColSpan(int new_span) {
      emp_assert((cur_col + new_span <= GetNumCols()) && "Col span too wide for table!");
        
      auto & datum = rows[cur_row].data[cur_col];
      const int old_span = datum.GetColSpan();
      const int row_span = datum.GetRowSpan();
      datum.SetColSpan(new_span);
        
      // For each row, make sure new columns are masked!
      for (int row = cur_row; row < cur_row + row_span; row++) {
        for (int col = cur_col + old_span; col < cur_col + new_span; col++) {
          rows[row].data[col].SetMasked(true);
        }
      }
        
      // For each row, make sure former columns are unmasked!
      for (int row = cur_row; row < cur_row + row_span; row++) {
        for (int col = cur_col + new_span; col < cur_col + old_span; col++) {
          rows[row].data[col].SetMasked(false);
        }
      }
      
      return *this;
    }
      
    Table & SetRowSpan(int new_span) {
      emp_assert((cur_row + new_span <= GetNumRows()) && "Row span too wide for table!");
      
      auto & datum = rows[cur_row].data[cur_col];
      const int old_span = datum.GetRowSpan();
      const int col_span = datum.GetColSpan();
      datum.SetRowSpan(new_span);
      
      // For each col, make sure NEW rows are masked!
      for (int row = cur_row + old_span; row < cur_row + new_span; row++) {
        for (int col = cur_col; col < cur_col + col_span; col++) {
          rows[row].data[col].SetMasked(true);
        }
      }
        
      // For each row, make sure former columns are unmasked!
      for (int row = cur_row + new_span; row < cur_row + old_span; row++) {
        for (int col = cur_col; col < cur_col + col_span; col++) {
          rows[row].data[col].SetMasked(false);
        }
      }
      
      return *this;
    }
    

    // Apply to target row.
    template <typename SETTING_TYPE>
    Table & RowCSS(int row_id, const std::string & setting, SETTING_TYPE && value) {
      emp_assert(row_id >= 0 && row_id < row_count);
      rows[row_id].CSS(setting, value);
      return *this;
    }
       
    // Apply to target cell.
    template <typename SETTING_TYPE>
    Table & CellCSS(int row_id, int col_id, const std::string & setting, SETTING_TYPE && value) {
      emp_assert(row_id >= 0 && row_id < row_count);
      emp_assert(col_id >= 0 && col_id < row_count);
      rows[row_id].CSS(setting, value);
      return *this;
    }
        
    // Apply to all rows.  (@CAO: Should we use fancier jquery here?)
    template <typename SETTING_TYPE>
    Table & RowsCSS(const std::string & setting, SETTING_TYPE && value) {
      for (auto & row : rows) row.CSS(setting, value);
      return *this;
    }
        
    // Apply to all rows.  (@CAO: Should we use fancier jquery here?)
    template <typename SETTING_TYPE>
    Table & CellsCSS(const std::string & setting, SETTING_TYPE && value) {
      for (auto & row : rows) row.CellsCSS(setting, value);
      return *this;
    }

    static std::string TypeName() { return "Table"; }
    
    virtual bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
      bool ok = true;
      if (verbose) {
        ss << prefix << "Scanning: emp::Table (rows=" << row_count
           << ", cols=" << col_count << ")." << std::endl;
      }
      
      if (row_count != (int) rows.size()) {
        ss << prefix << "Error: row_count = " << row_count
           << ", but rows has " << rows.size() << " elements." << std::endl;
        ok = false;
      }
      
      if (row_count < 1) {
        ss << prefix << "Error: Cannot have " << row_count << " rows in table." << std::endl;
        ok = false;
      }

      if (col_count < 1) {
        ss << prefix << "Error: Cannot have " << col_count << " cols in table." << std::endl;
        ok = false;
      }

      if (cur_row < 0 || cur_row >= row_count) {
        ss << prefix << "Error: cur_row = " << cur_row << "." << std::endl;
        ok = false;
      }
        
      if (cur_col < 0 || cur_col >= col_count) {
        ss << prefix << "Error: cur_col = " << cur_col << "." << std::endl;
        ok = false;
      }
      
      for (auto & row : rows) {
        ok = ok && row.OK(ss, verbose, prefix+"  ");
        if (col_count != row.GetSize()) {
          ss << prefix << "  Error: col_count = " << col_count
             << ", but row has " << row.GetSize() << " elements." << std::endl;
          ok = false;
        }
      }


      return ok;
    }
  };

};
};

#endif
