//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//

#include <algorithm>
#include <iostream>
#include <iterator>
#include <limits>

#include "math/LinAlg.h"
#include "math/consts.h"
#include "opengl/defaultShaders.h"
#include "opengl/glcanvas.h"

#include <cstdlib>

namespace gl = emp::opengl;
using namespace emp::math;

template <typename F>
class Region2D {
  public:
  Vec2<F> min;
  Vec2<F> max;

  constexpr Region2D(F minX, F minY, F maxX, F maxY)
    : min{minX, minY}, max{maxX, maxY} {}
  constexpr Region2D(const Vec2<F>& min, const Vec2<F>& max)
    : min{min}, max{max} {}

  constexpr Region2D(const Region2D&) = default;
  constexpr Region2D(Region2D&&) = default;

  constexpr Region2D& operator=(const Region2D&) = default;
  constexpr Region2D& operator=(Region2D&&) = default;

  constexpr decltype(auto) width() const { return max.x - min.x; }
  constexpr decltype(auto) height() const { return max.y - min.y; }
  constexpr decltype(auto) size() const { return max - min; }

  Vec2<F> rescale(const Vec2<F>& value, const Region2D<F>& from) const {
    return {
      ((value.x - from.min.x) / from.width()) * width() + min.x,
      ((value.y - from.min.y) / from.height()) * height() + min.y,
    };
  }
};

namespace detail {
  template <typename U, typename T>
  struct TupleHas : std::false_type {};

  template <typename U, typename... T>
  struct TupleHas<U, std::tuple<U, T...>> : std::true_type {};

  template <typename U, typename H, typename... T>
  struct TupleHas<U, std::tuple<H, T...>> : TupleHas<U, std::tuple<T...>> {};

  template <size_t I, typename U, typename T>
  struct IndexOfImpl;

  template <size_t I, typename U, typename... T>
  struct IndexOfImpl<I, U, std::tuple<U, T...>>
    : std::integral_constant<size_t, I> {};

  template <size_t I, typename U, typename H, typename... T>
  struct IndexOfImpl<I, U, std::tuple<H, T...>>
    : IndexOfImpl<I + 1, U, std::tuple<T...>> {};

  template <typename U, typename T>
  struct IndexOf : IndexOfImpl<0, U, T> {};

}  // namespace detail

namespace properties {
  struct Fill {};
  struct Stroke {};
  struct X {};
  struct Y {};
  struct Parent {};

};  // namespace properties

template <typename K, typename P>
class Bundle;

template <typename... K, typename... P>
class Bundle<std::tuple<K...>, std::tuple<P...>> {
  private:
  using keys_type = std::tuple<K...>;
  using properties_type = std::tuple<P...>;

  using decayed_keys_type = std::tuple<std::decay_t<K>...>;
  using decayed_properties_type = std::tuple<std::decay_t<P>...>;

  properties_type properties;

  public:
  template <typename U>
  static constexpr bool has() {
    return detail::TupleHas<std::decay_t<U>, decayed_keys_type>::value;
  }

  template <typename U>
  constexpr decltype(auto) get() const {
    static_assert(Bundle::has<U>(), "No such property");
    constexpr auto index =
      detail::IndexOf<std::decay_t<U>, decayed_keys_type>::value;
    return std::get<index>(properties);
  }

  template <typename U>
  constexpr decltype(auto) get() {
    static_assert(Bundle::has<U>(), "No such property");
    constexpr auto index =
      detail::IndexOf<std::decay_t<U>, decayed_keys_type>::value;
    return std::get<index>(properties);
  }

  template <typename U, typename D>
  constexpr decltype(auto) get(
    D&& defaultCallback, std::enable_if<Bundle::has<U>(), char> = 0) const {
    return get<U>();
  }

  template <typename U, typename D>
  constexpr decltype(auto) get(
    D&& defaultCallback, std::enable_if<!Bundle::has<U>(), char> = 0) const {
    return std::forward<D>(defaultCallback)();
  }

