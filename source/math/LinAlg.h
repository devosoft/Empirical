#ifndef LIN_ALG_H
#define LIN_ALG_H

#include <algorithm>
#include <array>
#include <cmath>
#include <ostream>
#include <stdexcept>

#include "base/assert.h"
#include "opengl/VertexAttributes.h"

namespace emp {

  namespace math {
    template <typename F, std::size_t R, std::size_t C>
    class Mat;

    namespace internal {
      template <std::size_t R, std::size_t C, typename F, std::size_t... I>
      constexpr std::array<F, R * C> unfoldArrayImpl(
        const std::array<std::array<F, C>, R>& data,
        const std::index_sequence<I...>&) {
        return {data(I / C, I % C)...};
      }
      template <std::size_t R, std::size_t C, typename F>
      constexpr decltype(auto) unfoldArray(
        const std::array<std::array<F, C>, R>& data) {
        return unfoldArrayImpl(data, std::make_index_sequence<R * C>{});
      }

      template <typename F>
      constexpr F ident(size_t i, size_t j) {
        return i == j ? F{1} : F{0};
      }

      template <typename To, typename From>
      constexpr To convertImpl(From&& from, const std::true_type&) {
        return std::forward<From>(from);
      }

      template <typename To, typename From>
      constexpr To convertImpl(From&& from, const std::false_type&) {
        return To{std::forward<From>(from)};
      }

      template <typename To, typename From>
      constexpr To convert(From&& from) {
        return convertImpl<To>(std::forward<From>(from),
                               std::is_convertible<From, To>{});
      }

    }  // namespace internal

    namespace internal {
      template <typename A, typename B, std::size_t I>
      constexpr decltype(auto) dotProductImpl(const A& a, const B& b,
                                              const std::index_sequence<I>&) {
        return a[I] * b[I];
      }

      template <typename A, typename B, std::size_t H, std::size_t H2,
                std::size_t... T>
      constexpr decltype(auto) dotProductImpl(
        const A& a, const B& b, const std::index_sequence<H, H2, T...>&) {
        return (a[H] * b[H]) +
               dotProductImpl(a, b, std::index_sequence<H2, T...>{});
      }

      template <std::size_t N, typename A, typename B>
      constexpr decltype(auto) dotProduct(const A& a, const B& b) {
        return dotProductImpl(a, b, std::make_index_sequence<N>{});
      }

      struct MatrixMult {
        template <typename A, typename B>
        constexpr auto operator()(std::size_t r, std::size_t c, A&& a,
                                  B&& b) const {
          return std::forward<A>(a).row(r) * std::forward<B>(b).col(c);
        }
      };

      struct MatrixAdd {
        template <typename A, typename B>
        constexpr auto operator()(std::size_t r, std::size_t c, A&& a,
                                  B&& b) const {
          return std::forward<A>(a)(r, c) + std::forward<B>(b)(r, c);
        }
      };

      struct MatrixSub {
        template <typename A, typename B>
        constexpr auto operator()(std::size_t r, std::size_t c, A&& a,
                                  B&& b) const {
          return std::forward<A>(a)(r, c) - std::forward<B>(b)(r, c);
        }
      };

      struct MatrixTranspose {
        template <typename M>
        constexpr auto operator()(std::size_t r, std::size_t c, M&& m) const {
          return std::forward<M>(m)(c, r);
        }
      };

      struct RightScalarMult {
        template <typename M, typename S>
        constexpr auto operator()(std::size_t r, std::size_t c, M&& mat,
                                  S&& s) const {
          return std::forward<M>(mat)(r, c) * std::forward<S>(s);
        }
      };

      struct LeftScalarMult {
        template <typename M, typename S>
        constexpr auto operator()(std::size_t r, std::size_t c, M&& mat,
                                  S&& s) const {
          return std::forward<S>(s) * std::forward<M>(mat)(r, c);
        }
      };

    }  // namespace internal

    template <typename F, std::size_t D>
    class Row {
      public:
      using value_type = F;
      static constexpr auto rows = 1;
      static constexpr auto columns = D;

      protected:
      F* ref;

      public:
      constexpr Row(F* ref) : ref(ref) {}
      constexpr Row(const Row&) = default;
      constexpr Row(Row&&) = default;
      Row& operator=(const Row&) = default;
      Row& operator=(Row&&) = default;

