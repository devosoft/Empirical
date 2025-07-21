/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/tools/Pos.hpp
 * @brief A simple bundling of row count and column count for grids, tables, etc.
 */

#pragma once

#ifndef INCLUDE_EMP_TOOLS_GRID_SIZE_HPP_GUARD
#define INCLUDE_EMP_TOOLS_GRID_SIZE_HPP_GUARD

#include <compare>
#include <iostream>

#include "../base/assert.hpp"
#include "../math/constants.hpp"

namespace emp {

  /// Simple enum for handling direction on a grid.
  /// Use ++ or -- (mod SIZE) to rotate clockwise or counterclockwise, respectively.
  struct Grid {
    enum Dir {UP_LEFT=0, UP, UP_RIGHT, RIGHT, DOWN_RIGHT, DOWN, DOWN_LEFT, LEFT, SIZE };
    static constexpr std::array<Dir, 8> DIR_SET{
      Dir::UP_LEFT, Dir::UP, Dir::UP_RIGHT, Dir::RIGHT,
      Dir::DOWN_RIGHT, Dir::DOWN, Dir::DOWN_LEFT, Dir::LEFT
    };
    static constexpr size_t NUM_DIRS = static_cast<size_t>(Dir::SIZE);

    /// A simple 2-dimensional grid point.
    class Pos {
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
        emp_assert(OK_ID(id));
        return static_cast<pos_t>(id);
      }
    public:
      constexpr Pos() = default;                     // Default = 0,0
      constexpr Pos(const Pos & _in) = default;  // Copy constructor

      constexpr Pos(size_t row, size_t col) : row_id(ToPosT(row)), col_id(ToPosT(col)) {}

      constexpr Pos & operator=(const Pos & _in) = default;

      [[nodiscard]] constexpr auto operator<=>(const Pos &) const = default;

      [[nodiscard]] constexpr pos_t Row() const { return row_id; }
      [[nodiscard]] constexpr pos_t Col() const { return col_id; }
      [[nodiscard]] constexpr pos_t & Row() { return row_id; }
      [[nodiscard]] constexpr pos_t & Col() { return col_id; }
      [[nodiscard]] constexpr int32_t SRow() const { return static_cast<int32_t>(row_id); }
      [[nodiscard]] constexpr int32_t SCol() const { return static_cast<int32_t>(col_id); }

      [[nodiscard]] constexpr bool OK() const { return OK_IDs(row_id, col_id); }
      [[nodiscard]] constexpr bool IsValid() const { return ValidIDs(row_id, col_id); }
      constexpr void SetInvalid() { row_id = col_id = MAX_ID; }

      constexpr Pos & SetRow(size_t in_row) {
        row_id = in_row;
        emp_assert(IsValid());
        return *this;
      }

      constexpr Pos & SetCol(size_t in_col) {
        col_id = in_col;
        emp_assert(IsValid());
        return *this;
      }

      constexpr Pos & Set(size_t in_row, size_t in_col) {
        row_id = in_row;
        col_id = in_col;
        emp_assert(IsValid());
        return *this;
      }

      [[nodiscard]] constexpr bool AtOrigin() const { return row_id == 0 && col_id == 0; }

      [[nodiscard]] constexpr bool NonZero() const { return row_id != 0 || col_id != 0; }

      [[nodiscard]] constexpr Pos GetOffset(int32_t off_row, int32_t off_col) const {
        const int32_t new_row = SRow() + off_row;
        const int32_t new_col = SCol() + off_col;
        emp_assert(new_row >= 0 && new_col >= 0);
        return {static_cast<pos_t>(new_row), static_cast<pos_t>(new_col)};
      }

      [[nodiscard]] constexpr Pos GetOffset(size_t off_row, size_t off_col) const {
        const size_t new_row = Row() + off_row;
        const size_t new_col = Col() + off_col;
        emp_assert(ValidIDs(new_row, new_col));
        return {static_cast<pos_t>(new_row), static_cast<pos_t>(new_col)};
      }

      [[nodiscard]] constexpr Pos operator+(const Pos & _in) const {
        return {row_id + _in.row_id, col_id + _in.col_id};
      }

      [[nodiscard]] constexpr Pos operator-(const Pos & _in) const {
        return {row_id - _in.row_id, col_id - _in.col_id};
      }

      [[nodiscard]] constexpr Pos operator%(const Pos & _in) const {
        return {row_id % _in.row_id, col_id % _in.col_id};
      }

      // Modify this point.
      template <typename T>
      constexpr Pos & Offset(T shift_row, T shift_col) {
        return *this = GetOffset(shift_row, shift_col);
      }

