#ifndef EMP_WEB_TABLE_H
#define EMP_WEB_TABLE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Table widget
//
//  TableInfo has two helper classes: TableRow and TableData.
//
//  A Table is composed of row x col cells
//  TableData is represented in its upper-left most cell, but may be muliple cells wide/tall.
//  Cells representing TableData are "Control Cells"; other cells are "Helper Cells".
//


#include "../tools/vector.h"

#include "Slate.h"
#include "Widget.h"

namespace emp {
namespace web {

  class Table : public internal::WidgetFacet<Table> {
    friend class TableInfo;
  protected:

  class TableRow;
  class TableInfo;

  class TableData {
    friend TableRow; friend Table;
  protected:
    int colspan;    // How many columns wide is this TableData?
    int rowspan;    // How many rows deep is this TableData?
    int child_id;   // Which child element is this cell associated with?
    bool header;    // Is this TableData a header (<th> vs <td>)?
    bool masked;    // Is this cell masked by another cell?
    Style style;    // CSS Style

  public:
    TableData() : colspan(1), rowspan(1), child_id(-1)
                , header(false), masked(false) { ; }
    ~TableData() { ; }

    int GetColSpan() const { return colspan; }
    int GetRowSpan() const { return rowspan; }
    int GetChildID() const { return child_id; }
    bool HasSlate() const { return child_id >= 0; }
    bool IsHeader() const { return header; }
    bool IsMasked() const { return masked; }
    Style & GetStyle() { return style; }
    const Style & GetStyle() const { return style; }
    explicit operator bool() { return !masked; }      // Unmasked cell = true, masked = false.
    
    void SetColSpan(int cs) { colspan = cs; }
    void SetRowSpan(int rs) { rowspan = rs; }
    void SetChildID(int cid) { child_id = cid; }
    void SetHeader(bool h=true) { header = h; }
    void SetMasked(bool m=true) { masked = m; }
    
    bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
      bool ok = true;
      if (verbose) ss << prefix << "Scanning: emp::TableData; child_id=" << child_id << std::endl;

      if (child_id >= 0 && masked == true) {
        ss << "Warning: Masked cell has contents!" << std::endl;
        ok = false;
      }
      
      return ok;
    }
    
  };
    
  class TableRow {
    friend Table; friend TableInfo;
  protected:
    emp::vector<TableData> data;  // detail object for each cell in this row.
    Style style;

  public:
    TableRow() { ; }
    ~TableRow() { ; }

    int GetSize() const { return (int) data.size(); }                 // How many cells in this row?
    TableData & operator[](int id) { return data[id]; }               // Get a single cell
    const TableData & operator[](int id) const { return data[id]; }   // Get a single const cell
    emp::vector<TableData> & GetCells() { return data; }              // Get ALL cells

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

    // Get a slate associated with the current cell (and build one if we need to...)
    Widget & GetCurSlate();

    // Add additional children on to this element.
    Widget Append(Widget info) override { return GetCurSlate() << info; }
    Widget Append(const std::string & text) override { return GetCurSlate() << text; }
    Widget Append(const std::function<std::string()> & in_fun) override {
      return GetCurSlate() << in_fun;
    }

    virtual void GetHTML(std::stringstream & HTML) override {
      HTML.str("");                                           // Clear the current text.
      HTML << "<table id=\"" << id << "\">";

      // Loop through all of the rows in the table.
      for (auto & row : rows) {
        HTML << "<tr>";

        // Loop through each cell in this row.
        for (auto & datum : row.GetCells()) {
          if (datum.IsMasked()) continue;  // If this cell is masked by another, skip it!

          // Print opening tag.
          HTML << (datum.IsHeader() ? "<th" : "<td");

          // If this cell spans multiple rows or columns, indicate!
          if (datum.GetColSpan() > 1) HTML << " colspan=\"" << datum.GetColSpan() << "\"";
          if (datum.GetRowSpan() > 1) HTML << " rowspan=\"" << datum.GetRowSpan() << "\"";
          
          HTML << ">";
          
          // If this cell has contents, initialize them!
          if (datum.HasSlate()) {
            Widget & widget = children[datum.GetChildID()];
            HTML << "<span id=\"" << widget.GetID() << "\"></span>\n";
          }

          // Print closing tag.
          HTML << (datum.IsHeader() ? "</th>" : "</td>");
        }

        HTML << "</tr>";
      }
      
      HTML << "</table>";
    }

    void UpdateRows(int r) {
      if (row_count != r) {                             // Update only if we're making a change.
        rows.resize(r);                                 // Resize rows.
        for (int i = row_count; i < r; i++) {
          rows[i].SetCols(col_count);                   // Initialize new rows.
        }
        row_count = r;                                  // Store new size.
        if (state == Widget::ACTIVE) ReplaceHTML();     // If active, update screen!
      }
    }
    void UpdateCols(int c) {
      if (col_count != c) {                               // Update only if we're making a change.
        col_count = c;                                    // Store new size.
        for (auto & row : rows) row.SetCols(col_count);   // Make sure all rows have new col_count
        if (state == Widget::ACTIVE) ReplaceHTML();       // If active, update screen!
      }
    }

      
  public:
    virtual std::string GetType() override { return "web::TableInfo"; }
  };