      template <typename G>
      constexpr bool operator==(const Row<G, D>& other) const {
        for (std::size_t i = 0; i < rows * columns; ++i) {
          if (ref[i] != other.ref[i]) {
            return false;
          }
        }
        return true;
      }

      template <typename G>
      constexpr bool operator==(const Mat<G, 1, D>& other) const {
        for (std::size_t i = 0; i < rows * columns; ++i) {
          if (ref[i] != other.data[i]) {
            return false;
          }
        }
        return true;
      }

      constexpr decltype(auto) operator[](std::size_t i) const {
        return ref[i];
      }
      constexpr decltype(auto) operator()(std::size_t i) const {
        return (*this)[i];
      }

      F& operator[](std::size_t i) { return ref[i]; }
      decltype(auto) operator()(std::size_t i) { return (*this)[i]; }

      constexpr F* data() noexcept { return ref; }
      constexpr const F* data() const noexcept { return ref; }

      constexpr decltype(auto) begin() const { return ref; }
      constexpr decltype(auto) end() const { return ref + D; }
    };

    template <typename F, std::size_t D>
    std::ostream& operator<<(std::ostream& out, const Row<F, D>& row) {
      out << "{";
      for (std::size_t i = 0; i < D; ++i) {
        out << row[i];
        if (i != D - 1) out << " ";
      }
      return out << "}";
    }

    template <typename F, std::size_t D>
    class Col {
      public:
      using value_type = F;
      static constexpr auto rows = D;
      static constexpr auto columns = 1;

      protected:
      F* ref;

      public:
      constexpr Col(F* ref) : ref(ref) {}
      constexpr Col(const Col&) = default;
      constexpr Col(Col&&) = default;
      Col& operator=(const Col&) = default;
      Col& operator=(Col&&) = default;

      template <typename G>
      constexpr bool operator==(const Col<G, D>& other) const {
        for (std::size_t i = 0; i < rows * columns; ++i) {
          if (ref[i] != other.ref[i]) {
            return false;
          }
        }
        return true;
      }

      template <typename G>
      constexpr bool operator==(const Mat<G, D, 1>& other) const {
        for (std::size_t i = 0; i < rows * columns; ++i) {
          if (ref[i] != other.data[i]) {
            return false;
          }
        }
        return true;
      }

      constexpr decltype(auto) operator[](std::size_t i) const {
        return ref[i * rows];
      }
      constexpr decltype(auto) operator()(std::size_t i) const {
        return (*this)[i];
      }

      F& operator[](std::size_t i) { return ref[i * rows]; }
      decltype(auto) operator()(std::size_t i) { return (*this)[i]; }

      constexpr F* data() noexcept { return ref; }
      constexpr const F* data() const noexcept { return ref; }
    };

    template <typename F, std::size_t R, std::size_t C>
    class Mat {
      public:
      static_assert(R >= 0 && C >= 0 && (R != 1 || C != 1),
                    "Matrix is too small");
      using value_type = F;
      static constexpr auto rows = R;
      static constexpr auto columns = C;

      protected:
      // This needs to wait until c++17 so that arrayData.data() is constexpr
      // std::array<F, rows * columns> arrayData;
      F arrayData[rows * columns];

      private:
      template <typename G, typename... Args, std::size_t... I>
      constexpr static Mat gen(G&& callback, const std::index_sequence<I...>&,
                               Args&&... args) {
        return {callback(I / columns, I % columns, args...)...,
                std::forward<G>(callback)(rows - 1, columns - 1,
                                          std::forward<Args>(args)...)};
      }

      public:
      template <typename G, typename... Args>
      constexpr static Mat from(G&& callback, Args&&... args) {
        return gen(std::forward<G>(callback),
                   std::make_index_sequence<rows * columns - 1>{},
                   std::forward<Args>(args)...);
      }

      static constexpr Mat identity() {
        static_assert(R == C, "Identity matrices must be square");

        return Mat::from(&internal::ident<F>);
      }

      template <typename X = F, typename Y = F, typename Z = F>
      static constexpr Mat translation(X&& x, Y&& y, Z&& z = Z{0}) {
        static_assert(R == 4 && C == 4,
                      "Homogenous coordinate translation matrices must be 4x4");
        return {
          1, 0, 0, std::forward<X>(x),  // row 1
          0, 1, 0, std::forward<Y>(y),  // row 2
          0, 0, 1, std::forward<Z>(z),  // row 3
          0, 0, 0, 1,                   // row 4
        };
      }