      constexpr Pos & Offset(const Pos & shift) {
        return *this += shift;
      }

      constexpr Pos & ToOrigin() {
        row_id = 0;
        col_id = 0;
        return *this;
      }

      constexpr Pos & BoundLower(const Pos & bound) {
        row_id = std::max(row_id, bound.row_id);
        col_id = std::max(col_id, bound.col_id);
        return *this;
      }

      constexpr Pos & BoundUpper(const Pos & bound) {
        row_id = std::min(row_id, bound.row_id);
        col_id = std::min(col_id, bound.col_id);
        return *this;
      }

      constexpr Pos & operator+=(const Pos & _in) { return *this = operator+(_in); }

      constexpr Pos & operator-=(const Pos & _in) { return *this = operator-(_in); }

      constexpr Pos & operator%=(const Pos & _in) { return *this = operator%(_in); }
    };

    class Size : public Pos {
    public:
      constexpr Size() = default;  // Default = 0,0

      constexpr Size(const Size &) = default;  // Copy constructor

      constexpr explicit Size(const Pos & in) : Pos(in) {}

      constexpr Size(size_t num_rows, size_t num_cols) : Pos(num_rows, num_cols) {}

      // Automatically find the size of vectors of vectors.
      template<typename T>
      constexpr Size(emp::vector< emp::vector<T> > v)
        : Size(v.size(), v.size() ? v[0].size() : 0) { }

      // Automatically find the size of vectors of vectors.
      template<typename T>
      constexpr Size(std::vector< std::vector<T> > v)
        : Size(v.size(), v.size() ? v[0].size() : 0) { }

      constexpr Size & operator=(const Size & _in) = default;

      [[nodiscard]] constexpr size_t NumRows() const { return row_id; }
      [[nodiscard]] constexpr size_t NumCols() const { return col_id; }
      [[nodiscard]] constexpr size_t NumCells() const { return NumRows() * NumCols(); }

      static constexpr Pos InvalidPos() { return {MAX_ID, MAX_ID}; }

      [[nodiscard]] constexpr bool IsInside(const Pos & pos) const {
        return pos.Row() < NumRows() && pos.Col() < NumCols();
      }

      [[nodiscard]] constexpr bool AtTopEdge(const Pos & pos) const {
        return pos.Row() == 0 && pos.Col() < NumCols();
      }

      [[nodiscard]] constexpr bool AtBottomEdge(const Pos & pos) const {
        return pos.Row() == NumRows() - 1 && pos.Col() < NumCols();
      }

      [[nodiscard]] constexpr bool AtLeftEdge(const Pos & pos) const {
        return pos.Row() < NumRows() && pos.Col() == 0;
      }

      [[nodiscard]] constexpr bool AtRightEdge(const Pos & pos) const {
        return pos.Row() < NumRows() && pos.Col() == NumCols() - 1;
      }

      // Convert a Pos to a linear index.
      [[nodiscard]] constexpr size_t ToIndex(const Pos & pos) const {
        if (!IsInside(pos)) return MAX_SIZE_T;
        return NumCols() * pos.Row() + pos.Col();
      }

      [[nodiscard]] constexpr Pos FromIndex(size_t id) const {
        return { id / NumCols(), id % NumCols() };
      }

      /// Identify the grid position in the specified direction.
      [[nodiscard]] constexpr Pos PosDir(const Pos & in, Dir dir) const {
        Pos out = in;
        if (!in.IsValid()) return out;  // Invalid positions are invalid in all directions.
        switch (dir) {
        case Dir::UP_LEFT:    out.Col() -= 1; [[fallthrough]];
        case Dir::UP:         out.Row() -= 1; break;
        case Dir::UP_RIGHT:   out.Row() -= 1; [[fallthrough]];
        case Dir::RIGHT:      out.Col() += 1; break;
        case Dir::DOWN_RIGHT: out.Col() += 1; [[fallthrough]];
        case Dir::DOWN:       out.Row() += 1; break;
        case Dir::DOWN_LEFT:  out.Row() += 1; [[fallthrough]];
        case Dir::LEFT:       out.Col() -= 1; break;
        default: emp_assert(false, "Invalid direction.");
        }
        if (IsInside(out)) return out;
        return InvalidPos();
      }

