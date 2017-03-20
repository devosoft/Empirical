//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The TableCol widget, which behaves like the Table widget, but focuses on a single column.
//
//  DO NOT include directly.  All files begining with '_' are for internal use only.

class TableCol : public Table {
public:
  TableCol(size_t r, size_t c, const std::string & in_id="") : Table(r,c,in_id) { ; }
  TableCol(const Table & in) : Table(in) { ; }
  TableCol(const Widget & in) : Table(in) { ; }
  TableCol(internal::TableInfo * in_info, size_t _col=0) : Table(in_info, 0, _col) { ; }

  // Apply CSS to appropriate component based on current state.
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

  bool InStateCol() const { return true; }

  TableCol & Clear() { Info()->ClearCol(cur_col); return *this; }
  TableCol & ClearStyle() { Info()->ClearColStyle(cur_col); return *this; }
  TableCol & ClearChildren() { Info()->ClearColChildren(cur_col); return *this; }
  TableCol & ClearCells() { Info()->ClearCol(cur_col); return *this; }

  std::string GetCSS(const std::string & setting) override {
    return Info()->cols[cur_col].extras.GetStyle(setting);
  }

};