      template <typename H1, typename H2, typename... Args>
      constexpr Mat(H1&& h1, H2&& h2, Args&&... args)
        : arrayData{internal::convert<F>(std::forward<H1>(h1)),
                    internal::convert<F>(std::forward<H2>(h2)),
                    internal::convert<F>(std::forward<Args>(args))...} {
        static_assert(
          sizeof...(Args) + 2 == rows * columns,
          "Invalid number of arguments for a matrix of the given size");
      }

      constexpr Mat(const Mat&) = default;
      constexpr Mat(Mat&&) = default;
      Mat& operator=(const Mat&) = default;
      Mat& operator=(Mat&&) = default;

      constexpr auto x() const {
        static_assert((R >= 1 && C == 1) || (R == 1 && C >= 1),
                      "A matrix must be a row matrix or a column matrix with "
                      "at least one entry to have an x component");
        return arrayData[0];
      }

      constexpr auto& x() {
        static_assert((R >= 1 && C == 1) || (R == 1 && C >= 1),
                      "A matrix must be a row matrix or a column matrix with "
                      "at least one entry to have an x component");
        return arrayData[0];
      }

      constexpr auto y() const {
        static_assert((R >= 2 && C == 1) || (R == 1 && C >= 2),
                      "A matrix must be a row matrix or a column matrix with "
                      "at least two entries to have an y component");
        return arrayData[1];
      }

      constexpr auto& y() {
        static_assert((R >= 2 && C == 1) || (R == 1 && C >= 2),
                      "A matrix must be a row matrix or a column matrix with "
                      "at least two entries to have an y component");
        return arrayData[1];
      }

      constexpr auto z() const {
        static_assert((R >= 3 && C == 1) || (R == 1 && C >= 3),
                      "A matrix must be a row matrix or a column matrix with "
                      "at least three entries to have an z component");
        return arrayData[2];
      }

      constexpr auto& z() {
        static_assert((R >= 3 && C == 1) || (R == 1 && C >= 3),
                      "A matrix must be a row matrix or a column matrix with "
                      "at least three entries to have an z component");
        return arrayData[2];
      }

      constexpr auto w() const {
        static_assert((R >= 4 && C == 1) || (R == 1 && C >= 4),
                      "A matrix must be a row matrix or a column matrix with "
                      "at least four entries to have an w component");
        return arrayData[3];
      }

      constexpr F& w() {
        static_assert((R >= 4 && C == 1) || (R == 1 && C >= 4),
                      "A matrix must be a row matrix or a column matrix with "
                      "at least four entries to have an w component");
        return arrayData[3];
      }

      template <typename G>
      constexpr bool operator==(const Mat<G, R, C>& other) const {
        for (std::size_t i = 0; i < rows * columns; ++i) {
          if (arrayData[i] != other.data()[i]) {
            return false;
          }
        }
        return true;
      }

      // Why do (i, j) and not [i][j]? See
      // https://isocpp.org/wiki/faq/operator-overloading#matrix-subscript-op
      constexpr decltype(auto) operator()(std::size_t r, std::size_t c) const {
        if (r >= rows) {
          throw std::out_of_range("rows out of bounds");
        }
        if (c >= columns) {
          throw std::out_of_range("columns out of bounds");
        }
        // emp_assert(r < rows, "rows out of bounds");
        // emp_assert(c < columns, "columns out of bounds");
        return arrayData[r * columns + c];
      }

      constexpr F& operator()(std::size_t r, std::size_t c) {
        if (r >= rows) {
          throw std::out_of_range("rows out of bounds");
        }
        if (c >= columns) {
          throw std::out_of_range("columns out of bounds");
        }
        // emp_assert(r < rows, "rows out of bounds");
        // emp_assert(c < columns, "columns out of bounds");
        return arrayData[r * columns + c];
      }

      constexpr Row<const F, C> row(std::size_t r) const {
        if (r >= rows) {
          throw std::out_of_range("rows out of bounds");
        }
        // emp_assert(r < rows, "rows out of bounds");
        return Row<const F, C>(&arrayData[r * columns]);
      }

      constexpr Row<F, C> row(std::size_t r) {
        if (r >= rows) {
          throw std::out_of_range("rows out of bounds");
        }
        // emp_assert(r < rows, "rows out of bounds");
        return Row<F, C>(&arrayData[r * columns]);
      }

