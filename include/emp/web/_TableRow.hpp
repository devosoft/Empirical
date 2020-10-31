/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018.
 *
 *  @file  _TableRow.hpp
 *  @brief The TableRow widget, which behaves like the Table widget, but focuses on a single row.
 *
 *  DO NOT include this file directly.  All files begining with '_' are for internal use only.
 */

#ifndef EMP_WEB_TABLE_ROW_H
#define EMP_WEB_TABLE_ROW_H

namespace emp {
namespace web {

  /// An object that focuses on a single column in a specified table.
  class TableRow : public TableWidget {
  public:
    TableRow(size_t r, size_t c, const std::string & in_id="") : TableWidget(r,c,in_id) { ; }
    TableRow(const TableWidget & in) : TableWidget(in) { ; }
    TableRow(const Widget & in) : TableWidget(in) { ; }
    TableRow(internal::TableInfo * in_info, size_t _row=0) : TableWidget(in_info, _row, 0) { ; }

    // Apply CSS to appropriate component based on current state.
    void DoCSS(const std::string & setting, const std::string & value) override {
      Info()->rows[cur_row].extras.style.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    void DoAttr(const std::string & setting, const std::string & value) override {
      Info()->rows[cur_row].extras.attr.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    void DoListen(const std::string & event_name, size_t fun_id) override {
      Info()->rows[cur_row].extras.listen.Set(event_name, fun_id);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    bool InStateRow() const { return true; }

    TableRow & Clear() { Info()->ClearRow(cur_row); return *this; }
    TableRow & ClearStyle() { Info()->rows[cur_row].extras.style.Clear(); return *this; }
    TableRow & ClearAttr() { Info()->rows[cur_row].extras.attr.Clear(); return *this; }
    TableRow & ClearListen() { Info()->rows[cur_row].extras.listen.Clear(); return *this; }
    TableRow & ClearExtras() { Info()->rows[cur_row].extras.Clear(); return *this; }
    TableRow & ClearChildren() { Info()->ClearRowChildren(cur_row); return *this; }
    TableRow & ClearCells() { Info()->ClearRow(cur_row); return *this; }

    const std::string & GetCSS(const std::string & setting) const override {
      return Info()->rows[cur_row].extras.GetStyle(setting);
    }

  };

}
}

#endif
