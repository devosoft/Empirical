/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/tools/GridPos.hpp
 * @brief A simple bundling of row count and column count for grids, tables, etc.
 */

#pragma once

#ifndef INCLUDE_EMP_TOOLS_GRID_SIZE_HPP_GUARD
#define INCLUDE_EMP_TOOLS_GRID_SIZE_HPP_GUARD

// #include <cmath>
#include <compare>
#include <iostream>

#include "../base/assert.hpp"
#include "../math/constants.hpp"

namespace emp {

  /// A simple 2-dimensional point.
  class GridPos {
  public:
    using pos_t = uint32_t;

  protected:
    // Values above MAX_ID indicate an internal problem (for error checking)
    // Values at MAX_ID are legal, but indicate a non-position.
    static constexpr size_t MAX_ID = emp::MAX_INT;
    pos_t row_id = 0;
    pos_t col_id = 0;

    [[nodiscard]] static constexpr bool OK_ID(size_t id) { return id <= MAX_ID; }
    [[nodiscard]] static constexpr bool OK_IDs(size_t id1, size_t id2) { return id1 <= MAX_ID && id2 <= MAX_ID; }

    [[nodiscard]] static constexpr bool ValidID(size_t id) { return id < MAX_ID; }
    [[nodiscard]] static constexpr bool ValidIDs(size_t id1, size_t id2) { return id1 < MAX_ID && id2 < MAX_ID; }

    [[nodiscard]] static constexpr pos_t ToPosT(size_t id) {
      emp_assert(ValidID(id));
      return static_cast<pos_t>(id);
    }
  public:
    constexpr GridPos() = default;                     // Default = 0,0
    constexpr GridPos(const GridPos & _in) = default;  // Copy constructor

    constexpr GridPos(size_t row, size_t col) : row_id(ToPosT(row)), col_id(ToPosT(col)) {}

    constexpr GridPos & operator=(const GridPos & _in) = default;

    [[nodiscard]] constexpr auto operator<=>(const GridPos &) const = default;

    [[nodiscard]] constexpr pos_t Row() const { return row_id; }
    [[nodiscard]] constexpr pos_t Col() const { return col_id; }
    [[nodiscard]] constexpr int32_t SRow() const { return static_cast<int32_t>(row_id); }
    [[nodiscard]] constexpr int32_t SCol() const { return static_cast<int32_t>(col_id); }

    [[nodiscard]] constexpr bool OK() const { return OK_IDs(row_id, col_id); }
    [[nodiscard]] constexpr bool IsValid() const { return ValidIDs(row_id, col_id); }
    constexpr void MakeInvalid() { row_id = col_id = MAX_ID; }

    constexpr GridPos & SetRow(size_t in_row) {
      row_id = in_row;
      emp_assert(IsValid());
      return *this;
    }

    constexpr GridPos & SetCol(size_t in_col) {
      col_id = in_col;
      emp_assert(IsValid());
      return *this;
    }

    constexpr GridPos & Set(size_t in_row, size_t in_col) {
      row_id = in_row;
      col_id = in_col;
      emp_assert(IsValid());
      return *this;
    }

    [[nodiscard]] constexpr bool AtOrigin() const { return row_id == 0 && col_id == 0; }

    [[nodiscard]] constexpr bool NonZero() const { return row_id != 0 || col_id != 0; }

    [[nodiscard]] constexpr GridPos GetOffset(int32_t off_row, int32_t off_col) const {
      const int32_t new_row = SRow() + off_row;
      const int32_t new_col = SCol() + off_col;
      emp_assert(ValidIDs(static_cast<size_t>(new_row), static_cast<size_t>(new_col)));
      return {static_cast<pos_t>(new_row), static_cast<pos_t>(new_col)};
    }

    [[nodiscard]] constexpr GridPos GetOffset(size_t off_row, size_t off_col) const {
      const size_t new_row = Row() + off_row;
      const size_t new_col = Col() + off_col;
      emp_assert(ValidIDs(new_row, new_col));
      return {static_cast<pos_t>(new_row), static_cast<pos_t>(new_col)};
    }

    [[nodiscard]] constexpr GridPos operator+(const GridPos & _in) const {
      return {row_id + _in.row_id, col_id + _in.col_id};
    }

    [[nodiscard]] constexpr GridPos operator-(const GridPos & _in) const {
      return {row_id - _in.row_id, col_id - _in.col_id};
    }

    [[nodiscard]] constexpr GridPos operator%(const GridPos & _in) const {
      return {row_id % _in.row_id, col_id % _in.col_id};
    }

    // Modify this point.
    template <typename T>
    constexpr GridPos & Offset(T shift_row, T shift_col) {
      return *this = GetOffset(shift_row, shift_col);
    }

    constexpr GridPos & ToOrigin() {
      row_id = 0;
      col_id = 0;
      return *this;
    }

    constexpr GridPos & BoundLower(const GridPos & bound) {
      row_id = std::max(row_id, bound.row_id);
      col_id = std::max(col_id, bound.col_id);
      return *this;
    }

    constexpr GridPos & BoundUpper(const GridPos & bound) {
      row_id = std::min(row_id, bound.row_id);
      col_id = std::min(col_id, bound.col_id);
      return *this;
    }

