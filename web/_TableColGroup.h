//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The TableColGroup widget, which behaves like the Table widget, but focuses on a column group.
//
//  DO NOT include directly.  All files begining with '_' are for internal use only.

class TableColGroup : public TableWidget {
public:
  TableColGroup(size_t r, size_t c, const std::string & in_id="") : TableWidget(r,c,in_id) { ; }
  TableColGroup(const TableWidget & in) : TableWidget(in) { ; }
  TableColGroup(const Widget & in) : TableWidget(in) { ; }
  TableColGroup(internal::TableInfo * in_info, size_t _col=0) : TableWidget(in_info, 0, _col) { ; }

  // Apply CSS to appropriate component based on current state.
  void DoCSS(const std::string & setting, const std::string & value) override {
    if (Info()->col_groups.size() == 0) Info()->col_groups.resize(GetNumCols());
    Info()->col_groups[cur_col].extras.style.Set(setting, value);
    if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
  }

  void DoAttr(const std::string & setting, const std::string & value) override {
    if (Info()->col_groups.size() == 0) Info()->col_groups.resize(GetNumCols());
    Info()->col_groups[cur_col].extras.attr.Set(setting, value);
    if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
  }

  void DoListen(const std::string & event_name, size_t fun_id) override {
    if (Info()->col_groups.size() == 0) Info()->col_groups.resize(GetNumCols());
    Info()->col_groups[cur_col].extras.listen.Set(event_name, fun_id);
    if (IsActive()) Info()->ReplaceHTML();   // @CAO only should replace cell's CSS
  }

  TableColGroup & Clear() { Info()->ClearColGroup(cur_col); return *this; }
  TableColGroup & ClearStyle() { Info()->col_groups[cur_col].extras.style.Clear(); return *this; }
  TableColGroup & ClearAttr() { Info()->col_groups[cur_col].extras.attr.Clear(); return *this; }
  TableColGroup & ClearListen() { Info()->col_groups[cur_col].extras.listen.Clear(); return *this; }
  TableColGroup & ClearExtras() { Info()->col_groups[cur_col].extras.Clear(); return *this; }
  TableColGroup & ClearChildren() { Info()->ClearColGroupChildren(cur_col); return *this; }
  TableColGroup & ClearCells() { Info()->ClearColGroup(cur_col); return *this; }

  std::string GetCSS(const std::string & setting) override {
    return Info()->col_groups[cur_col].extras.GetStyle(setting);
  }

  TableColGroup SetColSpan(size_t new_span) {
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

  TableColGroup SetSpan(size_t new_span) { SetColSpan(new_span); return *this; }
};
