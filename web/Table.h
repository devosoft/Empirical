//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The Table widget
//
//  TableInfo is the core information for a table and has two helper classes:
//  TableRowInfo and TableDataInfo.  The Table class is a smart pointer to a TableInfo
//  object.
//
//  A Table is composed of a series of rows, each with the same number of columns.
//  TableDataInfo may be muliple cells wide/tall, masking other cells.
//
//  Constructors:
//    Table(size_t r, size_t c, const std::string & in_id="")
//      Create a new r-by-c table with an optional DOM id specified.
//    Table(const Widget & in)
//      Point to an existing table (assert that widget IS a table!)
//
//  Accessors:
//    size_t GetNumCols() const
//    size_t GetNumRows() const
//    size_t GetNumCells() const
//      Return associated information about the table size.
//
//    size_t GetCurRow() const { return cur_row; }
//    size_t GetCurCol() const { return cur_col; }
//      Return information about the focal position on the table.
//
//  Adjusting Table size:
//    Table & Rows(size_t r)
//    Table & Cols(size_t c)
//    Table & Resize(size_t r, size_t c)
//      Set the number of rows, columns, or both in the table.
//
//  Setting or identifying the current table state:
//    bool InStateTable() const
//    bool InStateRow() const
//    bool InStateCell() const
//      Return true/false to identify what state the table is currently in.
//
//  Get table widget that affect specified cell, row, etc.
//    TableCell GetCell(size_t r, size_t c)
//    Table GetRow(size_t r)
//    Table GetCol(size_t c)
//    Table GetRowGroup(size_t r)
//    Table GetColGroup(size_t c)
//    Table GetTable()
//
//  Modifying data in table
//    Table & SetHeader(bool _h=true)
//      Set the current cell to be a header (or not if false is passed in)
//    Widget AddText(size_t r, size_t c, const std::string & text)
//      Add text to the specified table cell.
//    Widget AddHeader(size_t r, size_t c, const std::string & text)
//      Add text to the specified table cell AND set the cell to be a header.
//    Table & SetRowSpan(size_t row_span)
//    Table & SetColSpan(size_t col_span)
//    Table & SetSpan(size_t row_span, size_t col_span)
//      Allow the row and/or column span of the current cell to be adjusted.
//    Table & SetSpan(size_t new_span)
//      Set the span of a row group or column group to the value provided.
//
//  Clearing table contents:
//    Table & ClearTable()
//      Clear all style information from table, remove contents from all cells, and
//      shrink table to no rows and no cells.
//    Table & ClearRows()
//      Clear style information from rows and cells and remove contents from cells
//      (but leave table style information and size.)
//    Table & ClearRow(size_t r)
//      Clear style information from the specified row and contents from all cells
//      in that row (but leave other rows untouched).
//    Table & ClearCells()
//      If state is TABLE, clear contents from all cells in entire table.
//      If state is ROW or COL, clear contents from all cells in that row/column.
//      If state is ROW_GROUP or COL_GROUP, clear contents of cells in all rows/cols in group
//      If state is CELL, clear just that single cell.
//    Table & ClearCell(size_t r, size_t c)
//      Clear contents of just the specified cell.
//    Table & Clear()
//      Dynamically clear the entire active state
//
//  Style manipulation
//    std::string GetCSS(const std::string & setting)
//    std::string GetCSS(const std::string & setting, SETTING_TYPE && value)
//      Get or Set the current value of the specified Style setting, based on the state of
//      the table (i.e., TABLE affects full table style, ROW affects active row style, and
//      CELL affects active cell style.)
//    Table & RowCSS(size_t row_id, const std::string & setting, SETTING_TYPE && value)
//    Table & CellCSS(size_t row_id, size_t col_id, const std::string & setting, SETTING_TYPE && value)
//      Set the specified row or cell Style to the value indicated.
//    Table & RowsCSS(const std::string & setting, SETTING_TYPE && value)
//    Table & CellsCSS(const std::string & setting, SETTING_TYPE && value)
//      Set the specified Style setting of all rows or all cells to the value indicated.
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

#include "../base/vector.h"

