//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2015.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The Table widget
//
//  TableInfo is the core information for a table and has two helper classes:
//  TableRow and TableData.  The Table class is a smart pointer to a TableInfo
//  object.
//
//  A Table is composed of a series of rows, each with the same number of columns.
//  TableData may be muliple cells wide/tall, masking other cells.
//
//  Constructors:
//
//    Table(int r, int c, const std::string & in_id="")
//      Create a new r-by-c table with an optional DOM id specified.
//      State is initialized to TABLE; the first row and first cell are default locations.
//
//    Table(const Widget & in)
//      Point to an existing table (assert that widget IS a table!)
//
//
//  Accessors:
//
//    int GetNumCols() const
//    int GetNumRows() const
//    int GetNumCells() const
//      Return associated information about the table size.
//
//    int GetCurRow() const { return cur_row; }
//    int GetCurCol() const { return cur_col; }
//      Return information about the focal position on the table.
//
//
//  Adjusting Table size:
//
//    Table & Rows(int r)
//    Table & Cols(int c)
//    Table & Resize(int r, int c)
//      Set the number of rows, columns, or both in the table.
//
//
//  Setting or identifying the current table state:
//
//    bool InStateTable() const
//    bool InStateRow() const
//    bool InStateCell() const
//      Return true/false to identify what state the table is currently in.
//
//    Table & GetCell(int r, int c)
//      Make the specified row and column active and the table state CELL.
//      All further manipulations of the table object will focus on that cell until
//      the state is changed again.
//
//    Table & GetRow(int r)
//      Make the specified row active, column zero, and the table state ROW
//      All further manipulations of the table object will focus on that row until
//      the state is changed again.
//
//    Table & GetCol(int c)
//      Make the specified column active, row zero, and the table state COL
//      All further manipulations of the table object will focus on that column until
//      the state is changed again.
//
//    Table & GetTable()
//      Leave the active row and column, but set the table state to TABLE
//      All further manipulations of the table object will focus on the whole table
//      until the state is changed again.
//
//
//  Modifying data in table
//
//    Table & SetHeader(bool _h=true)
//      Set the current cell to be a header (or not if false is passed in)
//
//    Widget AddText(int r, int c, const std::string & text)
//      Add text to the specified table cell.
//
//    Widget AddHeader(int r, int c, const std::string & text)
//      Add text to the specified table cell AND set the cell to be a header.
//
//    Table & SetRowSpan(int row_span)
//    Table & SetColSpan(int col_span)
//    Table & SetSpan(int row_span, int col_span)
//      Allow the row and/or column span of the current cell to be adjusted.
//
//
//  Clearing table contents:
//
//    Table & ClearTable()
//      Clear all style information from table, remove contents from all cells, and
//      shrink table to no rows and no cells.
//
//    Table & ClearRows()
//      Clear style information from rows and cells and remove contents from cells
//      (but leave table style information and size.)
//
//    Table & ClearRow(int r)
//      Clear style information from the specified row and contents from all cells
//      in that row (but leave other rows untouched).
//
//    Table & ClearCells()
//      If state is TABLE, clear contents from all cells in entire table.
//      If state is ROW, clear contents from all cells in that row.
//      If state is COL, clear contents from all cells in that column.
//      If state is CELL, clear just that single cell.
//
//    Table & ClearCell(int r, int c)
//      Clear contents of just the specified cell.
//
//    Table & Clear()
//      Dynamically clear the entire active state (TABLE, ROW, COL, or CELL).
//
//
//  Style manipulation
//
//    std::string GetCSS(const std::string & setting)
//    std::string GetCSS(const std::string & setting, SETTING_TYPE && value)
//      Get or Set the current value of the specified Style setting, based on the state of
//      the table (i.e., TABLE affects full table style, ROW affects active row style, and
//      CELL affects active cell style.)
//
//    Table & RowCSS(int row_id, const std::string & setting, SETTING_TYPE && value)
//    Table & CellCSS(int row_id, int col_id, const std::string & setting, SETTING_TYPE && value)
//      Set the specified row or cell Style to the value indicated.
//
//    Table & RowsCSS(const std::string & setting, SETTING_TYPE && value)
//    Table & CellsCSS(const std::string & setting, SETTING_TYPE && value)
//      Set the specified Style setting of all rows or all cells to the value indicated.
//
//
//
//  Developer notes:
//  * Tables should more directly manage internal slates rather than just adding divs and
//    then having them filled in.
//  * TextTables should be created that simply use text in cells, radically speeding up
//    printing of such tables (and covering 80% of use cases).
//  * IDEALLY: Make a single table that will look at what each cell is pointing to (table
//    or text) and write out what it needs to, in place.
//  * Add a ClearColumn method, as well as other column functionality.


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
      
      emp::vector<Widget> children;  // Widgets contained in this cell.
      
    public:
      TableData() : colspan(1), rowspan(1), header(false), masked(false) { ; }
      ~TableData() { ; }
      
      bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
        bool ok = true;
        if (verbose) ss << prefix << "Scanning: emp::TableData" << std::endl;
        if (masked) { ss << "Warning: Masked cell may have contents!" << std::endl; ok = false; }
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

    class TableCol {
      friend Table; friend TableInfo;
    protected:
      Style style;
      int span;
      bool masked;

    public:
      TableCol() : span(1), masked(false) { ; }
      ~TableCol() { ; }

      int GetSpan() const { return span; }
    };
    
    class TableInfo : public internal::WidgetInfo {
      friend Table;
    protected:
      int row_count;                // How big is this table?
      int col_count;
      emp::vector<TableRow> rows;   // detail object for each row.
      emp::vector<TableCol> cols;   // detail object for each column (if needed).
      
      int append_row;               // Which row is triggering an append?
      int append_col;               // Which col is triggering an append?
      
      TableInfo(const std::string & in_id="")
        : internal::WidgetInfo(in_id), row_count(0), col_count(0), append_row(0), append_col(0) { ; }
      TableInfo(const TableInfo &) = delete;               // No copies of INFO allowed
      TableInfo & operator=(const TableInfo &) = delete;   // No copies of INFO allowed
      virtual ~TableInfo() { ; }
      
      virtual bool IsTableInfo() const override { return true; }    

      void Resize(int new_rows, int new_cols) {
        // Resize existing rows        
        if (new_cols != col_count) {
          for (int r = 0; r < (int) rows.size() && r < new_rows; r++) {
            rows[r].SetCols(new_cols);
            for (int c = col_count; c < new_cols; c++) { AddChild(r, c, Text("")); }
          }
          col_count = new_cols;
        }

        // Resize number of rows.
        if (new_rows != row_count) {
          rows.resize(new_rows);
          for (int r = row_count; r < new_rows; r++) {
            rows[r].SetCols(col_count);
            for (int c = 0; c < col_count; c++) { AddChild(r, c, Text("")); }
          }
          row_count = new_rows;          
        }
        
      }
      
      void DoActivate(bool top_level=true) override {
        // Activate all of the cell children.        
        for (int r = 0; r < row_count; r++) {
          for (int c = 0; c < col_count; c++) {
            for (auto & child : rows[r][c].children) child->DoActivate(false);
          }
        }

        // Activate this Table.
        internal::WidgetInfo::DoActivate(top_level);
      }
      
      
      // Return a text element for appending into a specific cell.
      // If the last element is text, use it; otherwise build a new one.
      web::Text & GetTextWidget(int r, int c) {
        // If the final element is not text, add one.
        if (rows[r][c].children.size() == 0
            || rows[r][c].children.back().IsText() == false
            || rows[r][c].children.back().AppendOK() == false)  {
          AddChild(Text());
        }
        return (Text &) rows[r][c].children.back();
      }
      
      web::Text & GetTextWidget() {
        // Make sure the number of rows hasn't changed, making the current position illegal.
        if (append_row >= row_count) append_row = 0;
        if (append_col >= col_count) append_col = 0;

        return GetTextWidget(append_row, append_col);
      }
      
      // Append into the current cell
      Widget Append(Widget in) override { AddChild(in); return in; }
      Widget Append(const std::string & text) override { return GetTextWidget() << text; }
      Widget Append(const std::function<std::string()> & in_fun) override {
        return GetTextWidget() << in_fun;
      }

      

      // Add a widget to the specified cell in the current table.
      void AddChild(int r, int c, Widget in) {
        emp_assert(in->parent == nullptr && "Cannot insert widget if already has parent!", in->id);
        emp_assert(in->state != Widget::ACTIVE && "Cannot insert a stand-alone active widget!");

        // Setup parent-child relationship in the specified cell.
        rows[r][c].children.emplace_back(in);
        in->parent = this;
        Register(in);

        
        // If this element (as new parent) is active, anchor widget and activate it!
        if (state == Widget::ACTIVE) {
          // Create a span tag to anchor the new widget.
          std::string cell_id = emp::to_string(id, '_', r, '_', c);
          EM_ASM_ARGS({
              parent_id = Pointer_stringify($0);
              child_id = Pointer_stringify($1);
              $('#' + parent_id).append('<span id=\'' + child_id + '\'></span>');
            }, cell_id.c_str(), in.GetID().c_str());

          // Now that the new widget has some place to hook in, activate it!
          in->DoActivate();
        }
      }

      // If no cell is specified for AddChild, use the current cell.
      void AddChild(Widget in) {
        // Make sure the number of rows hasn't changed, making the current position illegal.
        if (append_row >= row_count) append_row = 0;
        if (append_col >= col_count) append_col = 0;

        AddChild(append_row, append_col, in);
      }
        

      // Tables need to facilitate recursive registrations
      
      void RegisterChildren(internal::SlateInfo * regestrar) override {
        for (int r = 0; r < row_count; r++) {
          for (int c = 0; c < col_count; c++) {
            for (Widget & child : rows[r][c].children) regestrar->Register(child);
          }
        }
      }
      
      void UnregisterChildren(internal::SlateInfo * regestrar) override {
        for (int r = 0; r < row_count; r++) {
          for (int c = 0; c < col_count; c++) {
            for (Widget & child : rows[r][c].children) regestrar->Unregister(child);
          }
        }
      }
      
      
      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");                                           // Clear the current text.
        HTML << "<table id=\"" << id << "\">";

        // Include any column details, if needed.
        if (cols.size()) {
          for (int c = 0; c < (int) cols.size(); ++c) {
            if (cols[c].masked) continue;     // Skip masked columns.
            HTML << "<colgroup";
            if (cols[c].style.GetSize()) HTML << " id=" << id << "_c" << c;
            HTML << ">";

            HTML << "<col";
            if (cols[c].span > 1) HTML << " span=" << cols[c].span;
            HTML << ">";
          }
        }
        
        // Loop through all of the rows in the table. 
        for (int r = 0; r < (int) rows.size(); r++) {
          auto & row = rows[r];
          HTML << "<tr";
          if (row.style.GetSize()) HTML << " id=" << id << '_' << r;
          HTML << ">";
          
          // Loop through each cell in this row.
          for (int c = 0; c < row.GetSize(); c++) {
            auto & datum = row[c];
            if (datum.masked) continue;  // If this cell is masked by another, skip it!
            
            // Print opening tag.
            HTML << (datum.header ? "<th" : "<td");
            
            // Include an id for this cell if we have one.
            if (datum.style.GetSize()) HTML << " id=" << id << '_' << r << '_' << c;

            // If this cell spans multiple rows or columns, indicate!
            if (datum.colspan > 1) HTML << " colspan=\"" << datum.colspan << "\"";
            if (datum.rowspan > 1) HTML << " rowspan=\"" << datum.rowspan << "\"";
            
            HTML << ">";
            
            // Loop through all children of this cell and build a span element for each.
            for (Widget & w : datum.children) {
              HTML << "<span id=\'" << w.GetID() << "'></span>";
            }
            
            // Print closing tag.
            HTML << (datum.header ? "</th>" : "</td>");
          }
          
          HTML << "</tr>";
        }
        
        HTML << "</table>";
      }
      

      void ClearCell(int row_id, int col_id) {
        auto & datum = rows[row_id].data[col_id];
        datum.colspan = 1;
        datum.rowspan = 1;
        datum.header = false;
        datum.masked = false;  // @CAO Technically, cell might still be masked!
        datum.style.Clear();
        
        // Clear out this cell's children.
        // @CAO: Keep a starting text widget if we can!
        if (parent) {
          for (Widget & child : datum.children) parent->Unregister(child);
        }
        datum.children.resize(0);
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
            if (c + cell.colspan > col_count) {
              ss << prefix << "  Error: Cell at row " << r << ", col " << c
                 << " extends past right side of table." << std::endl;
              ok = false;
            }
            if (r + cell.rowspan > row_count) {
              ss << prefix << "  Error: Cell at row " << r << ", col " << c
                 << " extends past bottom of table." << std::endl;
              ok = false;
            }
          }
        }
        
        return ok;
      }

      
      void ReplaceHTML() override {
        // Replace Table's HTML...
        internal::WidgetInfo::ReplaceHTML();

        // Then replace cells
        for (int r = 0; r < row_count; r++) {
          rows[r].style.Apply(emp::to_string(id, '_', r));
          for (int c = 0; c < col_count; c++) {
            auto & datum = rows[r][c];
            if (datum.masked) continue;  // If this cell is masked by another, skip it!
            datum.style.Apply(emp::to_string(id, '_', r, '_', c));

            // If this widget is active, immediately replace children.
            if (state == Widget::ACTIVE) {
              for (auto & child : datum.children) child->ReplaceHTML();
            }

          }
        }

        // And setup columns.
        if (cols.size()) {
          for (int c = 0; c < col_count; c++) {
            if (cols[c].masked || cols[c].style.GetSize()==0) continue;
            cols[c].style.Apply(emp::to_string(id, "_c", c));
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
    enum state_t { TABLE, ROW, CELL, COL };
    state_t state;


    // Get a properly cast version of indo.  
    internal::TableInfo * Info() { return (internal::TableInfo *) info; }
    const internal::TableInfo * Info() const { return (internal::TableInfo *) info; }
 
    Table(internal::TableInfo * in_info) : WidgetFacet(in_info) { ; }

    // Apply to appropriate component based on current state.
    void DoCSS(const std::string & setting, const std::string & value) override {
      switch (state) {
      case TABLE:
        WidgetFacet<Table>::DoCSS(setting, value);
        break;
      case ROW:
        Info()->rows[cur_row].style.Set(setting, value);
        // @CAO need to make change active immediately...
        break;
      case CELL:
        Info()->rows[cur_row].data[cur_col].style.Set(setting, value);
        // @CAO need to make change active immediately...
        break;
      case COL:
        // If we haven't setup columns at all yet, do so.
        if (Info()->cols.size() == 0) Info()->cols.resize(GetNumCols());
        Info()->cols[cur_col].style.Set(setting, value);
      default:
        emp_assert(false && "Table in unknown state!");
      };
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
      emp_assert(state == TABLE || state == ROW || state == CELL || state == COL, state);
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
    virtual void PrepareAppend() override {
      Info()->append_row = cur_row;
      Info()->append_col = cur_col;
    }
    
    int GetCurRow() const { return cur_row; }
    int GetCurCol() const { return cur_col; }
    
    bool InStateTable() const { return state == TABLE; }
    bool InStateRow() const { return state == ROW; }
    bool InStateCol() const { return state == COL; }
    bool InStateCell() const { return state == CELL; }
    
    Table & Clear() {
      // Clear based on tables current state.
      if (state == TABLE) Info()->ClearTable();
      else if (state == ROW) Info()->ClearRow(cur_row);
      // @CAO Make work for state == COL
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
      // @CAO Make work for state == COL
      else if (state == CELL) Info()->ClearCell(cur_row, cur_col);
      else emp_assert(false && "Unknown State!", state);
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
    Table & GetCol(int c) {
      emp_assert(c < Info()->col_count,
                 c, Info()->col_count, GetID());
      cur_col = c; cur_row = 0;
      state = COL;
      return *this;
    }
    Table & GetTable() {
      // Leave row and col where they are.
      state = TABLE;
      return *this;
    }


    Table & SetHeader(bool _h=true) {
      emp_assert(state == CELL);
      Info()->rows[cur_row].data[cur_col].header = _h;
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
      if (state == COL) return Info()->cols[cur_col].style.Get(setting);
      if (state == CELL) return Info()->rows[cur_row].data[cur_col].style.Get(setting);
      return "";
    }
    
    // Allow the row and column span of the current cell to be adjusted.
    Table & SetRowSpan(int new_span) {
      emp_assert((cur_row + new_span <= GetNumRows()) && "Row span too wide for table!");
      emp_assert(state == CELL);
      
      auto & datum = Info()->rows[cur_row].data[cur_col];
      const int old_span = datum.rowspan;
      const int col_span = datum.colspan;
      datum.rowspan = new_span;
      
      // For each col, make sure NEW rows are masked!
      for (int row = cur_row + old_span; row < cur_row + new_span; row++) {
        for (int col = cur_col; col < cur_col + col_span; col++) {
          Info()->rows[row].data[col].masked = true;
        }
      }
        
      // For each row, make sure former columns are unmasked!
      for (int row = cur_row + new_span; row < cur_row + old_span; row++) {
        for (int col = cur_col; col < cur_col + col_span; col++) {
          Info()->rows[row].data[col].masked = false;
        }
      }
      
      // Redraw the entire table to fix row span information.
      if (IsActive()) Info()->ReplaceHTML();

      return *this;
    }
    
    Table & SetColSpan(int new_span) {
      emp_assert((cur_col + new_span <= GetNumCols()) && "Col span too wide for table!",
                 cur_col, new_span, GetNumCols(), GetID());
      emp_assert(state == CELL);
        
      auto & datum = Info()->rows[cur_row].data[cur_col];
      const int old_span = datum.colspan;
      const int row_span = datum.rowspan;
      datum.colspan = new_span;
        
      // For each row, make sure new columns are masked!
      for (int row = cur_row; row < cur_row + row_span; row++) {
        for (int col = cur_col + old_span; col < cur_col + new_span; col++) {
          Info()->rows[row].data[col].masked = true;
        }
      }
        
      // For each row, make sure former columns are unmasked!
      for (int row = cur_row; row < cur_row + row_span; row++) {
        for (int col = cur_col + new_span; col < cur_col + old_span; col++) {
          Info()->rows[row].data[col].masked = false;
        }
      }
      
      // Redraw the entire table to fix col span information.
      if (IsActive()) Info()->ReplaceHTML();

      return *this;
    }

    // We can control properties of whole columns.
    Table & SetColSpan(int col_id, int new_span) {
      emp_assert(col_id >= 0 && new_span >= 0);
      emp_assert(col_id + new_span <= GetNumCols());

      // If we haven't setup columns at all yet, do so.
      if (Info()->cols.size() == 0) Info()->cols.resize(GetNumCols());
      
      const int old_span = Info()->cols[col_id].GetSpan();
      if (old_span != new_span) {
        for (int i = old_span; i < new_span; i++) { Info()->cols[col_id + i].masked = true; }
        for (int i = new_span; i < old_span; i++) { Info()->cols[col_id + i].masked = false; }
      }
      
      return *this;
    }
    
    Table & SetSpan(int row_span, int col_span) {
      // @CAO Can do this more efficiently, but probably not worth it.
      SetRowSpan(row_span);
      SetColSpan(col_span);
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

  
}
}

#endif
