#ifndef EMP_RESOURCES_H
#define EMP_RESOURCES_H

#include <functional>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

namespace emp {
  template <typename T>
  class Resources {
    private:
    static std::unordered_map<std::string, std::function<T()>> lazy_resources;
    static std::unordered_map<std::string, T> resources;

    public:
    Resources() = delete;
    Resources(const Resources&) = delete;
    Resources(Resources&&) = delete;
    Resources& operator=(const Resources&) = delete;
    Resources& operator=(Resources&&) = delete;

    template <typename N, typename... U>
    static void Add(const N& name, U&&... args) {
      if (lazy_resources.count(name) ||
          resources
            .emplace(std::piecewise_construct, std::forward_as_tuple(name),
                     std::forward_as_tuple(std::forward<U>(args)...))
            .second) {
        std::stringstream msg;
        msg << "Cannot add new resource named " << name
            << ": a resource with that name is already added";
        throw std::logic_error(msg.str().c_str());
      }
    }

    template <typename N, typename F>
    static void Lazy(N&& name, F&& factory) {
      if (resources.count(name) ||
          lazy_resources
            .emplace(std::piecewise_construct, std::forward_as_tuple(name),
                     std::forward_as_tuple(std::forward<F>(factory)))
            .second) {
        std::stringstream msg;
        msg << "Cannot add new lazy resource named " << name
            << ": a resource with that name is already added";
      }
    }

    static T& Get(const std::string& name) {
      auto iter = resources.find(name);
      if (iter != resources.end()) {
        return iter->second;
      }

      std::function<T()> factory{std::move(lazy_resources.at(name))};
      // Note that, even though we have move'd the value, we don't need to
      // delete it, since we will now always use the normal resource instead
      auto resource = resources.emplace(std::make_pair(name, factory()));
      return resource.first->second;
    }
  };

  template <typename T>
  std::unordered_map<std::string, T> Resources<T>::resources;

  template <typename T>
  std::unordered_map<std::string, std::function<T()>>
    Resources<T>::lazy_resources;

  template <typename T>
  class StaticResource {
    public:
    template <typename N, typename... U>
    StaticResource(N&& name, U&&... args) {
      Resources<T>::Add(std::forward<N>(name), std::forward<U>(args)...);
    }

    StaticResource() = delete;
    StaticResource(const StaticResource&) = delete;
    StaticResource(StaticResource&&) = delete;
    StaticResource& operator=(const StaticResource&) = delete;
    StaticResource& operator=(StaticResource&&) = delete;
  };

#define STATIC_RESOURCE(TYPE, NAME, ...) \
  emp::StaticResource<TYPE> NAME{#NAME, __VA_ARGS__};

}  // namespace emp

#endif  // EMP_RESOURCES_H