      /// Identify the grid position in the specified direction, wrapping as needed.
      [[nodiscard]] constexpr Pos PosDirWrap(const Pos & in, Dir dir) const {
        Pos out = in;
        if (!in.IsValid()) return out;  // Invalid positions are invalid in all directions.
        switch (dir) {
        case Dir::UP_LEFT:    --out.Col() %= NumCols(); [[fallthrough]];
        case Dir::UP:         --out.Row() %= NumRows(); break;
        case Dir::UP_RIGHT:   --out.Row() %= NumRows(); [[fallthrough]];
        case Dir::RIGHT:      ++out.Col() %= NumCols(); break;
        case Dir::DOWN_RIGHT: ++out.Col() %= NumCols(); [[fallthrough]];
        case Dir::DOWN:       ++out.Row() %= NumRows(); break;
        case Dir::DOWN_LEFT:  ++out.Row() %= NumRows(); [[fallthrough]];
        case Dir::LEFT:       --out.Col() %= NumCols(); break;
        default: emp_assert(false, "Invalid direction.");
        }
        return out;
      }

      // If a wrap argument is included in PosDir, allow run-time determination of wrapping.
      [[nodiscard]] constexpr Pos PosDir(const Pos & in, Dir dir, bool wrap) const {
        return wrap ? PosDirWrap(in, dir) : PosDir(in, dir);
      }

      [[nodiscard]] constexpr Pos PosUp(Pos in) const { return PosDir(in, UP); }
      [[nodiscard]] constexpr Pos PosDown(Pos in) const { return PosDir(in, DOWN); }
      [[nodiscard]] constexpr Pos PosLeft(Pos in) const { return PosDir(in, LEFT); }
      [[nodiscard]] constexpr Pos PosRight(Pos in) const { return PosDir(in, RIGHT); }
      [[nodiscard]] constexpr Pos PosUL(Pos in) const { return PosDir(in, UP_LEFT); }
      [[nodiscard]] constexpr Pos PosUR(Pos in) const { return PosDir(in, UP_RIGHT); }
      [[nodiscard]] constexpr Pos PosDL(Pos in) const { return PosDir(in, DOWN_LEFT); }
      [[nodiscard]] constexpr Pos PosDR(Pos in) const { return PosDir(in, DOWN_RIGHT); }

      [[nodiscard]] constexpr Pos PosUpWrap(Pos in) const { return PosDirWrap(in, UP); }
      [[nodiscard]] constexpr Pos PosDownWrap(Pos in) const { return PosDirWrap(in, DOWN); }
      [[nodiscard]] constexpr Pos PosLeftWrap(Pos in) const { return PosDirWrap(in, LEFT); }
      [[nodiscard]] constexpr Pos PosRightWrap(Pos in) const { return PosDirWrap(in, RIGHT); }
      [[nodiscard]] constexpr Pos PosULWrap(Pos in) const { return PosDirWrap(in, UP_LEFT); }
      [[nodiscard]] constexpr Pos PosURWrap(Pos in) const { return PosDirWrap(in, UP_RIGHT); }
      [[nodiscard]] constexpr Pos PosDLWrap(Pos in) const { return PosDirWrap(in, DOWN_LEFT); }
      [[nodiscard]] constexpr Pos PosDRWrap(Pos in) const { return PosDirWrap(in, DOWN_RIGHT); }

      [[nodiscard]] constexpr Pos PosUp(Pos in, bool wrap) const {
        return wrap ? PosDirWrap(in, UP) : PosDir(in, UP);
      }
      [[nodiscard]] constexpr Pos PosDown(Pos in, bool wrap) const {
        return wrap ? PosDirWrap(in, DOWN) : PosDir(in, DOWN);
      }
      [[nodiscard]] constexpr Pos PosLeft(Pos in, bool wrap) const {
        return wrap ? PosDirWrap(in, LEFT) : PosDir(in, LEFT);
      }
      [[nodiscard]] constexpr Pos PosRight(Pos in, bool wrap) const {
        return wrap ? PosDirWrap(in, RIGHT) : PosDir(in, RIGHT);
      }
      [[nodiscard]] constexpr Pos PosUL(Pos in, bool wrap) const {
        return wrap ? PosDirWrap(in, UP_LEFT) : PosDir(in, UP_LEFT);
      }
      [[nodiscard]] constexpr Pos PosUR(Pos in, bool wrap) const {
        return wrap ? PosDirWrap(in, UP_RIGHT) : PosDir(in, UP_RIGHT);
      }
      [[nodiscard]] constexpr Pos PosDL(Pos in, bool wrap) const {
        return wrap ? PosDirWrap(in, DOWN_LEFT) : PosDir(in, DOWN_LEFT);
      }
      [[nodiscard]] constexpr Pos PosDR(Pos in, bool wrap) const {
        return wrap ? PosDirWrap(in, DOWN_RIGHT) : PosDir(in, DOWN_RIGHT);
      }
    };
  };

  using GridDir = Grid::Dir;
  using GridPos = Grid::Pos;
  using GridSize = Grid::Size;
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
