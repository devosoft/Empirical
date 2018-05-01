#ifndef EMP_RESOURCES_H
#define EMP_RESOURCES_H

#include <algorithm>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

// Interesting things about this:
// 1. Allows for lazy loading of resources
// 3. Con: this probably does not optimize well.
namespace emp {
  template <typename T>
  class ResourceRef;

  class ResourceEmptyError : public std::runtime_error {
    public:
    ResourceEmptyError()
      : runtime_error("Attempting to use an empty resource") {}
  };

  template <typename T>
  class Resource {
    using lazy_t = std::function<T()>;
    // TODO: these should probably be atomic?
    std::string name;
    mutable enum { Empty, Lazy, Value } state;
    mutable union value_t {
      // Functions are actually pretty big, so we want to store a pointer to it
      // instead. That way, it should not increase the size of value_t beyond
      // sizeof(void*)
      lazy_t* lazy;
      T value;
      // Note that when this is called, lazy will be set to point to garbage. We
      // rely on the enclosing Resource instance to make this safe, by ensuring
      // that this initial value is never referenced.
      value_t() {}
      // Note that this is *not* sufficient to destroy the value, and *must* be
      // managed by resource
      ~value_t() {}

    } value;
    std::vector<ResourceRef<T>*> references;

    void DispatchUnsetEvent() const {
      for (auto& reference : references) {
        reference->Unset();
      }
    }
    void DispatchSetEvent() const {
      for (auto& reference : references) {
        reference->Set();
      }
    }

    void FreeConst() const {
      auto old_state = state;
      state = Empty;
      switch (old_state) {
        case Empty:
          break;
        case Lazy:
          // We call unset *before* running the desctructor to ensure that all
          // the references will know that their value is invalid before it is
          // destroyed.
          DispatchUnsetEvent();
          delete value.lazy;
          break;
        case Value:
          DispatchUnsetEvent();
          value.value.~T();
          break;
      }
    }
    template <typename... U>
    void EmplaceConst(U&&... args) const {
      // Destroy anything that used to be here
      FreeConst();
      // Place the new value into the union
      new (&this->value.value) T(std::forward<U>(args)...);
      state = Value;
      DispatchSetEvent();
    }

    const T* Compute() const {
      // THIS ASSUMES THAT THE RESOURCE IS CURRENTLY LAZY!
      EmplaceConst((*value.lazy)());
      return &value.value;
    }
    T* Compute() {
      // THIS ASSUMES THAT THE RESOURCE IS CURRENTLY LAZY!
      Emplace((*value.lazy)());
      return &value.value;
    }

    friend ResourceRef<T>;

    public:
    Resource(const Resource&) = delete;
    Resource(Resource&&) = delete;

    Resource& operator=(const Resource&) = delete;
    Resource& operator=(Resource&&) = delete;

    Resource(const std::string& name) : name(name) {}

    Resource(const std::string& name, const lazy_t& lazy)
      : name(name), state(Lazy) {
      this->value.lazy = new lazy_t(lazy);
    }
    Resource(const std::string& name, lazy_t&& lazy) : name(name), state(Lazy) {
      this->value.lazy = new lazy_t(std::move(lazy));
    }

    Resource(const std::string& name, const T& value)
      : name(name), state(Value) {
      new (&this->value.value) T(value);
      DispatchSetEvent();
    }
    Resource(const std::string& name, T&& value) : name(name), state(Value) {
      new (&this->value.value) T(std::move(value));
      DispatchSetEvent();
    }

    ~Resource() { Free(); }

    template <typename... U>
    Resource& Emplace(U&&... args) {
      EmplaceConst(std::forward<U>(args)...);
      return *this;
    }

    Resource& operator=(const T& value) {
      // If this resource is not currently a value, we need to *construct* a new
      // value for it.
      if (!IsValue()) {
        Emplace(value);
      } else {
        // Otherwise, we can assume that value's operator= is correctly
        // implemented
        DispatchUnsetEvent();
        this->value.value = value;
        DispatchSetEvent();
      }
      return *this;
    }

    Resource& operator=(T&& value) {
      // If this resource is not currently a value, we need to *construct* a new
      // value for it.
      if (!IsValue()) {
        Emplace(value);
      } else {
        // Otherwise, we can assume that value's operator= is correctly
        // implemented
        DispatchUnsetEvent();
        this->value.value = std::move(value);
        DispatchSetEvent();
      }
      return *this;
    }