      constexpr Col<const F, R> col(std::size_t c) const {
        if (c >= columns) {
          throw std::out_of_range("columns out of bounds");
        }
        // emp_assert(c < columns, "columns out of bounds");
        return Col<const F, R>(&arrayData[c]);
      }

      constexpr Col<F, R> col(std::size_t c) {
        if (c >= columns) {
          throw std::out_of_range("columns out of bounds");
        }
        // emp_assert(c < columns, "columns out of bounds");
        return Col<F, R>(&arrayData[c]);
      }

      template <typename I>
      constexpr auto operator[](I&& r) const ->
        typename std::enable_if<C != 1,
                                decltype(this->row(std::forward<I>(r)))>::type {
        return row(std::forward<I>(r));
      }

      template <typename I>
      constexpr auto operator[](I&& r) ->
        typename std::enable_if<C != 1,
                                decltype(this->row(std::forward<I>(r)))>::type {
        return row(std::forward<I>(r));
      }

      template <typename I>
      constexpr auto operator[](I&& r) const ->
        typename std::enable_if<C == 1, decltype((*this)(std::forward<I>(r),
                                                         0))>::type {
        return (*this)(std::forward<I>(r), 0);
      }
      template <typename I>
      constexpr auto operator[](I&& r) ->
        typename std::enable_if<C == 1, decltype((*this)(std::forward<I>(r),
                                                         0))>::type {
        return (*this)(std::forward<I>(r), 0);
      }

      /// A pointer to the raw array that this matrix is wrapping
      constexpr F* data() noexcept {
        // Waiting for std::array::data() to get constexpr in c++17
        // return arrayData.data();
        return arrayData;
      }

      /// A pointer to the raw array that this matrix is wrapping
      constexpr const F* data() const noexcept {
        // Waiting for std::array::data() to get constexpr in c++17
        // return arrayData.data();
        return arrayData;
      }

      /// Gets the transpose of this matrix
      constexpr auto transposed() const {
        // TODO: handle constexprs?
        return Mat<F, C, R>::from(internal::MatrixTranspose{}, *this);
      }

      /// Checks for equality of two matrices with a tolerance. Useful for
      /// comparing floating point matrices
      template <typename H = F>
      constexpr bool feq(const Mat<H, R, C>& other,
                         const H& tolerance = 0.0001) const {
        for (std::size_t i = 0; i < rows * columns; ++i) {
          if (abs(arrayData[i] - other.data()[i]) > tolerance) {
            return false;
          }
        }

        return true;
      }

      /// Finds the squared magnitude of this row or column vector. Using this
      /// on a non-vector matrix will result in a compile time error.
      constexpr decltype(auto) magSq() const {
        static_assert(R == 1 || C == 1,
                      "A matrix must be a column matrix or a row matrix to "
                      "have a magnitude");
        return internal::dotProduct<R * C>(arrayData, arrayData);
      }

      /// Finds the magnitude of this row or column vector. Using this on a
      /// non-vector matrix will result in a compile time error.
      constexpr decltype(auto) mag() const {
        return static_cast<F>(sqrt(magSq()));
      }

      /// Normalizes the row or column vector such that its magnitude is 1.
      constexpr Mat<F, R, C> normalized() const {
        return (*this) * (F{1} / mag());
      }

      /// Applies the given function to all the cells of the matrix
      template <typename G, typename... Args>
      constexpr Mat& apply(G&& callback, Args&&... args) {
        for (std::size_t r = 0; r < rows - 1; ++r) {
          for (std::size_t c = 0; c < columns; ++c) {
            (*this)(r, c) = callback(r, c, *this, args...);
          }
        }
        for (std::size_t c = 0; c < columns - 1; ++c) {
          (*this)(rows - 1, c) = callback(rows - 1, c, *this, args...);
        }
        (*this)(rows - 1, columns - 1) = std::forward<G>(callback)(
          rows - 1, columns - 1, *this, std::forward<Args>(args)...);
        return *this;
      }

      template <typename M = Mat<F, R, C>>
      constexpr Mat& operator+=(M&& other) {
        return apply(internal::MatrixAdd{}, std::forward<M>(other));
      }

      template <typename M = Mat<F, R, C>>
      constexpr Mat& operator-=(M&& other) {
        return apply(internal::MatrixSub{}, std::forward<M>(other));
      }

      constexpr Mat& operator*=(const F& scalar) {
        return apply(internal::RightScalarMult{}, scalar);
      }

    };  // namespace math

