/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/in_progress/AST.hpp
 * @brief Tools to build an Abstract Syntax Tree.
 * @note  Status: ALPHA
 */

#pragma once

#ifndef INCLUDE_EMP_IN_PROGRESS_AST_HPP_GUARD
#define INCLUDE_EMP_IN_PROGRESS_AST_HPP_GUARD

#include <string>

#include "../base/assert.hpp"
#include "../meta/TypeID.hpp"

namespace emp {

  template <typename SYMBOL_TABLE_T>
  struct AST {
    // Base class for all AST nodes.
    class Node {
    protected:
      emp::Ptr<Node> parent;
    public:
      Node(emp::Ptr<Node> in_parent = nullptr) : parent(in_parent) {}

      virtual ~Node();

      emp::Ptr<Node> GetParent() { return parent; }

      void SetParent(emp::Ptr<Node> in_parent) { parent = in_parent; }

      virtual std::string GetName() const = 0;
      virtual emp::TypeID GetType() const = 0;

      virtual bool IsLeaf() const { return false; }

      virtual bool IsInternal() const { return false; }

      virtual size_t GetNumChildren() const { return 0; }

      virtual emp::Ptr<Node> GetChild(size_t /* id */) {
        emp_assert(false, "Calling GetChild on AST::Node with no children.");
        return nullptr;
      }

      // virtual emp::Ptr<ConfigScope> GetScope() { return parent ? parent->GetScope() : nullptr; }
      // virtual entry_ptr_t Process() = 0;
      virtual std::function<double(SYMBOL_TABLE_T &)> AsMathFunction() = 0;

      virtual void Write(std::ostream & /* os */          = std::cout,
                         const std::string & /* offset */ = "") const {}
    }
  };
}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_IN_PROGRESS_AST_HPP_GUARD