#include "Slate.h"
#include "Widget.h"
#include "WidgetExtras.h"

namespace emp {
namespace web {

  class TableWidget;
  class Table;
  class TableCell;
  class TableRow;
  class TableCol;
  class TableRowGroup;
  class TableColGroup;

  namespace internal {

    struct TableRowInfo;
    class TableInfo;

    struct TableDataInfo  {
      size_t colspan=1;    // How many columns wide is this TableData?
      size_t rowspan=1;    // How many rows deep is this TableData?
      bool header=false;   // Is this TableData a header (<th> vs <td>)?
      bool masked=false;   // Is this cell masked by another cell?
      WidgetExtras extras; // Extra annotations (attributes, style, listeners)

      emp::vector<Widget> children;  // Widgets contained in this cell.

      bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
        bool ok = true;
        if (verbose) ss << prefix << "Scanning: emp::TableDataInfo" << std::endl;
        if (masked) { ss << "Warning: Masked cell may have contents!" << std::endl; ok = false; }
        return ok;
      }
    };  // END: TableDataInfo


    struct TableRowInfo {
      emp::vector<TableDataInfo> data;  // detail object for each cell in this row.
      WidgetExtras extras; // Extra annotations (attributes, style, listeners)

      // Apply to all cells in row.
      template <typename SETTING_TYPE>
      TableRowInfo & CellsCSS(const std::string & setting, SETTING_TYPE && value) {
        for (auto & datum : data) datum.extras.style.Set(setting, value);
        return *this;
      }

      // Apply to specific cell in row.
      template <typename SETTING_TYPE>
      TableRowInfo & CellCSS(size_t col_id, const std::string & setting, SETTING_TYPE && value) {
        data[col_id].extras.style.Set(setting, value);
        return *this;
      }

      bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
        bool ok = true;
        if (verbose) { ss << prefix << "Scanning: emp::TableRowInfo" << std::endl; }
        for (auto & cell : data) ok = ok && cell.OK(ss, verbose, prefix+"  ");
        return ok;
      }
    };

    struct TableColInfo { WidgetExtras extras; };  // Currently only need annotations.

    // Group of rows or columns...
    struct TableGroupInfo : public WidgetExtras {
      size_t span = 1;       // How many rows/columns does this group represent?
      bool masked = false;   // Is the current group masked because of a previous span?
      WidgetExtras extras;   // Extra annotations (attributes, style, listeners)
    };

    class TableInfo : public internal::WidgetInfo {
      friend TableWidget; friend Table; friend TableCell; friend TableRow; friend TableCol;
      friend TableRowGroup; friend TableColGroup;
    protected:
      size_t row_count;                        // How big is this table?
      size_t col_count;
      emp::vector<TableRowInfo> rows;          // Detail object for each row
      emp::vector<TableColInfo> cols;          // Detail object for each column (if needed)
      emp::vector<TableGroupInfo> col_groups;  // Detail object for each column group (if needed)
      emp::vector<TableGroupInfo> row_groups;  // Detail object for each row group (if needed)

      size_t append_row;               // Which row is triggering an append?
      size_t append_col;               // Which col is triggering an append?

      TableInfo(const std::string & in_id="")
        : internal::WidgetInfo(in_id), row_count(0), col_count(0), append_row(0), append_col(0) { ; }
      TableInfo(const TableInfo &) = delete;               // No copies of INFO allowed
      TableInfo & operator=(const TableInfo &) = delete;   // No copies of INFO allowed
      virtual ~TableInfo() { ; }

      std::string TypeName() const override { return "TableInfo"; }
      virtual bool IsTableInfo() const override { return true; }

