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

  template <typename P>
  struct PropertyName {
    template <typename V>
    static constexpr auto from(V&& map) {
      return [map = std::forward<V>(map)](auto properties) {
        return properties.template set<V>(map(properties));
      };
    }

    template <typename Props>
    static constexpr auto& get(Props& properties) {
      return properties.template get<P>();
    }

    template <typename Props>
    static constexpr const auto& get(const Props& properties) {
      return properties.template get<P>();
    }
  };

  struct Fill : PropertyName<Fill> {};
  struct Stroke : PropertyName<Stroke> {};
  struct X : PropertyName<X> {};
  struct Y : PropertyName<Y> {};
  struct FillShader : PropertyName<FillShader> {};
  struct StrokeShader : PropertyName<StrokeShader> {};
  struct Value : PropertyName<Value> {};
};  // namespace properties

template <typename K, typename P>
class Props;

template <typename... K, typename... P>
class Props<std::tuple<K...>, std::tuple<P...>> {
  private:
  using keys_type = std::tuple<K...>;
  using properties_type = std::tuple<P...>;

  using decayed_keys_type = std::tuple<std::decay_t<K>...>;
  using decayed_properties_type = std::tuple<std::decay_t<P>...>;

  properties_type properties;

  public:
  Props() = default;
  Props(const Props&) = default;
  Props(Props&&) = default;

  Props(const properties_type& properties) : properties(properties) {}
  Props(properties_type&& properties) : properties(std::move(properties)) {}

  Props& operator=(const Props&) = default;
  Props& operator=(Props&&) = default;

  template <typename U>
  static constexpr bool has() {
    return detail::TupleHas<std::decay_t<U>, decayed_keys_type>::value;
  }

  template <typename U>
  constexpr decltype(auto) get() const {
    static_assert(Props::has<U>(), "No such property");
    constexpr auto index =
      detail::IndexOf<std::decay_t<U>, decayed_keys_type>::value;
    return std::get<index>(properties);
  }

  template <typename U>
  constexpr decltype(auto) get() {
    static_assert(Props::has<U>(), "No such property");
    constexpr auto index =
      detail::IndexOf<std::decay_t<U>, decayed_keys_type>::value;
    return std::get<index>(properties);
  }

  template <typename U, typename D>
  constexpr decltype(auto) get(
    D&& defaultCallback, std::enable_if<Props::has<U>(), char> = 0) const {
    return get<U>();
  }

  template <typename U, typename D>
  constexpr decltype(auto) get(
    D&& defaultCallback, std::enable_if<!Props::has<U>(), char> = 0) const {
    return std::forward<D>(defaultCallback)();
  }

  template <typename U, typename D>
  constexpr decltype(auto) get(D&& defaultCallback,
                               std::enable_if<Props::has<U>(), char> = 0) {
    return get<U>();
  }

  template <typename U, typename D>
  constexpr decltype(auto) get(D&& defaultCallback,
                               std::enable_if<!Props::has<U>(), char> = 0) {
    return std::forward<D>(defaultCallback)();
  }

  template <typename U>
  constexpr auto set(U&& value) -> std::enable_if<Props::has<U>(), Props&> {
    get<U>() = std::forward<U>(value);
    return *this;
  }

  template <typename Key, typename Value>
  constexpr auto set(Value value) -> typename std::enable_if<
    !Props::has<Key>(),
    Props<std::tuple<Key, K...>, std::tuple<Value, P...>>>::type {
    return Props<std::tuple<Key, K...>, std::tuple<Value, P...>>{
      std::tuple_cat(std::make_tuple(value), properties)};
  }
};

Props<std::tuple<>, std::tuple<>> nullProps() { return {}; };

class Scatter {
  private:
  gl::shaders::SimpleSolidColor shader;

  public:
  Scatter(gl::GLCanvas& canvas) : shader(canvas) {
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
    shader.shader.use();
    shader.vao.bind();
    shader.proj = projection;
    shader.view = view;

    using namespace properties;

    for (auto iter = begin; iter != end; ++iter) {
      shader.model = Mat4x4f::translation(X::get(*iter), Y::get(*iter));
      shader.color = Fill::get(*iter);

      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
  }
};

class Line {
  private:
  gl::shaders::SimpleSolidColor shader;

  public:
  Line(gl::GLCanvas& canvas) : shader(canvas) {}

  template <typename Iter>
  void show(const Mat4x4f& projection, const Mat4x4f& view, Iter begin,
            Iter end) {
    using namespace properties;

    if (begin == end) return;
    Vec2f last{X::get(*begin), Y::get(*begin)};
    ++begin;
    if (begin == end) return;

    shader.shader.use();
    shader.vao.bind();
    shader.proj = projection;
    shader.view = view;
    shader.color = Vec4f{1, 1, 0, 1};
    shader.model = Mat4x4f::translation(0, 0);

    std::vector<Vec3f> verts{{last.x(), last.y() + 2, 0},
                             {last.x(), last.y() - 2, 0}};
    std::vector<GLuint> triangles;
    size_t i = 0;
    for (auto iter = begin; iter != end; ++iter) {
      verts.emplace_back(X::get(*iter), Y::get(*iter) + 2, 0);
      verts.emplace_back(X::get(*iter), Y::get(*iter) - 2, 0);

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
    shader.vao.unbind();
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

template <typename C, typename T>
class Map {
  public:
  C change;
  T next;

  public:
  template <typename C1, typename T1>
  Map(C1&& change, T1&& next)
    : change(std::forward<C1>(change)), next(std::forward<T1>(next)) {}

  template <typename Iter>
  void show(const Mat4x4f& projection, const Mat4x4f& view, Iter begin,
            Iter end) {
    using data_point_type = decltype(change(*begin));

    std::vector<data_point_type> dataPoints;
    std::transform(begin, end, std::back_inserter(dataPoints), change);
    next.show(projection, view, dataPoints.begin(), dataPoints.end());
  }
};

template <typename C, typename T>
auto map(C&& change, T&& next) {
  return Map<C, T>{std::forward<C>(change), std::forward<T>(next)};
}

template <typename T>
class Graph {
  private:
  T next;

  public:
  template <typename T1>
  Graph(T1&& next) : next(std::forward<T1>(next)) {}

  template <typename Iter>
  void show(const Mat4x4f& projection, const Mat4x4f& view, Iter begin,
            Iter end) {
    auto map = [](auto&& value) {
      return nullProps().set<properties::Value>(value);
    };
    using data_point_type = decltype(map(*begin));

    std::vector<data_point_type> dataPoints;
    std::transform(begin, end, std::back_inserter(dataPoints), map);
    next.show(projection, view, dataPoints.begin(), dataPoints.end());

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
    // allDo(
    //   [&](auto&& layer) {
    //     std::forward<decltype(layer)>(layer).show(
    //       projection, view, dataPoints.begin(), dataPoints.end());
    //   },
    //   layers);
  }
};

template <typename T>
auto graph(T&& next) {
  return Graph<T>{std::forward<T>(next)};
}

int main(int argc, char* argv[]) {
  gl::GLCanvas canvas(1000, 1000);
  using namespace properties;
  auto g = graph(map(
    [](auto& props) {
      auto& value = Value::get(props);

      return props.template set<X>(value.x())
        .template set<Y>(value.y())
        .template set<Fill>(Vec4f{1, 1, 1, 1});
    },
    Scatter(canvas)));

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

    g.show(proj, view, data.begin(), data.end());
  });

  return 0;
}
