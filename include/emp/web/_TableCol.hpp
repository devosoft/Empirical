/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018.
 *
 *  @file  _TableCol.hpp
 *  @brief The TableCol widget, which behaves like the Table widget, but focuses on a single column.
 *
 *  DO NOT include this file directly.  All files begining with '_' are for internal use only.
 */

#ifndef EMP_WEB_TABLE_COL_H
#define EMP_WEB_TABLE_COL_H

namespace emp {
namespace web {

  /// An object that focuses on a single column in a specified table.
  class TableCol : public TableWidget {
  public:
    TableCol(size_t r, size_t c, const std::string & in_id="") : TableWidget(r,c,in_id) { ; }
    TableCol(const TableWidget & in) : TableWidget(in) { ; }
    TableCol(const Widget & in) : TableWidget(in) { ; }
    TableCol(internal::TableInfo * in_info, size_t _col=0) : TableWidget(in_info, 0, _col) { ; }

    /// Apply CSS to appropriate column based on current state.
    void DoCSS(const std::string & setting, const std::string & value) override {
      if (Info()->cols.size() == 0) Info()->cols.resize(GetNumCols());
      Info()->cols[cur_col].extras.style.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    void DoAttr(const std::string & setting, const std::string & value) override {
      if (Info()->cols.size() == 0) Info()->cols.resize(GetNumCols());
      Info()->cols[cur_col].extras.attr.Set(setting, value);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    void DoListen(const std::string & event_name, size_t fun_id) override {
      if (Info()->cols.size() == 0) Info()->cols.resize(GetNumCols());
      Info()->cols[cur_col].extras.listen.Set(event_name, fun_id);
      if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
    }

    TableCol & Clear() { Info()->ClearCol(cur_col); return *this; }
    TableCol & ClearStyle() { Info()->cols[cur_col].extras.style.Clear(); return *this; }
    TableCol & ClearAttr() { Info()->cols[cur_col].extras.attr.Clear(); return *this; }
    TableCol & ClearListen() { Info()->cols[cur_col].extras.listen.Clear(); return *this; }
    TableCol & ClearExtras() { Info()->cols[cur_col].extras.Clear(); return *this; }
    TableCol & ClearChildren() { Info()->ClearColChildren(cur_col); return *this; }
    TableCol & ClearCells() { Info()->ClearCol(cur_col); return *this; }

    /// Get the current CSS value for the specified setting of this Table column.
    const std::string & GetCSS(const std::string & setting) const override {
      return Info()->cols[cur_col].extras.GetStyle(setting);
    }

  };

}
}

#endif
