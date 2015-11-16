///////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2015.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Specs for the Table widget
//
//  TableInfo has two helper classes: TableRow and TableData.
//
//  A Table is composed of row x col cells
//  TableData may be muliple cells wide/tall, masking other cells.
//
//
//  Developer notes:
//  * Tables should more directly manage internal slates rather than just adding divs and
//    then having them filled in.
//  * TextTables should be created that simply use text in cells, radically speeding up
//    printing of such tables (and covering 80% of use cases).
//  * IDEALLY: Make a single table that will look at what each cell is pointing to (table
//    or text) and write out what it needs to, in place.
//

#ifndef EMP_WEB_TABLE_H
#define EMP_WEB_TABLE_H

#include "../tools/vector.h"

#include "Slate.h"
#include "Widget.h"

namespace emp {
namespace web {

  namespace internal {

    class TableRow;
    class TableInfo;

    class TableData {
      friend TableRow; friend Table; friend TableInfo;
    protected:
      int colspan;    // How many columns wide is this TableData?
      int rowspan;    // How many rows deep is this TableData?
      bool header;    // Is this TableData a header (<th> vs <td>)?
      bool masked;    // Is this cell masked by another cell?
      Style style;    // CSS Style
      
      Slate slate;   // Which slate is associated with this data cell?
      
    public:
      TableData() : colspan(1), rowspan(1), header(false), masked(false), slate("") { ; }
      ~TableData() { ; }
      
      int GetColSpan() const { return colspan; }
      int GetRowSpan() const { return rowspan; }
      bool IsHeader() const { return header; }
      bool IsMasked() const { return masked; }
      Style & GetStyle() { return style; }
      const Style & GetStyle() const { return style; }
      explicit operator bool() { return !masked; }      // Unmasked cell = true, masked = false.
      
      bool HasSlate() const { return slate; }
      Slate & GetSlate() {
        if (!slate) slate = Slate("");  // If we don't have a slate, build one!
        return slate;
      }
      
      void SetColSpan(int cs) { colspan = cs; }
      void SetRowSpan(int rs) { rowspan = rs; }
      void SetHeader(bool h=true) { header = h; }
      void SetMasked(bool m=true) { masked = m; }
      
      bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
        bool ok = true;
        if (verbose) ss << prefix << "Scanning: emp::TableData" << std::endl;
        
        if (slate && masked == true) {
          ss << "Warning: Masked cell may have contents!" << std::endl;
          ok = false;
        }
        
        return ok;
      }
      
    };  // END: TableData

    
    class TableRow {
      friend Table; friend TableInfo;
    protected:
      emp::vector<TableData> data;  // detail object for each cell in this row.
      Style style;
      
    public:
      TableRow() { ; }
      ~TableRow() { ; }
      
      int GetSize() const { return (int) data.size(); }               // How many cells in this row?
      TableData & operator[](int id) { return data[id]; }             // Get a single cell
      const TableData & operator[](int id) const { return data[id]; } // Get a single const cell
      emp::vector<TableData> & GetCells() { return data; }            // Get ALL cells
      
      TableRow & SetCols(int c) { data.resize(c); return *this; }

      // Apply to all cells in row.
      template <typename SETTING_TYPE>
      TableRow & CellsCSS(const std::string & setting, SETTING_TYPE && value) {
        for (auto & datum : data) datum.style.Set(setting, value);
        return *this;
      }
      
      // Apply to specific cell in row.
      template <typename SETTING_TYPE>
      TableRow & CellCSS(int col_id, const std::string & setting, SETTING_TYPE && value) {
        data[col_id].style.Set(setting, value);
        return *this;
      }

      virtual bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
        bool ok = true;
        if (verbose) { ss << prefix << "Scanning: emp::TableRow" << std::endl; }
        for (auto & cell : data) ok = ok && cell.OK(ss, verbose, prefix+"  ");      
        return ok;
      }
    };
    
    class TableInfo : public internal::WidgetInfo {
      friend Table;
    protected:
      int row_count;                // How big is this table?
      int col_count;
      emp::vector<TableRow> rows;   // detail object for each row.

      Table * append_widget;        // Which widget is triggering an append?
      
      TableInfo(const std::string & in_id="")
        : internal::WidgetInfo(in_id), row_count(0), col_count(0), append_widget(nullptr) { ; }
      TableInfo(const TableInfo &) = delete;               // No copies of INFO allowed
      TableInfo & operator=(const TableInfo &) = delete;   // No copies of INFO allowed
      virtual ~TableInfo() { ; }
      