    bool IsEmpty() const { return state == Empty; }
    bool IsLazy() const { return state == Lazy; }
    bool IsValue() const { return state == Value; }

    void Free() { FreeConst(); }

    const T& operator*() const {
      if (IsEmpty()) {
        throw ResourceEmptyError();
      }
      return IsValue() ? value.value : *Compute();
    }

    T& operator*() {
      if (IsEmpty()) {
        throw ResourceEmptyError();
      }
      return IsValue() ? value.value : *Compute();
    }

    const T* Ptr() const {
      if (IsEmpty()) {
        throw ResourceEmptyError();
      }
      return IsValue() ? &value.value : Compute();
    }
    T* Ptr() {
      if (IsEmpty()) {
        throw ResourceEmptyError();
      }
      return IsValue() ? &value.value : Compute();
    }

    const T* operator->() const { return Ptr(); }
    T* operator->() { return Ptr(); }

    operator bool() const { return state != Empty; }
  };

  template <typename T>
  class Resources {
    private:
    // static std::unordered_map<std::string, std::function<T()>>
    // lazy_resources;
    static std::unordered_map<std::string, Resource<T>> resources;

    friend ResourceRef<T>;

    public:
    Resources() = delete;
    Resources(const Resources&) = delete;
    Resources(Resources&&) = delete;
    Resources& operator=(const Resources&) = delete;
    Resources& operator=(Resources&&) = delete;

    template <typename N, typename... U>
    static void Add(const N& name, U&&... args) {
      emp_assert(resources.count(name) == 0);
      resources.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                        std::forward_as_tuple(name, std::forward<U>(args)...));
    }

    template <typename S>
    static Resource<T>& Get(const S& name) {
      emp_assert(resources.count(name) != 0);
      return resources.at(name);
    }
  };

  template <typename T>
  class ResourceRef {
    private:
    Resource<T>* resource = nullptr;
    std::vector<std::function<void()>> on_unset;
    std::vector<std::function<void(T&)>> on_set;

    void Unset() {
      for (auto& unsetter : on_unset) unsetter();
    }

    void Set() {
      for (auto& setter : on_set) setter(**resource);
    }

    public:
    ResourceRef() {}

    ResourceRef(Resource<T>& resource) { Ref(resource); }
    ResourceRef(const std::string& name) { Ref(name); }

    ResourceRef(const ResourceRef&) = default;
    ResourceRef(ResourceRef&&) = default;
    ResourceRef& operator=(const ResourceRef&) = default;
    ResourceRef& operator=(ResourceRef&&) = default;

    ~ResourceRef() { Ref(nullptr); }

    void Ref(const std::string& name) { Ref(Resources<T>::Get(name)); }
    void Ref(Resource<T>& target) { Ref(&target); }
    void Ref(ResourceRef<T>& target) { Ref(target.resource); }

    void Ref(Resource<T>* new_resource) {
      if (resource == new_resource) return;
      // Remove this reference from the old resource it pointed to
      if (resource != nullptr) {
        auto begin{std::begin(resource->references)};
        auto end{std::end(resource->references)};
        auto pos = std::find(begin, end, this);
        if (pos != end) {
          *pos = resource->references.back();
          resource->references.pop_back();
        }

        if (resource->IsValue()) Unset();
      }

      resource = new_resource;
      // Add this reference to the new resource
      if (resource != nullptr) {
        resource->references.push_back(this);
        if (resource->IsValue()) Set();
      }
    }

    const T& operator*() const { return **resource; }
    T& operator*() { return **resource; }

    const T* operator->() const { return resource->Ptr(); }
    T* operator->() { return resource->Ptr(); }

    template <typename F>
    void OnUnset(F&& function) {
      on_unset.emplace_back(std::forward<F>(function));
    }

    template <typename F>
    void OnSet(F&& function) {
      on_set.emplace_back(std::forward<F>(function));
    }

    friend Resource<T>;
  };

  template <typename T>
  std::unordered_map<std::string, Resource<T>> Resources<T>::resources;

}  // namespace emp

#endif  // EMP_RESOURCES_H
