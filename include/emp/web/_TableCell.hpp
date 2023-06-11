/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018.
 *
 *  @file _TableCell.hpp
 *  @brief The TableCell widget, which behaves like the Table widget, but focuses on a single cell.
 *
 *  DO NOT include this file directly.  All files begining with '_' are for internal use only.
 */

#ifndef EMP_WEB__TABLECELL_HPP_INCLUDE
#define EMP_WEB__TABLECELL_HPP_INCLUDE

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace emp {
namespace web {

  /// An object that focuses on a single cell in a specified table.
  class TableCell : public TableWidget {
  public:
    TableCell(size_t r, size_t c, const std::string & in_id="") : TableWidget(r,c,in_id) { ; }
    TableCell(const TableWidget & in) : TableWidget(in) { ; }
    TableCell(const Widget & in) : TableWidget(in) { ; }
    TableCell(internal::TableInfo * in_info, size_t _row=0, size_t _col=0)
      : TableWidget(in_info, _row, _col) { ; }

    /// Update the CSS for this cell. (override default Table)
    void DoCSS(const std::string & setting, const std::string & value) override {
      Info()->rows[cur_row].data[cur_col].extras.style.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    /// Udpate the attributes for this cell (override default Table)
    void DoAttr(const std::string & setting, const std::string & value) override {
      Info()->rows[cur_row].data[cur_col].extras.attr.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    /// Update a listener for this cell (override default Table)
    void DoListen(const std::string & event_name, size_t fun_id) override {
      Info()->rows[cur_row].data[cur_col].extras.listen.Set(event_name, fun_id);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    TableCell & Clear() { Info()->ClearCell(cur_row, cur_col); return *this; }
    TableCell & ClearStyle() { Info()->rows[cur_row].data[cur_col].extras.style.Clear(); return *this; }
    TableCell & ClearAttr() { Info()->rows[cur_row].data[cur_col].extras.attr.Clear(); return *this; }
    TableCell & ClearListen() { Info()->rows[cur_row].data[cur_col].extras.listen.Clear(); return *this; }
    TableCell & ClearExtras() { Info()->rows[cur_row].data[cur_col].extras.Clear(); return *this; }
    TableCell & ClearChildren() { Info()->ClearCellChildren(cur_row, cur_col); return *this; }
    TableCell & ClearCells() { Info()->ClearCell(cur_row, cur_col); return *this; }

    /// Get the current CSS value for the specified setting of this Cell.
    const std::string & GetCSS(const std::string & setting) const override {
      return Info()->rows[cur_row].data[cur_col].extras.GetStyle(setting);
    }

    TableCell & SetHeader(bool _h=true) {
      Info()->rows[cur_row].data[cur_col].header = _h;
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
      return *this;
    }

    /// Adjust the row span of the current cell.
    TableCell & SetRowSpan(size_t new_span) {
      emp_assert((cur_row + new_span <= GetNumRows()) && "Row span too wide for table!");

      auto & datum = Info()->rows[cur_row].data[cur_col];
      const size_t old_span = datum.rowspan;
      const size_t col_span = datum.colspan;
      datum.rowspan = new_span;

      // For each col, make sure NEW rows are masked!
      for (size_t row = cur_row + old_span; row < cur_row + new_span; row++) {
        for (size_t col = cur_col; col < cur_col + col_span; col++) {
          Info()->rows[row].data[col].masked = true;
        }
      }

      // For each row, make sure former columns are unmasked!
      for (size_t row = cur_row + new_span; row < cur_row + old_span; row++) {
        for (size_t col = cur_col; col < cur_col + col_span; col++) {
          Info()->rows[row].data[col].masked = false;
        }
      }

      // Redraw the entire table to fix row span information.
      if (IsActive()) Info()->ReplaceHTML();

      return *this;
    }

    /// Adjust the column span of the current cell.
    TableCell & SetColSpan(size_t new_span) {
      emp_assert((cur_col + new_span <= GetNumCols()) && "Col span too wide for table!",
                 cur_col, new_span, GetNumCols(), GetID());

      auto & datum = Info()->rows[cur_row].data[cur_col];
      const size_t old_span = datum.colspan;
      const size_t row_span = datum.rowspan;
      datum.colspan = new_span;

      // For each row, make sure new columns are masked!
      for (size_t row = cur_row; row < cur_row + row_span; row++) {
        for (size_t col = cur_col + old_span; col < cur_col + new_span; col++) {
          Info()->rows[row].data[col].masked = true;
        }
      }

      // For each row, make sure former columns are unmasked!
      for (size_t row = cur_row; row < cur_row + row_span; row++) {
        for (size_t col = cur_col + new_span; col < cur_col + old_span; col++) {
          Info()->rows[row].data[col].masked = false;
        }
      }

      // Redraw the entire table to fix col span information.
      if (IsActive()) Info()->ReplaceHTML();

      return *this;
    }

    /// Update both row and column span for this cell.
    TableCell & SetSpan(size_t row_span, size_t col_span) {
      // @CAO Can do this more efficiently, but probably not worth it.
      SetRowSpan(row_span);
      SetColSpan(col_span);
      return *this;
    }

  };
}
}
#endif // DOXYGEN_SHOULD_SKIP_THIS

#endif // #ifndef EMP_WEB__TABLECELL_HPP_INCLUDE