      virtual bool IsTableInfo() const override { return true; }    

      void Resize(int new_rows, int new_cols) {
        // Resize existing rows        
        if (new_cols != col_count) {
          for (auto & row : rows) {
            row.SetCols(new_cols);
            for (int c = col_count; c < new_cols; c++) {
              row[c].slate->parent = this;
              if (state == Widget::ACTIVE) row[c].slate->DoActivate();
            }
          }
          col_count = new_cols;
        }

        // Resize number of rows.
        if (new_rows != row_count) {
          rows.resize(new_rows);
          for (int r = row_count; r < new_rows; r++) {
            rows[r].SetCols(col_count);
            for (int c = 0; c < col_count; c++) {
              rows[r][c].slate->parent = this;
              if (state == Widget::ACTIVE) rows[r][c].slate->DoActivate();
            }
          }
          row_count = new_rows;          
        }
        
      }
      
      void DoActivate(bool top_level=true) override {
        // Activate all of the cell slates.
        for (int r = 0; r < row_count; r++) {
          for (int c = 0; c < col_count; c++) {
            rows[r][c].slate->DoActivate(false);
          }
        }
        internal::WidgetInfo::DoActivate(top_level);
      }
      
      
      // Get a slate associated with the current cell (and build one if we need to...)
      Widget & GetCurSlate();
      
      // Add additional children on to this element.
      Widget Append(Widget info) override { return GetCurSlate() << info; }
      Widget Append(const std::string & text) override {
        return GetCurSlate() << text;
      }
      Widget Append(const std::function<std::string()> & in_fun) override {
        return GetCurSlate() << in_fun;
      }
      
      // Normally only slates deal with registering other widgets, but Tables
      // need to facilitate recursive registrations.
      
      void RegisterChildren(internal::SlateInfo * regestrar) override {
        for (int r = 0; r < row_count; r++) {
          for (int c = 0; c < col_count; c++) {
            regestrar->Register( rows[r][c].slate );
          }
        }
      }
      
      void UnregisterChildren(internal::SlateInfo * regestrar) override {
        for (int r = 0; r < row_count; r++) {
          for (int c = 0; c < col_count; c++) {
            regestrar->Unregister( rows[r][c].slate );
          }
        }
      }
      
      
      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");                                           // Clear the current text.
        HTML << "<table id=\"" << id << "\">";
        
        // Loop through all of the rows in the table. 
        for (int r = 0; r < (int) rows.size(); r++) {
          auto & row = rows[r];
          HTML << "<tr";
          if (row.style.GetSize()) HTML << " id=" << id << '_' << r;
          HTML << ">";
          
          // Loop through each cell in this row.
          for (int c = 0; c < row.GetSize(); c++) {
            auto & datum = row[c];
            if (datum.IsMasked()) continue;  // If this cell is masked by another, skip it!
            
            // Print opening tag.
            HTML << (datum.IsHeader() ? "<th" : "<td");
            
            // Include an id for this cell if we have one.
            if (datum.style.GetSize()) HTML << " id=" << id << '_' << r << '_' << c;

            // If this cell spans multiple rows or columns, indicate!
            if (datum.GetColSpan() > 1) HTML << " colspan=\"" << datum.GetColSpan() << "\"";
            if (datum.GetRowSpan() > 1) HTML << " rowspan=\"" << datum.GetRowSpan() << "\"";
            
            HTML << ">";
            
            // If this cell has contents, initialize them!
            if (datum.HasSlate()) {
              HTML << "<span id=\"" << datum.GetSlate().GetID() << "\"></span>\n";
            }
            
            // Print closing tag.
            HTML << (datum.IsHeader() ? "</th>" : "</td>");
          }
          
          HTML << "</tr>";
        }
        
        HTML << "</table>";
      }
      

      void ClearCell(int row_id, int col_id) {
        rows[row_id].data[col_id].colspan = 1;
        rows[row_id].data[col_id].rowspan = 1;
        Slate & slate = rows[row_id].data[col_id].GetSlate();
        if (slate) slate.ClearChildren();
        rows[row_id].data[col_id].header = false;
        rows[row_id].data[col_id].masked = false;  // @CAO Technically, cell might still be masked!
        rows[row_id].data[col_id].style.Clear();
      }
      void ClearRowCells(int row_id) {
        for (int col_id = 0; col_id < col_count; col_id++) ClearCell(row_id, col_id);
      }
      void ClearRow(int row_id) {
        rows[row_id].style.Clear();
        ClearRowCells(row_id);
      }
      void ClearTableCells() {
        for (int row_id = 0; row_id < row_count; row_id++) ClearRowCells(row_id);
      }
      void ClearTableRows() {
        for (int row_id = 0; row_id < row_count; row_id++) ClearRow(row_id);
      }
      void ClearTable() {
        style.Clear();
        Resize(0,0);
      }
      
      bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
        bool ok = true;
        
        // Basic info
        if (verbose) {
          ss << prefix << "Scanning: emp::TableInfo (rows=" << row_count
             << ", cols=" << col_count << ")." << std::endl;
        }
        
        // Make sure rows and columns are being sized correctly.
        if (row_count != (int) rows.size()) {
          ss << prefix << "Error: row_count = " << row_count
             << ", but rows has " << rows.size() << " elements." << std::endl;
          ok = false;
        }
        
        if (row_count < 1) {
          ss << prefix << "Error: Cannot have " << row_count
             << " rows in table." << std::endl;
          ok = false;
        }
        
        if (col_count < 1) {
          ss << prefix << "Error: Cannot have " << col_count
             << " cols in table." << std::endl;
          ok = false;
        }
        
        // Recursively call OK on rows and data.
        for (int r = 0; r < row_count; r++) {
          ok = ok && rows[r].OK(ss, verbose, prefix+"  ");
          if (col_count != rows[r].GetSize()) {
            ss << prefix << "  Error: col_count = " << col_count
               << ", but row has " << rows[r].GetSize() << " elements." << std::endl;
            ok = false;
          }
          for (int c = 0; c < col_count; c++) {
            auto & cell = rows[r][c];
            if (c + cell.GetColSpan() > col_count) {
              ss << prefix << "  Error: Cell at row " << r << ", col " << c
                 << " extends past right side of table." << std::endl;
              ok = false;
            }
            if (r + cell.GetRowSpan() > row_count) {
              ss << prefix << "  Error: Cell at row " << r << ", col " << c
                 << " extends past bottom of table." << std::endl;
              ok = false;
            }
          }
        }
        