      void Resize(size_t new_rows, size_t new_cols) {
        // Resize preexisting rows if remaining
        if (new_cols != col_count) {
          for (size_t r = 0; r < rows.size() && r < new_rows; r++) {
            rows[r].data.resize(new_cols);
            for (size_t c = col_count; c < new_cols; c++) { AddChild(r, c, Text("")); }
          }
          col_count = new_cols;                    // Store the new column count

          // Resize extra column info, only if currently in use.
          if (cols.size()) cols.resize(new_cols);
          if (col_groups.size()) col_groups.resize(new_cols);
        }

        // Resize number of rows.
        if (new_rows != row_count) {
          rows.resize(new_rows);
          for (size_t r = row_count; r < new_rows; r++) {
            rows[r].data.resize(col_count);
            for (size_t c = 0; c < col_count; c++) { AddChild(r, c, Text("")); }
          }
          row_count = new_rows;

          // Resize extra row group info, only if needed.
          if (row_groups.size()) row_groups.resize(new_rows);
        }

      }

      void DoActivate(bool top_level=true) override {
        // Activate all of the cell children.
        for (size_t r = 0; r < row_count; r++) {
          for (size_t c = 0; c < col_count; c++) {
            for (auto & child : rows[r].data[c].children) child->DoActivate(false);
          }
        }

        // Activate this Table.
        internal::WidgetInfo::DoActivate(top_level);
      }


