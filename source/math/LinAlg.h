#ifndef LIN_ALG_H
#define LIN_ALG_H

#include <algorithm>
#include <array>
#include <ostream>

#include "base/assert.h"

namespace emp {

  namespace math {
    template <typename F, std::size_t R, std::size_t C>
    class Mat;

    namespace internal {
      template <std::size_t R, std::size_t C, typename F, std::size_t... I>
      constexpr std::array<F, R * C> unfoldArrayImpl(
        const std::array<std::array<F, C>, R>& data,
        const std::index_sequence<I...>&) {
        return {data[I / C, I % C]...};
      }
      template <std::size_t R, std::size_t C, typename F>
      constexpr decltype(auto) unfoldArray(
        const std::array<std::array<F, C>, R>& data) {
        return unfoldArrayImpl(data, std::make_index_sequence<R * C>{});
      }

    }  // namespace internal

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

      constexpr F& operator()(std::size_t r, std::size_t c) {
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

      constexpr F* data() noexcept { return arrayData.data(); }
      constexpr const F* data() const noexcept { return arrayData.data(); }

      constexpr auto transpose() const {
        return MatUtils<C, R>::generate(
          [this](std::size_t r, std::size_t c) { return (*this)(c, r); });
      }

      template <typename G = F>
      constexpr bool feq(const Mat<G, R, C>& other,
                         const G& tolerance = 0.0001) const {
        for (std::size_t i = 0; i < rows * columns; ++i) {
          if (abs(arrayData[i] - other.data()[i]) > tolerance) {
            return false;
          }
        }

        return true;
      }
    };  // class Mat

    template <typename F, std::size_t R, std::size_t C>
    std::ostream& operator<<(std::ostream& out, const Mat<F, R, C>& mat) {
      out << "{" << std::endl;
      for (std::size_t i = 0; i < R; ++i) {
        out << "\t" << mat[i] << "\n";
      }
      return out << "}";
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
      return MatUtils<R, C>::generate(
        [a, b](std::size_t r, std::size_t c) { return a.row(r) * b.col(c); });
    }

    template <typename F>
    using Mat2x2 = Mat<F, 2, 2>;

    template <typename F>
    using Mat3x3 = Mat<F, 3, 3>;

    template <typename F>
    using Mat4x4 = Mat<F, 4, 4>;

    namespace proj {
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
        emp_assert(i <= 4, "Index out of bounds");

        if (i == 0) return w;
        if (i == 1) return x;
        if (i == 2) return y;
        if (i == 3) return z;
      }

      constexpr T& operator[](std::size_t i) {
        emp_assert(i <= 4, "Index out of bounds");

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

}  // namespace emp

#endif  // LIN_ALG_H
