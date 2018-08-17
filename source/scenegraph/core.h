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

    template <size_t D>
    class Node {
      public:
      virtual ~Node() {}

      virtual emp::math::Vec<float, D> UpdateMinSize(
        const graphics::Graphics& g) {
        return {0};
      }

      // This should always be called soon before a call to render relative
      virtual emp::math::Vec<float, D> UpdateSize(
        const graphics::Graphics& g, const emp::math::Vec<float, D>& max_size) {
        return max_size;
      }

      virtual void RenderRelative(
        emp::graphics::Graphics& g, const math::Mat4x4f& transform,
        const emp::math::Vec<float, D>& allocated_size) = 0;
    };

    namespace emp__impl_emp_scenegraph_core {

      constexpr decltype(auto) GetMajorAxis(
        emp::math::Vec<float, 2>& dest,
        const emp::math::Vec<float, 2>& source) {}
    }  // namespace emp__impl_emp_scenegraph_core

    template <size_t D>
    class Stack : public Node<D> {
      private:
      std::vector<std::shared_ptr<Node<D>>> children;

      public:
      virtual ~Stack() {}

      void RenderRelative(
        emp::graphics::Graphics& g, const math::Mat4x4f& transform,
        const emp::math::Vec<float, D>& allocated_size) override {
        for (auto& child : children) {
          if (child) {
            child->RenderRelative(g, transform, allocated_size);
          }
        }
      }

      Stack& Append(std::shared_ptr<Node<D>> node) {
        children.emplace_back(node);
        return *this;
      }
    };

    template <size_t>
    struct FlowDirection;

    template <>
    struct FlowDirection<1> {
      enum { X } axis = X;
      bool reversed = false;
    };
    template <>
    struct FlowDirection<2> {
      enum { X, Y } axis = X;
      bool reversed = false;
    };
    template <>
    struct FlowDirection<3> {
      enum { X, Y, Z } axis = X;
      bool reversed = false;
    };

    namespace emp__impl_emp_scenegraph_core {

      template <typename V = emp::math::Vec<float, 2>>
      constexpr decltype(auto) GetMajorAxis(const FlowDirection<2>& direction,
                                            V&& v) {
        switch (direction.axis) {
          case FlowDirection<2>::X:
            return std::forward<V>(v).x();
          case FlowDirection<2>::Y:
            return std::forward<V>(v).y();
        }
      }

      template <typename V = emp::math::Vec<float, 3>>
      constexpr decltype(auto) GetMajorAxis(const FlowDirection<3>& direction,
                                            V&& v) {
        switch (direction.axis) {
          case FlowDirection<3>::X:
            return std::forward<V>(v).x();
          case FlowDirection<3>::Y:
            return std::forward<V>(v).y();
          case FlowDirection<3>::Z:
            return std::forward<V>(v).z();
        }
      }

      constexpr decltype(auto) MaxMinorAxes(const FlowDirection<2>& direction,
                                            emp::math::Vec2f& dest,
                                            const emp::math::Vec2f& source) {
        switch (direction.axis) {
          case FlowDirection<2>::X:
            dest.y() = std::max(dest.y(), source.y());
            break;
          case FlowDirection<2>::Y:
            dest.x() = std::max(dest.x(), source.x());
            break;
        }
      }

      constexpr decltype(auto) MaxMinorAxes(const FlowDirection<3>& direction,
                                            emp::math::Vec3f& dest,
                                            const emp::math::Vec3f& source) {
        switch (direction.axis) {
          case FlowDirection<3>::X:
            dest.y() = std::max(dest.y(), source.y());
            dest.z() = std::max(dest.z(), source.z());
            break;
          case FlowDirection<3>::Y:
            dest.x() = std::max(dest.x(), source.x());
            dest.z() = std::max(dest.z(), source.z());
            break;
          case FlowDirection<3>::Z:
            dest.x() = std::max(dest.x(), source.x());
            dest.y() = std::max(dest.y(), source.y());
            break;
        }
      }

      constexpr emp::math::Region<float, 2> GetMajorAxisRegion(
        const FlowDirection<2>& direction,
        const emp::math::Region<float, 2>& max_region, float major_start,
        float major_length) {
        auto min = max_region.min;
        auto max = max_region.max;

        if (direction.reversed) {
          GetMajorAxis(direction, max) -= major_start;
          GetMajorAxis(direction, min) =
            GetMajorAxis(direction, max) - major_length;
        } else {
          GetMajorAxis(direction, min) += major_start;
          GetMajorAxis(direction, max) =
            GetMajorAxis(direction, min) + major_length;
        }

        return emp::math::Region2f{min, max};
      }

      template <size_t D>
      constexpr emp::math::Vec<float, D> GetSize(
        const FlowDirection<D>& direction,
        const emp::math::Vec<float, D>& max_size, float major_length) {
        auto size = max_size;

        GetMajorAxis(direction, size) = major_length;

        return size;
      }

    }  // namespace emp__impl_emp_scenegraph_core

    template <size_t D>
    struct FlowMember {
      std::shared_ptr<Node<D>> node;
      float weight;
      emp::math::Vec<float, D> min_size;
      emp::math::Vec<float, D> size;

      FlowMember(std::shared_ptr<Node<D>> node, float weight = 1)
        : node(node), weight(weight) {}
    };

    template <size_t D>
    class Flow : public Node<D> {
      private:
      bool expands;
      FlowDirection<D> direction;
      emp::math::Vec<float, D> size;
      std::vector<FlowMember<D>> children;

      public:
      template <typename FD = FlowDirection<D>>
      Flow(bool expands = false, FD&& direction = {FlowDirection<D>::X})
        : expands(expands), direction({std::forward<FD>(direction)}) {}

      virtual ~Flow() {}

      Flow& Append(std::shared_ptr<Node<D>> node, float weight = 1) {
        children.emplace_back(node, weight);
        return *this;
      }

      emp::math::Vec<float, D> UpdateMinSize(
        const graphics::Graphics& g) override {
        emp::math::Vec<float, D> min_size{0};
        for (auto& child : children) {
          if (child.node && child.weight >= 0) {
            child.min_size = child.node->UpdateMinSize(g);

            emp__impl_emp_scenegraph_core::MaxMinorAxes(direction, min_size,
                                                        child.min_size);
            emp__impl_emp_scenegraph_core::GetMajorAxis(direction, min_size) +=
              emp__impl_emp_scenegraph_core::GetMajorAxis(direction,
                                                          child.min_size);
          }
        }

        return min_size;
      }

      emp::math::Vec<float, D> UpdateSize(
        const graphics::Graphics& g,
        const emp::math::Vec<float, D>& max_size) override {
        auto min_size = UpdateMinSize(g);

        float total_weight = 0;
        for (auto& child : children) {
          if (child.node) {
            total_weight += std::abs(child.weight);
          }
        }

        auto free_space_size = max_size - min_size;

        float major_axis_length = emp__impl_emp_scenegraph_core::GetMajorAxis(
          direction, free_space_size);

        size = {0};

        for (auto& child : children) {
          if (child.node) {
            // Each child is given as much room as it wants on the minor axes,
            // and on the major axis it gets (free space) * (its share) + (the
            // minimum space it needs)

            // If a negative weight it given, the child is not allocated a
            // minimum space, and will be given as much of the free space as it
            // can get
            float min_space_length =
              child.weight < 0 ? 0
                               : emp__impl_emp_scenegraph_core::GetMajorAxis(
                                   direction, child.min_size);

            // Don't divide by zero! There must not have been anyone who asked
            // for a share of the free space
            float free_space_length =
              total_weight == 0
                ? 0
                : major_axis_length * child.weight / total_weight;

            auto child_size = expands ? max_size : min_size;
            emp__impl_emp_scenegraph_core::GetMajorAxis(direction, child_size) =
              min_space_length + free_space_length;

            child.size = child.node->UpdateSize(g, child_size);

            // Update the total size of this container
            emp__impl_emp_scenegraph_core::MaxMinorAxes(direction, size,
                                                        child.size);
            emp__impl_emp_scenegraph_core::GetMajorAxis(direction, size) +=
              emp__impl_emp_scenegraph_core::GetMajorAxis(direction,
                                                          child.size);
          }
        }

        return expands ? max_size : size;
      }

      void RenderRelative(
        emp::graphics::Graphics& g, const math::Mat4x4f& transform,
        const emp::math::Vec<float, D>& allocated_size) override {
        emp::math::Vec<float, D> position;
        emp__impl_emp_scenegraph_core::GetMajorAxis(direction, position) =
          emp__impl_emp_scenegraph_core::GetMajorAxis(direction,
                                                      allocated_size);

        for (auto& child : children) {
          if (child.node) {
            emp__impl_emp_scenegraph_core::GetMajorAxis(direction, position) -=
              emp__impl_emp_scenegraph_core::GetMajorAxis(direction,
                                                          child.size);
            child.node->RenderRelative(
              g, math::Mat4x4f::Translation(position) * transform, child.size);
          }
        }
      }
    };

    template <size_t D>
    class Stage {
      math::Region<float, D> region;
      std::shared_ptr<Node<D>> root;

      public:
      Stage(const math::Region<float, D>& region) : region(region) {}

      void SetRoot(std::shared_ptr<Node<D>> root) { this->root = root; }

      template <typename C, typename... U>
      std::shared_ptr<C> MakeRoot(U&&... args) {
        auto root = std::make_shared<C>(std::forward<U>(args)...);
        this->root = root;
        return root;
      }

      void Render(emp::graphics::Graphics& g) {
        if (root) {
          root->UpdateSize(g, region.extents());
          root->RenderRelative(g, math::Mat4x4f::Identity(), region.extents());
        }
      }
    };

    template <size_t D>
    class Text : public scenegraph::Node<D> {
      private:
      std::string text;
      float size;

      public:
      Text(const std::string& text, float size = 16) : text(text), size(size) {}

      virtual ~Text() {}

      emp::math::Vec<float, D> UpdateMinSize(
        const graphics::Graphics& g) override {
        return g.Measure(text, size);
      }

      void RenderRelative(graphics::Graphics& g, const math::Mat4x4f& transform,
                          const math::Vec<float, D>& allocated_size) override {
        g.Text()
          .Draw({
            emp::graphics::Text = text,
            emp::graphics::Fill = emp::opengl::Color::black(1),
            emp::graphics::Transform = transform,
            emp::graphics::TextSize = size,
          })
          .Flush();
      }
    };
  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_SCENEGRAPH_CORE_H
