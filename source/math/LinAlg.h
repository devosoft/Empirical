#ifndef LIN_ALG_H
#define LIN_ALG_H

#include <algorithm>
#include <array>

#include "base/assert.h"

namespace emp {

  namespace math {
    template <typename F, std::size_t R, std::size_t C>
    class Mat;

    template <std::size_t R, std::size_t C>
    class MatUtils {
      private:
      template <std::size_t... S, typename G>
      constexpr static auto genMatImpl(const std::index_sequence<S...>&,
                                       G&& generator)
        -> Mat<decltype(generator(std::declval<std::size_t>(),
                                  std::declval<std::size_t>())),
               R, C> {
        return {generator(S / C, S % C)...};
      }

      template <typename T>
      static constexpr T identGenerator(std::size_t r, std::size_t c) {
        return r == c ? T{1} : T{0};
      }

      public:
      template <typename G>
      constexpr static decltype(auto) generate(G&& generator) {
        return genMatImpl(std::make_index_sequence<R * C>{},
                          std::forward<G>(generator));
      }

      template <typename T>
      constexpr static auto identity{generate(&identGenerator<T>)};
    };

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
    };

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
      using value_type = F;
      static constexpr auto rows = R;
      static constexpr auto columns = C;

      protected:
      std::array<F, rows * columns> arrayData;

      public:
      template <typename... Args>
      constexpr Mat(Args&&... args) : arrayData{{std::forward<Args>(args)...}} {
        static_assert(
          sizeof...(Args) == rows * columns,
          "Invalid number of arguments for a matrix of the given size");
      }

      constexpr Mat(const Mat&) = default;
      constexpr Mat(Mat&&) = default;
      Mat& operator=(const Mat&) = default;
      Mat& operator=(Mat&&) = default;

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
        emp_assert(r < rows, "rows out of bounds");
        emp_assert(c < columns, "columns out of bounds");
        return arrayData[r * columns + c];
      }

      constexpr Row<const F, C> row(std::size_t r) const {
        emp_assert(r < rows, "rows out of bounds");
        return Row<const F, C>(&arrayData[r * columns]);
      }

      constexpr Row<F, C> row(std::size_t r) {
        emp_assert(r < rows, "rows out of bounds");
        return Row<F, C>(&arrayData[r * columns]);
      }

      constexpr Col<const F, R> col(std::size_t c) const {
        emp_assert(c < columns, "columns out of bounds");
        return Col<const F, R>(&arrayData[c]);
      }

      constexpr Col<F, R> col(std::size_t c) {
        emp_assert(c < columns, "columns out of bounds");
        return Col<F, R>(&arrayData[c]);
      }

      constexpr F* data() noexcept { return arrayData.data(); }
      constexpr const F* data() const noexcept { return arrayData.data(); }

      constexpr auto transpose() const {
        return MatUtils<C, R>::generate(
          [this](std::size_t r, std::size_t c) { return (*this)(c, r); });
      }
    };  // class Mat

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
      return MatUtils<R, C>::generate(
        [a, b](std::size_t r, std::size_t c) { return a.row(r) * b.col(c); });
    }

    template <typename F>
    using Mat2x2 = Mat<F, 2, 2>;

    template <typename F>
    using Mat3x3 = Mat<F, 3, 3>;

    template <typename F>
    using Mat4x4 = Mat<F, 4, 4>;

    namespace gl {
      Mat<float, 4, 4> ortho(float left, float right, float bottom, float top,
                             float near, float far) {
        return Mat<float, 4, 4>{
          2.0f / (right - left),
          0.0f,
          0.0f,
          -(right + left) / (right - left),  // row 1
          0.0f,
          2.0f / (top - bottom),
          0.0f,
          -(top + bottom) / (top - bottom),  // row 2
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

    }  // namespace gl
  }    // namespace math

}  // namespace emp

#endif  // LIN_ALG_H
