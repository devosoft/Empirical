/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018.
 *
 *  @file _TableRowGroup.hpp
 *  @brief The TableRowGoup widget, which behaves like the Table widget, but focuses on a group of rows.
 *
 *  DO NOT include this file directly.  All files begining with '_' are for internal use only.
 */

#ifndef EMP_WEB__TABLEROWGROUP_HPP_INCLUDE
#define EMP_WEB__TABLEROWGROUP_HPP_INCLUDE

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace emp {
namespace web {

  /// An object that focuses on a group of rows in a specified table.
  class TableRowGroup : public TableWidget {
  public:
    TableRowGroup(size_t r, size_t c, const std::string & in_id="") : TableWidget(r,c,in_id) { ; }
    TableRowGroup(const TableWidget & in) : TableWidget(in) { ; }
    TableRowGroup(const Widget & in) : TableWidget(in) { ; }
    TableRowGroup(internal::TableInfo * in_info, size_t _row=0) : TableWidget(in_info, _row, 0) { ; }

    // Apply CSS to appropriate component based on current state.
    void DoCSS(const std::string & setting, const std::string & value) override {
      if (Info()->row_groups.size() == 0) Info()->row_groups.resize(GetNumRows());
      Info()->row_groups[cur_row].extras.style.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    void DoAttr(const std::string & setting, const std::string & value) override {
      if (Info()->row_groups.size() == 0) Info()->row_groups.resize(GetNumRows());
      Info()->row_groups[cur_row].extras.attr.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    void DoListen(const std::string & event_name, size_t fun_id) override {
      if (Info()->row_groups.size() == 0) Info()->row_groups.resize(GetNumRows());
      Info()->row_groups[cur_row].extras.listen.Set(event_name, fun_id);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    TableRowGroup & Clear() { Info()->ClearRowGroup(cur_row); return *this; }
    TableRowGroup & ClearStyle() { Info()->row_groups[cur_row].extras.style.Clear(); return *this; }
    TableRowGroup & ClearAttr() { Info()->row_groups[cur_row].extras.attr.Clear(); return *this; }
    TableRowGroup & ClearListen() { Info()->row_groups[cur_row].extras.listen.Clear(); return *this; }
    TableRowGroup & ClearExtras() { Info()->row_groups[cur_row].extras.Clear(); return *this; }
    TableRowGroup & ClearChildren() { Info()->ClearRowGroupChildren(cur_row); return *this; }
    TableRowGroup & ClearCells() { Info()->ClearRowGroup(cur_row); return *this; }

    const std::string & GetCSS(const std::string & setting) const override {
      return Info()->row_groups[cur_row].extras.GetStyle(setting);
    }

    TableRowGroup & SetRowSpan(size_t new_span) {
      emp_assert((cur_row + new_span <= GetNumRows()) && "Row span too wide for table!");

      // If we haven't setup rows at all yet, do so.
      if (Info()->row_groups.size() == 0) Info()->row_groups.resize(GetNumRows());

      const size_t old_span = Info()->row_groups[cur_row].span;
      Info()->row_groups[cur_row].span = new_span;

      if (old_span != new_span) {
        for (size_t i=old_span; i<new_span; i++) { Info()->row_groups[cur_row+i].masked = true; }
        for (size_t i=new_span; i<old_span; i++) { Info()->row_groups[cur_row+i].masked = false; }
      }

      // Redraw the entire table to fix row span information.
      if (IsActive()) Info()->ReplaceHTML();

      return *this;
    }

    TableRowGroup SetSpan(size_t new_span) { SetRowSpan(new_span); return *this; }
  };

}
}

#endif // DOXYGEN_SHOULD_SKIP_THIS

#endif // #ifndef EMP_WEB__TABLEROWGROUP_HPP_INCLUDE