        return ok;
      }

      
      void ReplaceHTML() override {
        // Replace Slate's HTML...
        internal::WidgetInfo::ReplaceHTML();

        // Then replace cells
        for (int r = 0; r < row_count; r++) {
          rows[r].style.Apply(emp::to_string(id, '_', r));
          for (int c = 0; c < col_count; c++) {
            auto & datum = rows[r][c];
            if (datum.IsMasked()) continue;  // If this cell is masked by another, skip it!
            datum.style.Apply(emp::to_string(id, '_', r, '_', c));
            datum.slate->ReplaceHTML();
          }
        }
      }

      
    public:
      virtual std::string GetType() override { return "web::TableInfo"; }
    }; // end TableInfo
    
    
  } // end namespace internal
  

  class Table : public internal::WidgetFacet<Table> {
    friend class internal::TableInfo;
  protected:

    int cur_row;      // Which row/col is currently active?
    int cur_col;
           
    // A table's state determines how some operations work.
    enum state_t { TABLE, ROW, CELL };
    state_t state;


    // Get a properly cast version of indo.  
    internal::TableInfo * Info() { return (internal::TableInfo *) info; }
    const internal::TableInfo * Info() const { return (internal::TableInfo *) info; }
 
    Table(internal::TableInfo * in_info) : WidgetFacet(in_info) { ; }

    // Apply to appropriate component based on current state.
    void DoCSS(const std::string & setting, const std::string & value) override {
      if (state == TABLE) WidgetFacet<Table>::DoCSS(setting, value);
      else if (state == ROW) {
        Info()->rows[cur_row].style.Set(setting, value);
        // @CAO need to make change active immediately...
      } else if (state == CELL) {
        Info()->rows[cur_row].data[cur_col].style.Set(setting, value);
        // @CAO need to make change active immediately...
      } else emp_assert(false && "Table in unknown state!");
    }
    
  public:

    Table(int r, int c, const std::string & in_id="")
      : WidgetFacet(in_id), cur_row(0), cur_col(0), state(TABLE)
    {
      emp_assert(c > 0 && r > 0);              // Ensure that we have rows and columns!

      info = new internal::TableInfo(in_id);
      Info()->Resize(r, c);
    }
    Table(const Table & in)
      : WidgetFacet(in), cur_row(in.cur_row), cur_col(in.cur_col), state(in.state)
    {
      emp_assert(state == TABLE || state == ROW || state == CELL, state);
    }
    Table(const Widget & in) : WidgetFacet(in), cur_row(0), cur_col(0), state(TABLE) {
      emp_assert(info->IsTableInfo());
    }
    virtual ~Table() { ; }

    using INFO_TYPE = internal::TableInfo;

    bool IsTable() const override { return true; } 

    int GetNumCols() const { return Info()->col_count; }
    int GetNumRows() const { return Info()->row_count; }
    int GetNumCells() const { return Info()->col_count*Info()->row_count; }

    // Called before an append.
    virtual void PrepareAppend() override { Info()->append_widget = this; }
    
    int GetCurRow() const { return cur_row; }
    int GetCurCol() const { return cur_col; }
    
    bool InStateTable() const { return state == TABLE; }
    bool InStateRow() const { return state == ROW; }
    bool InStateCell() const { return state == CELL; }
    
    Table & Clear() {
      // Clear based on tables current state.
      if (state == TABLE) Info()->ClearTable();
      else if (state == ROW) Info()->ClearRow(cur_row);
      else if (state == CELL) Info()->ClearCell(cur_row, cur_col);
      else emp_assert(false && "Table in unknown state!", state);
      return *this;
    }
    Table & ClearTable() { Info()->ClearTable(); return *this; }
    Table & ClearRows() { Info()->ClearTableRows(); return *this; }
    Table & ClearRow(int r) { Info()->ClearRow(r); return *this; }
    Table & ClearCells() {
      if (state == TABLE) Info()->ClearTableCells();
      else if (state == ROW) Info()->ClearRowCells(cur_row);
      else emp_assert(false && "Cannot run ClearCells on single cell!", state);
      return *this;
    }
    Table & ClearCell(int r, int c) { Info()->ClearCell(r, c); return *this; }
    

    Table & Rows(int r) {
      Info()->Resize(r, Info()->col_count);
      if (cur_row >= r) cur_row = 0;
      return *this;
    }
    Table & Cols(int c) {
      Info()->Resize(Info()->row_count, c);
      if (cur_col >= c) cur_col = 0;
      return *this;
    }
    Table & Resize(int r, int c) {
      Info()->Resize(r, c);
      if (cur_row >= r) cur_row = 0;
      if (cur_col >= c) cur_col = 0;
      return *this;
    }
      
    Table & GetCell(int r, int c) {
      emp_assert(Info() != nullptr);
      emp_assert(r < Info()->row_count && c < Info()->col_count,
                 r, c, Info()->row_count, Info()->col_count, GetID());
      cur_row = r; cur_col = c;
      state = CELL;
      return *this;
    }
    Table & GetRow(int r) {
      emp_assert(r < Info()->row_count,
                 r, Info()->row_count, GetID());
      cur_row = r; cur_col = 0;
      state = ROW;
      return *this;
    }
    Table & GetTable() {
      // Leave row and col where they are.
      state = TABLE;
      return *this;
    }


    Table & SetHeader(bool _h=true) {
      emp_assert(state == CELL);
      Info()->rows[cur_row].data[cur_col].SetHeader(_h);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
      return *this;
    }

    Widget AddText(int r, int c, const std::string & text) {
      GetCell(r,c) << text;
      return *this;
    }

    Widget AddHeader(int r, int c, const std::string & text) {
      GetCell(r,c) << text;
      SetHeader();
      return *this;
    }
    
    
    // Apply to appropriate component based on current state.
    using WidgetFacet<Table>::SetCSS;
    std::string GetCSS(const std::string & setting) override {
      if (state == TABLE) return Info()->style.Get(setting);
      if (state == ROW) return Info()->rows[cur_row].style.Get(setting);
      if (state == CELL) return Info()->rows[cur_row].data[cur_col].style.Get(setting);
      return "";
    }
    
    // Allow the row and column span of the current cell to be adjusted.
    Table & SetColSpan(int new_span) {
      emp_assert(state == CELL);
      emp_assert((cur_col + new_span <= GetNumCols()) && "Col span too wide for table!",
                 cur_col, new_span, GetNumCols(), GetID());
        
      auto & datum = Info()->rows[cur_row].data[cur_col];
      const int old_span = datum.GetColSpan();
      const int row_span = datum.GetRowSpan();
      datum.SetColSpan(new_span);
        
      // For each row, make sure new columns are masked!
      for (int row = cur_row; row < cur_row + row_span; row++) {
        for (int col = cur_col + old_span; col < cur_col + new_span; col++) {
          Info()->rows[row].data[col].SetMasked(true);
        }
      }
        
      // For each row, make sure former columns are unmasked!
      for (int row = cur_row; row < cur_row + row_span; row++) {
        for (int col = cur_col + new_span; col < cur_col + old_span; col++) {
          Info()->rows[row].data[col].SetMasked(false);
        }
      }
      
      // Redraw the entire table to fix col span information.
      if (IsActive()) Info()->ReplaceHTML();

      return *this;
    }
      
    Table & SetRowSpan(int new_span) {
      emp_assert((cur_row + new_span <= GetNumRows()) && "Row span too wide for table!");
      emp_assert(state == CELL);
      
      auto & datum = Info()->rows[cur_row].data[cur_col];
      const int old_span = datum.GetRowSpan();
      const int col_span = datum.GetColSpan();
      datum.SetRowSpan(new_span);
      
      // For each col, make sure NEW rows are masked!
      for (int row = cur_row + old_span; row < cur_row + new_span; row++) {
        for (int col = cur_col; col < cur_col + col_span; col++) {
          Info()->rows[row].data[col].SetMasked(true);
        }
      }
        
      // For each row, make sure former columns are unmasked!
      for (int row = cur_row + new_span; row < cur_row + old_span; row++) {
        for (int col = cur_col; col < cur_col + col_span; col++) {
          Info()->rows[row].data[col].SetMasked(false);
        }
      }
      
      // Redraw the entire table to fix row span information.
      if (IsActive()) Info()->ReplaceHTML();

      return *this;
    }
    

    // Apply to target row.
    template <typename SETTING_TYPE>
    Table & RowCSS(int row_id, const std::string & setting, SETTING_TYPE && value) {
      emp_assert(row_id >= 0 && row_id < Info()->row_count);
      Info()->rows[row_id].style.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace row's CSS
      return *this;
    }
       
    // Apply to target cell.
    template <typename SETTING_TYPE>
    Table & CellCSS(int row_id, int col_id, const std::string & setting, SETTING_TYPE && value) {
      emp_assert(row_id >= 0 && row_id < Info()->row_count);
      emp_assert(col_id >= 0 && col_id < Info()->row_count);
      Info()->rows[row_id].style.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
      return *this;
    }
        
    // Apply to all rows.  (@CAO: Should we use fancier jquery here?)
    template <typename SETTING_TYPE>
    Table & RowsCSS(const std::string & setting, SETTING_TYPE && value) {
      for (auto & row : Info()->rows) row.style.Set(setting, emp::to_string(value));
      if (IsActive()) Info()->ReplaceHTML();
      return *this;
    }
        
    // Apply to all rows.  (@CAO: Should we use fancier jquery here?)
    template <typename SETTING_TYPE>
    Table & CellsCSS(const std::string & setting, SETTING_TYPE && value) {
      for (auto & row : Info()->rows) row.CellsCSS(setting, emp::to_string(value));
      if (IsActive()) Info()->ReplaceHTML();
      return *this;
    }

    virtual bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
      bool ok = true;

      // Basic info
      if (verbose) {
        ss << prefix << "Scanning: emp::Table (rows=" << Info()->row_count
           << ", cols=" << Info()->col_count << ")." << std::endl;
      }

      // Make sure current row and col are valid.
      if (cur_row < 0 || cur_row >= Info()->row_count) {
        ss << prefix << "Error: cur_row = " << cur_row << "." << std::endl;
        ok = false;
      }
        
      if (cur_col < 0 || cur_col >= Info()->col_count) {
        ss << prefix << "Error: cur_col = " << cur_col << "." << std::endl;
        ok = false;
      }

      // Make sure internal info is okay.
      ok = ok && Info()->OK(ss, verbose, prefix+"  ");
 
      return ok;
    }
  };

  // Setup mechanism to retrieve current slate for table append.
  Widget & internal::TableInfo::GetCurSlate() {
    int cur_row = append_widget->cur_row;
    int cur_col = append_widget->cur_col;

    // Make sure the number of rows hasn't changed, making the current position illegal.
    if (cur_row >= row_count) cur_row = 0;
    if (cur_col >= col_count) cur_col = 0;

    return rows[cur_row].data[cur_col].GetSlate();
  }
  
}
}

#endif