      // Return a text element for appending into a specific cell (use existing one or build new)
      web::Text & GetTextWidget(size_t r, size_t c) {
        auto & cell_children = rows[r].data[c].children;
        // If final element in this cell doesn't exists, isn't text, or can't append, but new Text!
        if (cell_children.size() == 0
            || cell_children.back().IsText() == false
            || cell_children.back().AppendOK() == false)  {
          AddChild(Text());
        }
        return (Text &) cell_children.back();
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
      void AddChild(size_t r, size_t c, Widget in) {
        emp_assert(in->parent == nullptr && "Cannot insert widget if already has parent!", in->id);
        emp_assert(in->state != Widget::ACTIVE && "Cannot insert a stand-alone active widget!");

        // Setup parent-child relationship in the specified cell.
        rows[r].data[c].children.emplace_back(in);
        in->parent = this;
        Register(in);

        // If this element (as new parent) is active, anchor widget and activate it!
        if (state == Widget::ACTIVE) {
          // Create a span tag to anchor the new widget.
          std::string cell_id = emp::to_string(id, '_', r, '_', c);
          EM_ASM_ARGS({
              parent_id = Pointer_stringify($0);
              child_id = Pointer_stringify($1);
              $('#' + parent_id).append('<span id="' + child_id + '"></span>');
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
        for (size_t r = 0; r < row_count; r++) {
          for (size_t c = 0; c < col_count; c++) {
            for (Widget & child : rows[r].data[c].children) regestrar->Register(child);
          }
        }
      }

      void UnregisterChildren(internal::SlateInfo * regestrar) override {
        for (size_t r = 0; r < row_count; r++) {
          for (size_t c = 0; c < col_count; c++) {
            for (Widget & child : rows[r].data[c].children) regestrar->Unregister(child);
          }
        }
      }

      virtual void GetHTML(std::stringstream & HTML) override {
        emp_assert(cols.size() == 0 || cols.size() == col_count);
        emp_assert(col_groups.size() == 0 || col_groups.size() == col_count);

        HTML.str("");                                           // Clear the current text.
        HTML << "<table id=\"" << id << "\">";

        // Include column/row details only as needed.
        const bool use_colg = col_groups.size();
        const bool use_cols = cols.size();
        const bool use_rowg = row_groups.size();

        if (use_colg || use_cols) {
          for (size_t c = 0; c < col_count; ++c) {
            if (use_colg && col_groups[c].masked == false) {
              HTML << "<colgroup";
              if (col_groups[c].extras) HTML << " id=" << id << "_cg" << c;
              HTML << ">";
            }
            HTML << "<col";
            if (use_cols && cols[c].extras) HTML << " id=" << id << "_c" << c;
            HTML << ">";
          }
        }

        // Loop through all of the rows in the table.
        for (size_t r = 0; r < rows.size(); r++) {
          if (use_rowg && row_groups[r].masked == false) {
            HTML << "<tbody";
            if (row_groups[r].extras) HTML << " id=" << id << "_rg" << r;
            HTML << ">";
          }

          auto & row = rows[r];
          HTML << "<tr";
          if (row.extras) HTML << " id=" << id << '_' << r;
          HTML << ">";

          // Loop through each cell in this row.
          for (size_t c = 0; c < row.data.size(); c++) {
            auto & datum = row.data[c];
            if (datum.masked) continue;  // If this cell is masked by another, skip it!

            // Print opening tag.
            HTML << (datum.header ? "<th" : "<td");

            // Include an id for this cell if we have one.
            if (datum.extras) HTML << " id=" << id << '_' << r << '_' << c;

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

      void ClearCellChildren(size_t row_id, size_t col_id) {
        // Clear out this cell's children.   @CAO: Keep a starting text widget if we can?
        auto & datum = rows[row_id].data[col_id];
        if (parent) for (Widget & child : datum.children) parent->Unregister(child);
        datum.children.resize(0);
      }
      void ClearRowChildren(size_t row_id) {
        for (size_t col_id = 0; col_id < col_count; col_id++) ClearCellChildren(row_id, col_id);
      }
      void ClearColChildren(size_t col_id) {
        for (size_t row_id = 0; row_id < row_count; row_id++) ClearCellChildren(row_id, col_id);
      }
      void ClearRowGroupChildren(size_t row_id) {
        for (size_t offset=0; offset < row_groups[row_id].span; offset++) {
          ClearRowChildren(row_id+offset);
        }
      }
      void ClearColGroupChildren(size_t col_id) {
        for (size_t offset=0; offset < col_groups[col_id].span; offset++) {
          ClearColChildren(col_id+offset);
        }
      }
      void ClearTableChildren() {
        for (size_t col_id = 0; col_id < col_count; col_id++) {
          for (size_t row_id = 0; row_id < row_count; row_id++) {
            ClearCellChildren(row_id, col_id);
          }
        }
      }

      void ClearCell(size_t row_id, size_t col_id) {
        auto & datum = rows[row_id].data[col_id];
        datum.colspan = 1;
        datum.rowspan = 1;
        datum.header = false;
        datum.masked = false;  // @CAO Technically, cell might still be masked!
        datum.extras.Clear();

        ClearCellChildren(row_id, col_id);
      }
      void ClearRowCells(size_t row_id) {
        for (size_t col_id = 0; col_id < col_count; col_id++) ClearCell(row_id, col_id);
      }
      void ClearColCells(size_t col_id) {
        for (size_t row_id = 0; row_id < row_count; row_id++) ClearCell(row_id, col_id);
      }
      void ClearRow(size_t row_id) {
        rows[row_id].extras.Clear();
        ClearRowCells(row_id);
      }
      void ClearCol(size_t col_id) {
        cols[col_id].extras.Clear();
        ClearColCells(col_id);
      }
      void ClearRowGroup(size_t row_id) {
        row_groups[row_id].extras.Clear();
        for (size_t offset=0; offset < row_groups[row_id].span; offset++) ClearRow(row_id+offset);
      }
      void ClearColGroup(size_t col_id) {
        col_groups[col_id].extras.Clear();
        for (size_t offset=0; offset < col_groups[col_id].span; offset++) ClearCol(col_id+offset);
      }

      void ClearTableCells() { for (size_t r = 0; r < row_count; r++) ClearRowCells(r); }
      void ClearTableRows() { for (size_t r = 0; r < row_count; r++) ClearRow(r); }
      void ClearTable() { extras.Clear(); Resize(0,0); }

      bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
        bool ok = true;

        // Basic info
        if (verbose) {
          ss << prefix << "Scanning: emp::TableInfo (rows=" << row_count
             << ", cols=" << col_count << ")." << std::endl;
        }

        // Make sure rows and columns are being sized correctly.
        if (row_count != rows.size()) {
          ss << prefix << "Error: row_count = " << row_count
             << ", but rows has " << rows.size() << " elements." << std::endl;
          ok = false;
        }

        if (cols.size() && col_count != cols.size()) {
          ss << prefix << "Error: col_count = " << col_count
             << ", but cols has " << cols.size() << " elements." << std::endl;
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

        // And perform the same test for row/column groups.
        if (col_groups.size() && col_count != col_groups.size()) {
          ss << prefix << "Error: col_count = " << col_count
             << ", but col_groups has " << col_groups.size() << " elements." << std::endl;
          ok = false;
        }

        if (row_groups.size() && row_count != row_groups.size()) {
          ss << prefix << "Error: row_count = " << row_count
             << ", but row_groups has " << row_groups.size() << " elements." << std::endl;
          ok = false;
        }

        // Recursively call OK on rows and data.
        for (size_t r = 0; r < row_count; r++) {
          ok = ok && rows[r].OK(ss, verbose, prefix+"  ");
          if (col_count != rows[r].data.size()) {
            ss << prefix << "  Error: col_count = " << col_count
               << ", but row has " << rows[r].data.size() << " elements." << std::endl;
            ok = false;
          }
          for (size_t c = 0; c < col_count; c++) {
            auto & cell = rows[r].data[c];
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
        emp_assert(cols.size() == 0 || cols.size() == col_count);
        emp_assert(col_groups.size() == 0 || col_groups.size() == col_count);
        emp_assert(row_groups.size() == 0 || row_groups.size() == row_count);

        // Replace Table's HTML...
        internal::WidgetInfo::ReplaceHTML();

        // Then replace cells
        for (size_t r = 0; r < row_count; r++) {
          rows[r].extras.Apply(emp::to_string(id, '_', r));
          for (size_t c = 0; c < col_count; c++) {
            auto & datum = rows[r].data[c];
            if (datum.masked) continue;  // If this cell is masked by another, skip it!
            datum.extras.Apply(emp::to_string(id, '_', r, '_', c));

            // If this widget is active, immediately replace children.
            if (state == Widget::ACTIVE) {
              for (auto & child : datum.children) child->ReplaceHTML();
            }
          }
        }

        // And setup columns, column groups, and row groups, as needed.
        if (cols.size()) {
          for (size_t c = 0; c < col_count; c++) {
            if (!cols[c].extras) continue;
            cols[c].extras.Apply(emp::to_string(id, "_c", c));
          }
        }
        if (col_groups.size()) {
          for (size_t c = 0; c < col_count; c++) {
            if (col_groups[c].masked || !col_groups[c].extras) continue;
            col_groups[c].extras.Apply(emp::to_string(id, "_cg", c));
          }
        }
        if (row_groups.size()) {
          for (size_t c = 0; c < col_count; c++) {
            if (row_groups[c].masked || !row_groups[c].extras) continue;
            row_groups[c].extras.Apply(emp::to_string(id, "_rg", c));
          }
        }
      }

    public:
      virtual std::string GetType() override { return "web::TableInfo"; }
    }; // end TableInfo


  } // end namespace internal

  class TableWidget : public internal::WidgetFacet<TableWidget> {
    friend class internal::TableInfo;
  protected:
    size_t cur_row;      // Which row/col is currently active?
    size_t cur_col;

    using parent_t = internal::WidgetFacet<TableWidget>;

    // Get a properly cast version of indo.
    internal::TableInfo * Info() { return (internal::TableInfo *) info; }
    const internal::TableInfo * Info() const { return (internal::TableInfo *) info; }

    TableWidget(internal::TableInfo * in_info, size_t _row=0, size_t _col=0)
     : WidgetFacet(in_info), cur_row(_row), cur_col(_col) { ; }

    // Apply CSS to appropriate component based on current state.
    void DoCSS(const std::string & setting, const std::string & value) override {
      parent_t::DoCSS(setting, value);
    }

    // Apply CSS to appropriate component based on current state.
    void DoAttr(const std::string & setting, const std::string & value) override {
      parent_t::DoAttr(setting, value);
    }

    // Apply CSS to appropriate component based on current state.
    void DoListen(const std::string & event_name, size_t fun_id) override {
      parent_t::DoListen(event_name, fun_id);
    }

  public:
    TableWidget(size_t r, size_t c, const std::string & in_id="")
      : WidgetFacet(in_id), cur_row(0), cur_col(0)
    {
      emp_assert(r > 0 && c > 0);              // Ensure that we have rows and columns!
      info = new internal::TableInfo(in_id);
      Info()->Resize(r, c);
    }
    TableWidget(const TableWidget & in)
      : WidgetFacet(in), cur_row(in.cur_row), cur_col(in.cur_col) { ; }
    TableWidget(const Widget & in) : WidgetFacet(in), cur_row(0), cur_col(0) {
      emp_assert(info->IsTableInfo());
    }
    virtual ~TableWidget() { ; }

    using INFO_TYPE = internal::TableInfo;

    size_t GetNumCols() const { return Info()->col_count; }
    size_t GetNumRows() const { return Info()->row_count; }
    size_t GetNumCells() const { return Info()->col_count*Info()->row_count; }

    // Called before an append.
    virtual void PrepareAppend() override {
      Info()->append_row = cur_row;
      Info()->append_col = cur_col;
    }

    size_t GetCurRow() const { return cur_row; }
    size_t GetCurCol() const { return cur_col; }

    // Can clear anything from any widget, if properly specified.
    // Specialized widgets should define Clear(), ClearChildren(), ClearStyle(), ClearAttr(),
    // and ClearListen() for that table component type.
    void ClearTable() { Info()->ClearTable(); }
    void ClearRows() { Info()->ClearTableRows(); }
    void ClearRow(size_t r) { Info()->ClearRow(r); }
    void ClearCol(size_t c) { Info()->ClearCol(c); }
    void ClearRowGroup(size_t r) { Info()->ClearRowGroup(r); }
    void ClearColGroup(size_t c) { Info()->ClearColGroup(c); }
    void ClearCells() { Info()->ClearTableCells(); }
    void ClearCell(size_t r, size_t c) { Info()->ClearCell(r, c); }

    TableCell GetCell(size_t r, size_t c);
    TableRow GetRow(size_t r);
    TableCol GetCol(size_t c);
    TableRowGroup GetRowGroup(size_t r);
    TableColGroup GetColGroup(size_t c);
    Table GetTable();

    web::Text GetTextWidget() { return Info()->GetTextWidget(); }

    Widget AddText(size_t r, size_t c, const std::string & text);
    Widget AddHeader(size_t r, size_t c, const std::string & text);

    using parent_t::SetCSS;
    std::string GetCSS(const std::string & setting) override {
      return Info()->extras.GetStyle(setting);
    }

    virtual bool OK(std::stringstream & ss, bool verbose=false, const std::string & prefix="") {
      bool ok = true;

      // Basic info
      if (verbose) {
        ss << prefix << "Scanning: emp::Table (rows=" << Info()->row_count
           << ", cols=" << Info()->col_count << ")." << std::endl;
      }

      // Make sure current row and col are valid.
      if (cur_row >= Info()->row_count) {
        ss << prefix << "Error: cur_row = " << cur_row << "." << std::endl;
        ok = false;
      }

      if (cur_col >= Info()->col_count) {
        ss << prefix << "Error: cur_col = " << cur_col << "." << std::endl;
        ok = false;
      }

      // Make sure internal info is okay.
      ok = ok && Info()->OK(ss, verbose, prefix+"  ");

      return ok;
    }
  };

  class Table : public TableWidget {
  public:
    Table(size_t r, size_t c, const std::string & in_id="") : TableWidget(r,c,in_id) { ; }
    Table(const TableWidget & in) : TableWidget(in) { ; }
    Table(const Widget & in) : TableWidget(in) { ; }
    Table(internal::TableInfo * in_info, size_t _row, size_t _col)
      : TableWidget(in_info, _row, _col) { ; }

    Table & Clear() { Info()->ClearTable(); return *this; }
    Table & ClearStyle() { Info()->extras.style.Clear(); return *this; }
    Table & ClearAttr() { Info()->extras.attr.Clear(); return *this; }
    Table & ClearListen() { Info()->extras.listen.Clear(); return *this; }
    Table & ClearExtras() { Info()->extras.Clear(); return *this; }
    Table & ClearChildren() { Info()->ClearTableChildren(); return *this; }

    // Functions to resize the number of rows, columns, or both!
    Table & Rows(size_t r) {
      Info()->Resize(r, Info()->col_count);
      if (cur_row >= r) cur_row = 0;
      return *this;
    }
    Table & Cols(size_t c) {
      Info()->Resize(Info()->row_count, c);
      if (cur_col >= c) cur_col = 0;
      return *this;
    }
    Table & Resize(size_t r, size_t c) {
      Info()->Resize(r, c);
      if (cur_row >= r) cur_row = 0;
      if (cur_col >= c) cur_col = 0;
      return *this;
    }

    Table & SetColSpan(size_t new_span) {
      emp_assert((cur_col + new_span <= GetNumCols()) && "Col span too wide for table!",
                 cur_col, new_span, GetNumCols(), GetID());

      // If we haven't setup columns at all yet, do so.
      if (Info()->col_groups.size() == 0) Info()->col_groups.resize(GetNumCols());

      const size_t old_span = Info()->col_groups[cur_col].span;
      Info()->col_groups[cur_col].span = new_span;

      if (old_span != new_span) {
        for (size_t i=old_span; i<new_span; i++) { Info()->col_groups[cur_col+i].masked = true; }
        for (size_t i=new_span; i<old_span; i++) { Info()->col_groups[cur_col+i].masked = false; }
      }

      // Redraw the entire table to fix col span information.
      if (IsActive()) Info()->ReplaceHTML();

      return *this;
    }

    // Apply to target row.
    template <typename SETTING_TYPE>
    Table & RowCSS(size_t row_id, const std::string & setting, SETTING_TYPE && value) {
      emp_assert(row_id >= 0 && row_id < Info()->row_count);
      Info()->rows[row_id].extras.style.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace row's CSS
      return *this;
    }

    // Apply to target cell.
    template <typename SETTING_TYPE>
    Table & CellCSS(size_t row_id, size_t col_id, const std::string & setting, SETTING_TYPE && value) {
      emp_assert(row_id >= 0 && row_id < Info()->row_count);
      emp_assert(col_id >= 0 && col_id < Info()->row_count);
      Info()->rows[row_id].extras.style.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
      return *this;
    }

    // Apply to all rows.  (@CAO: Should we use fancier jquery here?)
    template <typename SETTING_TYPE>
    Table & RowsCSS(const std::string & setting, SETTING_TYPE && value) {
      for (auto & row : Info()->rows) row.extras.style.Set(setting, emp::to_string(value));
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
  };

  #include "_TableCell.h"
  #include "_TableRow.h"
  #include "_TableCol.h"
  #include "_TableRowGroup.h"
  #include "_TableColGroup.h"

  // Fill out members of Table that require extra classes...

  TableCell TableWidget::GetCell(size_t r, size_t c) {
    emp_assert(Info() != nullptr);
    emp_assert(r < Info()->row_count && c < Info()->col_count,
               r, c, Info()->row_count, Info()->col_count, GetID());
    return TableCell(Info(), r, c);
  }

  TableRow TableWidget::GetRow(size_t r) {
    emp_assert(r < Info()->row_count, r, Info()->row_count, GetID());
    return TableRow(Info(), r);
  }

  TableCol TableWidget::GetCol(size_t c) {
    emp_assert(c < Info()->col_count, c, Info()->col_count, GetID());
    return TableCol(Info(), c);
  }

  TableRowGroup TableWidget::GetRowGroup(size_t r) {
    emp_assert(r < Info()->row_count, r, Info()->row_count, GetID());
    return TableRowGroup(Info(), r);
  }

  TableColGroup TableWidget::GetColGroup(size_t c) {
    emp_assert(c < Info()->col_count, c, Info()->col_count, GetID());
    return TableColGroup(Info(), c);
  }

  Table TableWidget::GetTable() {
    return Table(Info(), cur_row, cur_col);
  }

  Widget TableWidget::AddText(size_t r, size_t c, const std::string & text) {
    GetCell(r,c) << text;
    return *this;
  }

  Widget TableWidget::AddHeader(size_t r, size_t c, const std::string & text) {
    TableCell cell = GetCell(r,c);
    cell << text;
    cell.SetHeader();
    return *this;
  }

}
}

#endif