    constexpr GridPos & operator+=(const GridPos & _in) { return *this = operator+(_in); }

    constexpr GridPos & operator-=(const GridPos & _in) { return *this = operator-(_in); }

    constexpr GridPos & operator%=(const GridPos & _in) { return *this = operator%(_in); }
  };

  class GridSize : public GridPos {
  private:
    constexpr static GridPos InvalidPos() { return {MAX_ID, MAX_ID}; }
  public:
    constexpr GridSize() = default;  // Default = 0,0

    constexpr GridSize(const GridSize &) = default;  // Copy constructor

    constexpr explicit GridSize(const GridPos & in) : GridPos(in) {}

    constexpr GridSize(size_t num_rows, size_t num_cols) : GridPos(num_rows, num_cols) {}

    // Automatically find the size of vectors of vectors.
    template<typename T>
    constexpr GridSize(emp::vector< emp::vector<T> > v)
      : GridSize(v.size(), v.size() ? v[0].size() : 0) { }

    // Automatically find the size of vectors of vectors.
    template<typename T>
    constexpr GridSize(std::vector< std::vector<T> > v)
      : GridSize(v.size(), v.size() ? v[0].size() : 0) { }

    constexpr GridSize & operator=(const GridSize & _in) = default;

    [[nodiscard]] constexpr size_t NumRows() const { return row_id; }
    [[nodiscard]] constexpr size_t NumCols() const { return col_id; }
    [[nodiscard]] constexpr size_t NumCells() const { return NumRows() * NumCols(); }

    [[nodiscard]] constexpr bool IsInside(const GridPos & pos) const {
      return pos.Row() < NumRows() && pos.Col() < NumCols();
    }

    [[nodiscard]] constexpr bool AtTopEdge(const GridPos & pos) const {
      return pos.Row() == 0 && pos.Col() < NumCols();
    }

    [[nodiscard]] constexpr bool AtBottomEdge(const GridPos & pos) const {
      return pos.Row() == NumRows() - 1 && pos.Col() < NumCols();
    }

    [[nodiscard]] constexpr bool AtLeftEdge(const GridPos & pos) const {
      return pos.Row() < NumRows() && pos.Col() == 0;
    }

    [[nodiscard]] constexpr bool AtRightEdge(const GridPos & pos) const {
      return pos.Row() < NumRows() && pos.Col() == NumCols() - 1;
    }

    // Convert a GridPos to a linear index.
    [[nodiscard]] constexpr size_t ToIndex(const GridPos & pos) const {
      emp_assert(IsInside(pos));
      return NumCols() * pos.Row() + pos.Col();
    }

    [[nodiscard]] constexpr GridPos FromIndex(size_t id) const {
      return { id / NumRows(), id % NumRows() };
    }

    [[nodiscard]] constexpr GridPos PosUp(const GridPos & in, bool wrap=false) const {
      if (!in.IsValid()) return in;
      if (AtTopEdge(in)) {
        if (wrap) return {NumRows()-1, in.Col()};
        else return InvalidPos();
      } else { return {in.Row()-1, in.Col()}; }
    }
    [[nodiscard]] constexpr GridPos PosDown(const GridPos & in, bool wrap=false) const {
      if (!in.IsValid()) return in;
      if (AtBottomEdge(in)) {
        if (wrap) return {0, in.Col()};
        else { return InvalidPos(); }
      } else { return {in.Row()+1, in.Col()}; }
    }
    [[nodiscard]] constexpr GridPos PosLeft(const GridPos & in, bool wrap=false) const {
      if (!in.IsValid()) return in;
      if (AtLeftEdge(in)) {
        if (wrap) return {in.Row(), NumCols()-1};
        else return InvalidPos();
      } else { return {in.Row(), in.Col()-1}; }
    }
    [[nodiscard]] constexpr GridPos PosRight(const GridPos & in, bool wrap=false) const {
      if (!in.IsValid()) return in;
      if (AtRightEdge(in)) {
        if (wrap) return {in.Row(), 0};
        else return InvalidPos();
      } else { return {in.Row(), in.Col()+1}; }
    }

    [[nodiscard]] constexpr GridPos PosUL(const GridPos & in, bool wrap=false) const {
      return PosLeft(PosUp(in, wrap), wrap);
    }

    [[nodiscard]] constexpr GridPos PosUR(const GridPos & in, bool wrap=false) const {
      return PosRight(PosUp(in, wrap), wrap);
    }

    [[nodiscard]] constexpr GridPos PosDL(const GridPos & in, bool wrap=false) const {
      return PosLeft(PosDown(in, wrap), wrap);
    }

    [[nodiscard]] constexpr GridPos PosDR(const GridPos & in, bool wrap=false) const {
      return PosRight(PosDown(in, wrap), wrap);
    }
  };

}  // namespace emp

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace std {
  // Overload ostream to work with points.
  std::ostream & operator<<(std::ostream & os, const emp::GridPos & point) {
    return os << "(" << point.Row() << "," << point.Col() << ")";
  }
}  // namespace std
#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif  // #ifndef INCLUDE_EMP_TOOLS_GRID_SIZE_HPP_GUARD