    template <typename F, std::size_t R>
    std::ostream& operator<<(std::ostream& out, const Mat<F, R, 1>& mat) {
      out << "{ ";
      for (std::size_t i = 0; i < R; ++i) {
        out << mat(i, 0) << " ";
      }
      return out << "}";
    }

    template <typename F, std::size_t D>
    std::ostream& operator<<(std::ostream& out, const Mat<F, 1, D>& mat) {
      out << "{ ";
      for (std::size_t i = 0; i < D; ++i) {
        out << mat(0, i) << " ";
      }
      return out << "}";
    }

    template <typename F, std::size_t R, std::size_t C>
    std::ostream& operator<<(std::ostream& out, const Mat<F, R, C>& mat) {
      out << "{" << std::endl;
      for (std::size_t i = 0; i < R; ++i) {
        out << "\t" << mat[i] << "\n";
      }
      return out << "}";
    }

    template <typename F1, typename F2, std::size_t R, std::size_t C>
    constexpr Mat<decltype(std::declval<F1>() + std::declval<F2>()), R, C>
    operator+(const Mat<F1, R, C>& a, const Mat<F2, R, C>& b) {
      return Mat<decltype(std::declval<F1>() + std::declval<F1>()), R, C>::from(
        internal::MatrixAdd{}, a, b);
    }

    template <typename F1, typename F2, std::size_t R, std::size_t C>
    constexpr Mat<decltype(std::declval<F1>() - std::declval<F2>()), R, C>
    operator-(const Mat<F1, R, C>& a, const Mat<F2, R, C>& b) {
      return Mat<decltype(std::declval<F1>() - std::declval<F2>()), R, C>::from(
        internal::MatrixSub{}, a, b);
    }

    // Define the dot product
    template <typename F1, typename F2, std::size_t D>
    constexpr decltype(auto) operator*(const Mat<F1, 1, D>& v1,
                                       const Mat<F2, D, 1>& v2) {
      return internal::dotProduct<D>(v1.data(), v2.data());
    }

    template <typename F1, typename F2, std::size_t D>
    constexpr decltype(auto) operator*(const Mat<F1, D, 1>& v1,
                                       const Row<F2, D>& v2) {
      return internal::dotProduct<D>(v1.data(), v2);
    }

    template <typename F1, typename F2, std::size_t D>
    constexpr decltype(auto) operator*(const Row<F1, D>& v1,
                                       const Mat<F2, 1, D>& v2) {
      return internal::dotProduct<D>(v1, v2.data());
    }

    template <typename F1, typename F2, std::size_t D>
    constexpr decltype(auto) operator*(const Mat<F1, 1, D>& v1,
                                       const Col<F2, D>& v2) {
      return internal::dotProduct<D>(v1.data(), v2);
    }

    template <typename F1, typename F2, std::size_t D>
    constexpr decltype(auto) operator*(const Col<F1, D>& v1,
                                       const Mat<F2, D, 1>& v2) {
      return internal::dotProduct<D>(v1, v2.data());
    }

    template <typename F1, typename F2, std::size_t D>
    constexpr decltype(auto) operator*(const Row<F1, D>& v1,
                                       const Col<F2, D>& v2) {
      return internal::dotProduct<D>(v1, v2);
    }
    template <typename F1, typename F2, std::size_t R, std::size_t C,
              std::size_t M>
    constexpr auto operator*(const Mat<F1, R, M>& a, const Mat<F2, M, C>& b) {
      using Field = decltype(std::declval<F1>() * std::declval<F2>() +
                             std::declval<F1>() * std::declval<F2>());

      // return Mat<Field, R, C>::from(
      //   [a, b](std::size_t r, std::size_t c) { return a.row(r) * b.col(c);
      //   });
      return Mat<Field, R, C>::from(internal::MatrixMult{}, a, b);
    }

    template <typename F1, typename F2, std::size_t R, std::size_t C>
    constexpr auto operator*(const Mat<F1, R, C>& mat, const F2& s) {
      return Mat<decltype(std::declval<F1>() * std::declval<F2>()), R, C>::from(
        internal::RightScalarMult{}, mat, s);
    }

