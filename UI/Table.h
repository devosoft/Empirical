#ifndef EMP_UI_TABLE_H
#define EMP_UI_TABLE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Table widget.
//


#include "UI_base.h"

namespace emp {
namespace UI {

  // Forward-declare the internal Table_detail class
  namespace internal { class Table_detail; };
  
  // Specify the Table class for use in return values in the Table_detail definitions below.
  using Table = internal::Widget_wrap<internal::Table_detail, int, int>;

  struct TableCell {
    int row;      // In which row is the upper-left corner of this cell?
    int col;      // In which column is the upper-left corner of this cell?
    int width;    // How many columns wide is this cell?.
    int height;   // How many rows tall is this cell?
    bool header;  // Is this cell a header?
    bool masked;  // Is this cell hidden by another?
  };

  namespace internal {
    class Table_detail : public Widget_base {
    protected:
      int cols;
      int rows;
      
      std::vector<TableCell> cell_info;
      
      void WriteHTML(std::ostream & os) {
      }
    public:
      Table_detail(int in_cols, int in_rows) : cols(in_cols), rows(in_rows) { ; }
      
      Table & Cols(int c) { cols = c; return (Table &) *this; }
      Table & Rows(int r) { rows = r; return (Table &) *this; }
    };
  }

};
};

#endif
