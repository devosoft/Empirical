//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The TableCol widget, which behaves like the Table widget, but focuses on a single column.
//
//  DO NOT include directly.  All files begining with '_' are for internal use only.

class TableCol : public TableWidget {
public:
  TableCol(size_t r, size_t c, const std::string & in_id="") : TableWidget(r,c,in_id) { ; }
  TableCol(const TableWidget & in) : TableWidget(in) { ; }
  TableCol(const Widget & in) : TableWidget(in) { ; }
  TableCol(internal::TableInfo * in_info, size_t _col=0) : TableWidget(in_info, 0, _col) { ; }

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

  void Clear() { Info()->ClearCol(cur_col); }
  void ClearStyle() { Info()->cols[cur_col].extras.style.Clear(); }
  void ClearAttr() { Info()->cols[cur_col].extras.attr.Clear(); }
  void ClearListen() { Info()->cols[cur_col].extras.listen.Clear(); }
  void ClearExtras() { Info()->cols[cur_col].extras.Clear(); }
  void ClearChildren() { Info()->ClearColChildren(cur_col); }
  void ClearCells() { Info()->ClearCol(cur_col); }

  std::string GetCSS(const std::string & setting) override {
    return Info()->cols[cur_col].extras.GetStyle(setting);
  }

};