  template <typename U, typename D>
  constexpr decltype(auto) get(D&& defaultCallback,
                               std::enable_if<Bundle::has<U>(), char> = 0) {
    return get<U>();
  }

  template <typename U, typename D>
  constexpr decltype(auto) get(D&& defaultCallback,
                               std::enable_if<!Bundle::has<U>(), char> = 0) {
    return std::forward<D>(defaultCallback)();
  }

  template <typename U>
  constexpr auto set(U&& value) -> std::enable_if<Bundle::has<U>(), Bundle> {
    get<U>() = std::forward<U>(value);

    return std::move(*this);
  }

  template <typename Key, typename Value>
  constexpr auto set(Value&& value)
    -> std::enable_if<!Bundle::has<Key>(),
                      Bundle<std::tuple<Key, K...>, std::tuple<Value, P...>>> {
    return std::tuple_cat(std::move(properties),
                          std::forward_as_tuple<Value>(value));
  }
};

template <typename T>
class Scatter {
  private:
  gl::shaders::SimpleSolidColor shader;

  std::function<Vec4f(const T&)> color;

  public:
  Scatter(gl::GLCanvas& canvas, std::function<Vec4f(const T&)> color)
    : shader(canvas), color(color) {
    shader.vao.bind();
    shader.shader.use();
    shader.vao.getBuffer<gl::BufferType::Array>().set(
      {Vec3f{-5, +5, 0}, Vec3f{+5, +5, 0}, Vec3f{+5, -5, 0}, Vec3f{-5, -5, 0}},
      gl::BufferUsage::StaticDraw);
    shader.vao.getBuffer<gl::BufferType::ElementArray>().set(
      {
        0, 1, 2,  // First Triangle
        2, 3, 0   // Second Triangle
      },
      gl::BufferUsage::StaticDraw);
  }

  template <typename Iter>
  void show(const Mat4x4f& projection, const Mat4x4f& view, Iter begin,
            Iter end) {
    shader.vao.bind();
    shader.shader.use();
    shader.shader.use();
    shader.proj = projection;
    shader.view = view;

    for (auto iter = begin; iter != end; ++iter) {
      shader.model = Mat4x4f::translation(iter->x, iter->y);
      shader.color = color(iter->value);

      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
  }
};

template <typename T>
class Line {
  private:
  gl::shaders::SimpleSolidColor shader;

  public:
  Line(gl::GLCanvas& canvas) : shader(canvas) {}

  template <typename Iter>
  void show(const Mat4x4f& projection, const Mat4x4f& view, Iter begin,
            Iter end) {
    if (begin == end) return;
    Vec2f last{begin->x, begin->y};
    ++begin;
    if (begin == end) return;

    shader.vao.bind();
    shader.shader.use();
    shader.proj = projection;
    shader.view = view;
    shader.color = Vec4f{1, 1, 0, 1};
    shader.model = Mat4x4f::translation(0, 0);

    std::vector<Vec3f> verts{{last.x(), last.y() + 2, 0},
                             {last.x(), last.y() - 2, 0}};
    std::vector<GLuint> triangles;
    size_t i = 0;
    for (auto iter = begin; iter != end; ++iter) {
      verts.emplace_back(iter->x, iter->y + 2, 0);
      verts.emplace_back(iter->x, iter->y - 2, 0);

      triangles.push_back(i);
      triangles.push_back(i + 1);
      triangles.push_back(i + 2);

      triangles.push_back(i + 2);
      triangles.push_back(i + 3);
      triangles.push_back(i + 1);
      i += 2;
    }

    shader.vao.getBuffer<gl::BufferType::Array>().set(
      verts, gl::BufferUsage::DynamicDraw);
    shader.vao.getBuffer<gl::BufferType::ElementArray>().set(
      triangles, gl::BufferUsage::DynamicDraw);

    glDrawElements(GL_TRIANGLES, triangles.size(), GL_UNSIGNED_INT, 0);
  }
};

namespace __impl {
  template <typename F, typename T>
  void allDo(F&& callback, T&& tuple, const std::index_sequence<>&) {}