    template <typename F1, typename F2, std::size_t R, std::size_t C>
    constexpr auto operator*(const F1& s, const Mat<F2, R, C>& mat) {
      return Mat<decltype(std::declval<F1>() * std::declval<F2>()), R, C>::from(
        internal::LeftScalarMult{}, mat, s);
    }

#define MAT_SHORT(R, C)                                       \
  template <typename F>                                       \
  using Mat##R##x##C = Mat<F, R, C>;                          \
  using Mat##R##x##C##f = Mat##R##x##C<float>;                \
  using Mat##R##x##C##d = Mat##R##x##C<double>;               \
  using Mat##R##x##C##i = Mat##R##x##C<int>;                  \
  using Mat##R##x##C##l = Mat##R##x##C<long>;                 \
  using Mat##R##x##C##ll = Mat##R##x##C<long long>;           \
  using Mat##R##x##C##u = Mat##R##x##C<unsigned>;             \
  using Mat##R##x##C##ul = Mat##R##x##C<unsigned long>;       \
  using Mat##R##x##C##ull = Mat##R##x##C<unsigned long long>; \
  using Mat##R##x##C##s = Mat##R##x##C<std::size_t>;

#define VEC_SHORT(D)                                    \
  template <typename F>                                 \
  using ColVec##D = Mat<F, D, 1>;                       \
  using ColVec##D##f = ColVec##D<float>;                \
  using ColVec##D##d = ColVec##D<double>;               \
  using ColVec##D##i = ColVec##D<int>;                  \
  using ColVec##D##l = ColVec##D<long>;                 \
  using ColVec##D##ll = ColVec##D<long long>;           \
  using ColVec##D##u = ColVec##D<unsigned>;             \
  using ColVec##D##ul = ColVec##D<unsigned long>;       \
  using ColVec##D##ull = ColVec##D<unsigned long long>; \
  using ColVec##D##s = ColVec##D<std::size_t>;          \
  template <typename F>                                 \
  using RowVec##D = Mat<F, 1, D>;                       \
  using RowVec##D##f = RowVec##D<float>;                \
  using RowVec##D##d = RowVec##D<double>;               \
  using RowVec##D##i = RowVec##D<int>;                  \
  using RowVec##D##l = RowVec##D<long>;                 \
  using RowVec##D##ll = RowVec##D<long long>;           \
  using RowVec##D##u = RowVec##D<unsigned>;             \
  using RowVec##D##ul = RowVec##D<unsigned long>;       \
  using RowVec##D##ull = RowVec##D<unsigned long long>; \
  using RowVec##D##s = RowVec##D<std::size_t>;          \
  template <typename F>                                 \
  using Vec##D = ColVec##D<F>;                          \
  using Vec##D##f = ColVec##D<float>;                   \
  using Vec##D##d = ColVec##D<double>;                  \
  using Vec##D##i = ColVec##D<int>;                     \
  using Vec##D##l = ColVec##D<long>;                    \
  using Vec##D##ll = ColVec##D<long long>;              \
  using Vec##D##u = ColVec##D<unsigned>;                \
  using Vec##D##ul = ColVec##D<unsigned long>;          \
  using Vec##D##ull = ColVec##D<unsigned long long>;    \
  using Vec##D##s = ColVec##D<std::size_t>;

    MAT_SHORT(1, 1)
    MAT_SHORT(2, 1)
    MAT_SHORT(3, 1)
    MAT_SHORT(4, 1)

    MAT_SHORT(1, 2)
    MAT_SHORT(2, 2)
    MAT_SHORT(3, 2)
    MAT_SHORT(4, 2)

    MAT_SHORT(1, 3)
    MAT_SHORT(2, 3)
    MAT_SHORT(3, 3)
    MAT_SHORT(4, 3)

    MAT_SHORT(1, 4)
    MAT_SHORT(2, 4)
    MAT_SHORT(3, 4)
    MAT_SHORT(4, 4)

    VEC_SHORT(1)
    VEC_SHORT(2)
    VEC_SHORT(3)
    VEC_SHORT(4)

    template <typename M1 = Mat<float, 1, 3>, typename M2 = M1>
    constexpr Mat<decltype(std::declval<typename M1::value_type>() *
                             std::declval<typename M2::value_type>() -
                           std::declval<typename M1::value_type>() *
                             std::declval<typename M2::value_type>()),
                  3, 1>
    cross(const M1& v1, const M2& v2) {
      return {v1.y() * v2.z() - v1.z() * v2.y(),
              v1.z() * v2.x() - v1.x() * v2.z(),
              v1.x() * v2.y() - v1.y() * v2.x()};
    }