    int cur_row;      // Which row/col is currently active?
    int cur_col;
           
    // A table's state determines how some operations work.
    enum state_t { TABLE, ROW, CELL };
    state_t state;


    // Get a properly cast version of indo.  
    TableInfo * Info() { return (TableInfo *) info; }
    const TableInfo * Info() const { return (TableInfo *) info; }
 
    Table(TableInfo * in_info) : WidgetFacet(in_info) { ; }

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

      info = new TableInfo(in_id);
    
      Info()->row_count = r;
      Info()->col_count = c;
      Info()->rows.resize(r);
      for (auto & row : Info()->rows) row.SetCols(c);  // Set all rows to correct # of columns
    }
    Table(const Table & in)
      : WidgetFacet(in), cur_row(in.cur_row), cur_col(in.cur_col), state(in.state) { ; }
    Table(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsTableInfo()); }
    virtual ~Table() { ; }

    using INFO_TYPE = TableInfo;

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
    

    Table & Rows(int r) {
      Info()->UpdateRows(r);
      if (cur_row >= r) cur_row = 0;
      return *this;
    }
    Table & Cols(int c) {
      Info()->UpdateCols(c);
      if (cur_col >= c) cur_col = 0;
      return *this;
    }
      
    Table & GetCell(int r, int c) {
      emp_assert(r < Info()->row_count && c < Info()->col_count);
      cur_row = r; cur_col = c;
      state = CELL;
      return *this;
    }
    Table & GetRow(int r) {
      emp_assert(r < Info()->row_count);
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
      emp_assert((cur_col + new_span <= GetNumCols()) && "Col span too wide for table!");
        
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
      if (verbose) {
        ss << prefix << "Scanning: emp::Table (rows=" << Info()->row_count
           << ", cols=" << Info()->col_count << ")." << std::endl;
      }
      
      if (Info()->row_count != (int) Info()->rows.size()) {
        ss << prefix << "Error: row_count = " << Info()->row_count
           << ", but rows has " << Info()->rows.size() << " elements." << std::endl;
        ok = false;
      }
      
      if (Info()->row_count < 1) {
        ss << prefix << "Error: Cannot have " << Info()->row_count
           << " rows in table." << std::endl;
        ok = false;
      }

      if (Info()->col_count < 1) {
        ss << prefix << "Error: Cannot have " << Info()->col_count
           << " cols in table." << std::endl;
        ok = false;
      }

      if (cur_row < 0 || cur_row >= Info()->row_count) {
        ss << prefix << "Error: cur_row = " << cur_row << "." << std::endl;
        ok = false;
      }
        
      if (cur_col < 0 || cur_col >= Info()->col_count) {
        ss << prefix << "Error: cur_col = " << cur_col << "." << std::endl;
        ok = false;
      }
      
      // Recursively call OK on rows and data.
      // @CAO Move to OK in TableInfo?
      for (int r = 0; r < Info()->row_count; r++) {
        ok = ok && Info()->rows[r].OK(ss, verbose, prefix+"  ");
        if (Info()->col_count != Info()->rows[r].GetSize()) {
          ss << prefix << "  Error: col_count = " << Info()->col_count
             << ", but row has " << Info()->rows[r].GetSize() << " elements." << std::endl;
          ok = false;
        }
        for (int c = 0; c < Info()->col_count; c++) {
          auto & cell = Info()->rows[r][c];
          if (c + cell.GetColSpan() > Info()->col_count) {
            ss << prefix << "  Error: Cell at row " << r << ", col " << c
               << " extends past right side of table." << std::endl;
            ok = false;
          }
          if (r + cell.GetRowSpan() > Info()->row_count) {
            ss << prefix << "  Error: Cell at row " << r << ", col " << c
               << " extends past bottom of table." << std::endl;
            ok = false;
          }
        }
      }


      return ok;
    }
  };

  // Setup mechanism to retrieve current slate for table append.
  Widget & Table::TableInfo::GetCurSlate() {
    int cur_col = append_widget->cur_row;
    int cur_row = append_widget->cur_row;

    // Make sure the number of rows hasn't changed, making the current position illegal.
    if (cur_col >= col_count) cur_col = 0;
    if (cur_row >= row_count) cur_row = 0;

    auto & cur_cell = rows[cur_row].data[cur_col];
    
    // If the current cell does not have a slate, generate one now.
    if (!cur_cell.HasSlate()) {
      cur_cell.SetChildID(children.size());
      AddChild( Slate("") );
    }
    
    // Return the element, now that we know we have it.
    return children.back();
  }
  
}
}

#endif