  template <typename F, typename T, size_t H>
  void allDo(F&& callback, T&& tuple, const std::index_sequence<H>&) {
    std::forward<F>(callback)(std::get<H>(std::forward<T>(tuple)));
  }

  template <typename F, typename T, size_t H, size_t H2, size_t... I>
  void allDo(F&& callback, T&& tuple, const std::index_sequence<H, H2, I...>&) {
    callback(std::get<H>(tuple));
    allDo(std::forward<F>(callback), std::forward<T>(tuple),
          std::index_sequence<H2, I...>{});
  }

}  // namespace __impl

template <typename F, typename T>
void allDo(F&& callback, T&& tuple) {
  __impl::allDo(std::forward<F>(callback), std::forward<T>(tuple),
                std::make_index_sequence<
                  std::tuple_size<typename std::decay<T>::type>::value>{});
}

template <typename T, typename... L>
class DataSet {
  public:
  std::function<float(const T&)> x;
  std::function<float(const T&)> y;

  struct DataPoint {
    const T& value;
    float x;
    float y;
  };

  std::tuple<L...> layers;

  public:
  template <typename X, typename Y, typename... L1>
  DataSet(X&& x, Y&& y, L1&&... layers)
    : x(std::forward<X>(x)),
      y(std::forward<Y>(y)),
      layers(std::forward<L1>(layers)...) {}

  template <typename Iter>
  void show(const Mat4x4f& projection, const Mat4x4f& view, Iter begin,
            Iter end) {
    std::vector<DataPoint> dataPoints;
    std::transform(begin, end, std::back_inserter(dataPoints),
                   [this](const T& data) {
                     return DataPoint{data, this->x(data), this->y(data)};
                   });

    // float dminX = std::numeric_limits<float>::max();
    // float dminY = std::numeric_limits<float>::max();
    //
    // float dmaxX = std::numeric_limits<float>::lowest();
    // float dmaxY = std::numeric_limits<float>::lowest();
    //
    // for (auto& dp : dataPoints) {
    //   if (dp.x > dmaxX) dmaxX = dp.x;
    //   if (dp.x < dminX) dminX = dp.x;
    //
    //   if (dp.y > dmaxY) dmaxY = dp.y;
    //   if (dp.y < dminY) dminY = dp.y;
    // }
    allDo(
      [&](auto&& layer) {
        std::forward<decltype(layer)>(layer).show(
          projection, view, dataPoints.begin(), dataPoints.end());
      },
      layers);
  }
};

template <typename T, typename X, typename Y, typename... L>
DataSet<T, L...> dataset(X&& x, Y&& y, L&&... layers) {
  return DataSet<T, L...>(std::forward<X>(x), std::forward<Y>(y),
                          std::forward<L>(layers)...);
}

int main(int argc, char* argv[]) {
  gl::GLCanvas canvas(1000, 1000);

  auto graph = dataset<Vec2f>(
    [](auto& v) { return v.x(); }, [](auto& v) { return v.y(); },
    Line<Vec2f>{canvas}, Scatter<Vec2f>{canvas, [](auto&) {
                                          return Vec4f{1, 0, 1, 1};
                                        }});

  std::vector<Vec2f> data;
  for (int i = 0; i < 100; ++i) {
    data.emplace_back(i * 10 - 50, 200 * (rand() / (float)RAND_MAX) - 100);
  }

  auto proj =
    proj::orthoFromScreen(1000, 1000, canvas.getWidth(), canvas.getHeight());
  auto view = Mat4x4f::translation(0, 0, 0);

  canvas.runForever([&](auto&&) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    graph.show(proj, view, data.begin(), data.end());
  });

  return 0;
}
