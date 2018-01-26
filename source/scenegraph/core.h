#ifndef EMP_SCENEGRAPH_CORE_H
#define EMP_SCENEGRAPH_CORE_H

#include <algorithm>
#include <memory>
#include <queue>
#include <vector>

#include "renderable.h"

namespace emp {
  namespace scenegraph {

    class Child : public Renderable {
      public:
      virtual ~Child() {}
    };

    template <class C>
    class Parent : public Renderable {
      private:
      template <class... Tail>
      void __impl_attachAll(Tail&&...) {}

      template <class Head>
      void __impl_attachAll(Head&& head) {
        attach(std::forward<Head>(head));
      }

      template <class Head, class Head2, class... Tail>
      void __impl_attachAll(Head&& head, Head2&& head2, Tail&&... tail) {
        attach(std::forward<Head>(head));
        __impl_attachAll(std::forward<Head2>(head2),
                         std::forward<Tail>(tail)...);
      }

      public:
      virtual ~Parent() {}
      void render(const Camera& camera) {
        renderRelative(camera, math::Mat4x4f::identity());
      }

      virtual void attach(std::shared_ptr<C> child) = 0;
      virtual void detach(std::shared_ptr<C> child) = 0;

      template <class... Args>
      void attachAll(Args&&... args) {
        __impl_attachAll(std::forward<Args>(args)...);
      }
    };

    class Group : public Parent<Child>, public Child {
      private:
      std::vector<std::shared_ptr<Child>> children;

      public:
      virtual ~Group() {}
      void renderRelative(const Camera& camera,
                          const math::Mat4x4f& transform) override {
        for (auto& child : children) {
          if (child) {
            child->renderRelative(camera, transform);
          }
        }

        children.erase(std::remove_if(children.begin(), children.end(),
                                      [](auto& child) { return !child; }),
                       children.end());
      }

      void attach(std::shared_ptr<Child> child) override {
        children.push_back(child);
      }

      void detach(std::shared_ptr<Child> child) override {
        auto pos{std::find(children.begin(), children.end(), child)};
        if (pos != children.end()) {
          children.erase(pos);
        }
      }
    };
  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_SCENEGRAPH_CORE_H