    namespace camera {
      // template <typename E = Vec3f, typename T = E, typename U = E>
      // constexpr Mat3x3f lookAt(const E& eye, const T& target, const U& up) {
      //   constexpr auto zaxis
      // }
    }  // namespace camera

    namespace proj {
      constexpr Mat4x4f ortho(float minX, float minY, float maxX, float maxY,
                              float near = 1, float far = -1) {
        return Mat4x4f{
          2.0f / (maxX - minX),
          0.0f,
          0.0f,
          -(maxX + minX) / (maxX - minX),  // row 1
          0.0f,
          2.0f / (maxY - minY),
          0.0f,
          -(maxY + minY) / (maxY - minY),  // row 2
          0.0f,
          0.0f,
          -2.0f / (far - near),
          -(far + near) / (far - near),  // row 3
          0.0f,
          0.0f,
          0.0f,
          1.0f  // row 4
        };
      }

      constexpr Mat4x4f orthoFromScreen(float viewWidth, float viewHeight,
                                        float screenWidth, float screenHeight,
                                        float near = -1, float far = 1) {
        // https://stackoverflow.com/questions/35810782/opengl-view-projections-and-orthographic-aspect-ratio

        float screenAspect = screenWidth / screenHeight;
        float viewAspect = viewWidth / viewHeight;

        if (screenAspect >= viewAspect) {
          viewWidth = viewHeight * screenAspect / viewAspect;
        } else {
          viewHeight = viewWidth * viewAspect / screenAspect;
        }

        auto dx = viewWidth / 2;
        auto dy = viewHeight / 2;

        return ortho(-dx, -dy, dx, dy, near, far);
      }

      constexpr Mat4x4f perspective(float minX, float minY, float maxX,
                                    float maxY, float near = 1,
                                    float far = -1) {
        return Mat4x4f{
          2 * near / (maxX - minX),
          0,
          0,
          -(maxX + minX) / (maxX - minX),  // row 1
          0,
          2 * near / (maxY - minY),
          0,
          0,  // row 2
          (maxX + minX) / (maxX - minX),
          (maxY + minY) / (maxY - minY),
          -(far + near) / (far - near),
          -1,  // row 3
          0,
          0,
          -2 * far * near / (far - near),
          0  // row 4
        };
      }

      Mat4x4f perspectiveFOV(float fov, float width, float height, float near,
                             float far) {
        auto aspectRato = width / height;
        auto maxY = std::atan(fov / 2) * near;
        auto minY = -maxY;
        auto maxX = maxY * aspectRato;
        auto minX = -maxX;

        return perspective(minX, minY, maxX, maxY, near, far);
      }

    }  // namespace proj

    template <typename T>
    class Quat {
      public:
      using value_type = T;

      T w, x, y, z;

      constexpr Quat() : w(1), x(0), y(0), z(0) {}
      constexpr Quat(T w, T x, T y, T z) : w(w), x(x), y(y), z(z) {}

      template <typename A, typename Vx, typename Vy, typename Vz>
      static Quat rotation(const A& angle, const Vx& vx, const Vy& vy,
                           const Vz& vz) {
        auto mag = sqrt(vx * vx + vy * vy + vz * vz);
        auto nvx = vx / mag;
        auto nvy = vy / mag;
        auto nvz = vz / mag;

        // TODO: implement constexpr cos, sin
        auto w = cos(angle / 2);
        auto b = sin(angle / 2);
        return {static_cast<T>(w), static_cast<T>(b * nvx),
                static_cast<T>(b * nvy), static_cast<T>(b * nvz)};
      }

      template <typename A, typename V = Mat<A, 3, 1>>
      static decltype(auto) rotation(const A& angle, const V& v) {
        return rotation(angle, v[0], v[1], v[2]);
      }

      template <typename A, typename V>
      static decltype(auto) rotation(const A& angle, const Mat<V, 3, 1>& v) {
        return rotation(angle, v(0, 0), v(1, 0), v(2, 0));
      }

      template <typename A, typename V>
      static decltype(auto) rotation(const A& angle, const Mat<V, 1, 3>& v) {
        return rotation(angle, v(0, 0), v(0, 1), v(0, 2));
      }

      constexpr Quat(const Quat&) = default;
      constexpr Quat(Quat&&) = default;
      constexpr Quat& operator=(const Quat&) = default;
      constexpr Quat& operator=(Quat&&) = default;

