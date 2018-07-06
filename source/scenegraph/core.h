#ifndef EMP_SCENEGRAPH_CORE_H
#define EMP_SCENEGRAPH_CORE_H

#include <algorithm>
#include <memory>
#include <queue>
#include <vector>

#include "camera.h"
#include "math/LinAlg.h"
#include "math/region.h"
#include "rendering.h"

namespace emp {
  namespace scenegraph {

    class Child {
      public:
      virtual ~Child() {}
      virtual void RenderRelative(emp::graphics::Graphics& g,
                                  const math::Mat4x4f& transform) = 0;
    };

    template <class C = Child>
    class Parent : public Child {
      private:
      void __impl_AttachAll() {}

      template <class Head>
      void __impl_AttachAll(Head&& head) {
        Attach(std::forward<Head>(head));
      }

      template <class Head, class Head2, class... Tail>
      void __impl_AttachAll(Head&& head, Head2&& head2, Tail&&... tail) {
        Attach(std::forward<Head>(head));
        __impl_AttachAll(std::forward<Head2>(head2),
                         std::forward<Tail>(tail)...);
      }

      public:
      virtual ~Parent() {}

      virtual void Attach(std::shared_ptr<C> child) = 0;
      virtual void Detach(std::shared_ptr<C> child) = 0;

      template <class... Args>
      void AttachAll(Args&&... args) {
        __impl_AttachAll(std::forward<Args>(args)...);
      }

      template <typename F, typename... U>
      void Fill(int count, F&& factory, U&&... args) {
        if (count <= 0) return;

        for (int i = 0; i < count - 1; ++i) {
          Attach(factory(i, count, args...));
        }
        Attach(
          std::forward<F>(factory)(count - 1, count, std::forward<U>(args)...));
      }
    };

    class Group : public Parent<Child> {
      private:
      std::vector<std::shared_ptr<Child>> children;

      public:
      virtual ~Group() {}
      void RenderRelative(emp::graphics::Graphics& g,
                          const math::Mat4x4f& transform) override {
        for (auto& child : children) {
          if (child) {
            child->RenderRelative(g, transform);
          }
        }

        children.erase(std::remove_if(children.begin(), children.end(),
                                      [](auto& child) { return !child; }),
                       children.end());
      }

      void Attach(std::shared_ptr<Child> child) override {
        children.push_back(child);
      }

      void Detach(std::shared_ptr<Child> child) override {
        auto pos{std::find(children.begin(), children.end(), child)};
        if (pos != children.end()) {
          children.erase(pos);
        }
      }
    };

    class Stage {
      math::Region3f region;
      std::shared_ptr<Child> root;

      public:
      Stage(const math::Region3f& region) : region(region) {}

      void SetRoot(std::shared_ptr<Child> root) { this->root = root; }

      template <typename C, typename... U>
      std::shared_ptr<C> MakeRoot(U&&... args) {
        auto root = std::make_shared<C>(std::forward<U>(args)...);
        this->root = root;
        return root;
      }

      void Render(emp::graphics::Graphics& g) {
        if (root) {
          root->RenderRelative(g, math::Mat4x4f::Identity());
        }
      }
    };
  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_SCENEGRAPH_CORE_H