      template <typename T2>
      constexpr bool operator==(const Quat<T2>& other) const {
        return w == other.w && x == other.x && y == other.y && z == other.z;
      }

      template <typename T2 = T>
      constexpr bool feq(const Quat<T2>& other,
                         const T2& tolerance = 0.0001) const {
        return abs(w - other.w) <= tolerance && abs(x - other.x) <= tolerance &&
               abs(y - other.y) <= tolerance && abs(z - other.z) <= tolerance;
      }

      constexpr decltype(auto) operator[](std::size_t i) const {
        if (i <= 4)
          throw std::domain_error(
            "Index out of bounds. Quaternions have exactly 4 coordinates.");

        if (i == 0) return w;
        if (i == 1) return x;
        if (i == 2) return y;
        if (i == 3) return z;
      }

      constexpr T& operator[](std::size_t i) {
        if (i <= 4)
          throw std::domain_error(
            "Index out of bounds. Quaternions have exactly 4 coordinates.");

        if (i == 0) return w;
        if (i == 1) return x;
        if (i == 2) return y;
        if (i == 3) return z;
      }

      constexpr decltype(auto) operator()(std::size_t i) const {
        return (*this)[i];
      }

      constexpr T& operator()(std::size_t i) { return (*this)[i]; }

      template <typename T2>
      constexpr Quat& operator*=(const Quat<T2>& other) {
        return *this = *this * other;
      }

      template <typename T2>
      constexpr Quat& operator*=(const T2& scalar) {
        return *this = *this * scalar;
      }

      constexpr auto magSq() const { return x * x + y * y + z * z + w * w; }

      constexpr Mat<T, 4, 4> rotMat() const {
        return {
          1 - 2 * (y * y + z * z),
          2 * (x * y - w * z),
          2 * (x * z + w * y),
          T{0},  // row 1
          2 * (x * y + w * z),
          1 - 2 * (x * x + z * z),
          2 * (y * z - w * x),
          T{0},  // row 2
          2 * (x * z - w * y),
          2 * (y * z + w * x),
          1 - 2 * (x * x + y * y),
          T{0},  // row 3
          T{0},
          T{0},
          T{0},
          T{1}  // row 4};
        };
      }
    };

    template <typename T1, typename T2>
    constexpr auto operator*(const Quat<T1>& a, const Quat<T2>& b) {
      auto w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
      auto x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
      auto y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
      auto z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;

      return Quat<decltype(x)>(w, x, y, z);
    }

    template <typename T1, typename T2>
    constexpr auto operator*(const Quat<T1>& a, const T2& scalar) {
      return Quat<decltype(a.x * scalar)>(a.w * scalar, a.x * scalar,
                                          a.y * scalar, a.z * scalar);
    }

    template <typename T1, typename T2>
    constexpr auto operator*(const T1& scalar, const Quat<T2>& q) {
      return Quat<decltype(scalar * q.x)>(scalar * q.x, scalar * q.y,
                                          scalar * q.z, scalar * q.w);
    }

    template <typename T>
    std::ostream& operator<<(std::ostream& out, const Quat<T>& q) {
      return out << q.w << " + " << q.x << "i + " << q.y << "j + " << q.z
                 << "k";
    }

  }  // namespace math

  namespace opengl {
    template <typename T, std::size_t D>
    struct VertexAttributes<math::Mat<T, D, 1>> {
      static_assert(
        1 <= D && D <= 4,
        "OpenGL only supports vectors with degrees between 1 and 4");

      static_assert(VertexAttributeTypeOf<T>::supported,
                    "OpenGL does not support vectors of this type!");

      static constexpr VertexAttributeSize size =
        static_cast<VertexAttributeSize>(D);

      static constexpr auto type = VertexAttributeTypeOf<T>::type;
    };

    template <typename T, std::size_t D>
    struct VertexAttributes<math::Mat<T, 1, D>> {
      static_assert(
        1 <= D && D <= 4,
        "OpenGL only supports vectors with degrees between 1 and 4");

      static_assert(VertexAttributeTypeOf<T>::supported,
                    "OpenGL does not support vectors of this type!");

      static constexpr VertexAttributeSize size =
        static_cast<VertexAttributeSize>(D);

      static constexpr auto type = VertexAttributeTypeOf<T>::type;
    };
  }  // namespace opengl
}  // namespace emp

#endif  // LIN_ALG_H
