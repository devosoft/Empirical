/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/compiler/_ASTNode.hpp
 * @brief A node in an abstract syntax tree produced by a Parser (helper for Parser.hpp)
 * @note Status: ALPHA
 *
 * An ASTNode is either a LEAF (wrapping a single Token from the lexer) or an internal
 * RULE node (identifying which parse rule -- and which alternative pattern of that rule --
 * was matched, along with an ordered set of child nodes).
 *
 * Each rule node may be associated with an ACTION: a function to call when the node is
 * Run().  Actions are stored in the Parser that produced the AST; nodes simply point to
 * them.  As such, an AST should not be Run() after its Parser has been destroyed or had
 * its grammar modified.
 */

#pragma once

#ifndef INCLUDE_EMP_COMPILER_AST_NODE_HPP_impl_GUARD
#define INCLUDE_EMP_COMPILER_AST_NODE_HPP_impl_GUARD

#include <functional>
#include <type_traits>
#include <iostream>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../tools/String.hpp"

#include "_Token.hpp"

namespace emp {

  /// A single node in an abstract syntax tree; either a leaf (token) or a rule node.
  /// VALUE_T is the type produced when a node is Run().
  template <typename VALUE_T>
  class ASTNode {
  public:
    using value_t  = VALUE_T;
    using action_t = std::function<VALUE_T(ASTNode &)>;

    static constexpr size_t npos = static_cast<size_t>(-1);

  private:
    size_t rule_id    = npos;  ///< Which parse rule produced this node? (npos = leaf)
    size_t pattern_id = 0;     ///< Which alternative pattern of the rule was matched?
    Token token{-1};           ///< For leaves: the token matched.
    String note;               ///< Optional annotation attached to the matched alternative.

    emp::vector<emp::Ptr<ASTNode>> children;  ///< Sub-nodes, in matched order (owned).

    /// Action to call on Run(); points into the Parser's rule set (may be null).
    emp::Ptr<const action_t> action = nullptr;

  public:
    ASTNode() = default;

    /// Build a LEAF node from a token.
    explicit ASTNode(const Token & in_token) : token(in_token) {}

    /// Build an internal RULE node.
    ASTNode(size_t _rule_id, size_t _pattern_id) : rule_id(_rule_id), pattern_id(_pattern_id) {}

    // AST nodes own their children, so they can be moved but not copied.
    ASTNode(const ASTNode &) = delete;
    ASTNode(ASTNode && in)
      : rule_id(in.rule_id)
      , pattern_id(in.pattern_id)
      , token(std::move(in.token))
      , note(std::move(in.note))
      , children(std::move(in.children))
      , action(in.action) {
      in.children.resize(0);
    }

    ~ASTNode() {
      for (auto child : children) { child.Delete(); }
    }

    ASTNode & operator=(const ASTNode &) = delete;
    ASTNode & operator=(ASTNode && in) {
      if (this == &in) return *this;
      for (auto child : children) { child.Delete(); }
      rule_id    = in.rule_id;
      pattern_id = in.pattern_id;
      token      = std::move(in.token);
      note       = std::move(in.note);
      children   = std::move(in.children);
      action     = in.action;
      in.children.resize(0);
      return *this;
    }

    [[nodiscard]] bool IsLeaf() const { return rule_id == npos; }
    [[nodiscard]] bool IsRule() const { return rule_id != npos; }

    [[nodiscard]] size_t GetRuleID() const { return rule_id; }
    [[nodiscard]] size_t GetPatternID() const { return pattern_id; }

    /// Get the token matched by a leaf node.
    [[nodiscard]] const Token & GetToken() const {
      emp_assert(IsLeaf(), "Only leaf AST nodes have tokens.", rule_id);
      return token;
    }

    /// Get the annotation attached to the matched alternative.
    [[nodiscard]] const String & GetNote() const { return note; }
    void SetNote(const String & in_note) { note = in_note; }

    /// Get the lexeme matched by a leaf node.
    [[nodiscard]] const String & GetLexeme() const { return GetToken().lexeme; }

    [[nodiscard]] size_t NumChildren() const { return children.size(); }

    [[nodiscard]] ASTNode & GetChild(size_t id) {
      emp_assert(id < children.size(), id, children.size());
      return *children[id];
    }
    [[nodiscard]] const ASTNode & GetChild(size_t id) const {
      emp_assert(id < children.size(), id, children.size());
      return *children[id];
    }

    /// Shortcut: get the lexeme matched by child leaf node `id`.
    [[nodiscard]] const String & GetChildLexeme(size_t id) const {
      return GetChild(id).GetLexeme();
    }

    /// Take ownership of a new child node, adding it at the end of the child list.
    void AdoptChild(emp::Ptr<ASTNode> child) {
      emp_assert(!child.IsNull());
      children.push_back(child);
    }

    /// Release ownership of the only child when promoting a transparent root.
    emp::Ptr<ASTNode> ReleaseOnlyChild() {
      emp_assert(children.size() == 1);
      auto child = children.front();
      children.clear();
      return child;
    }

    /// Move all of the children out of another node onto the end of this node's child list.
    /// (Used to splice anonymous group nodes into their parent.)
    void SpliceChildren(ASTNode & in) {
      for (auto child : in.children) { children.push_back(child); }
      in.children.resize(0);  // Ownership has been transferred.
    }

    /// Remove (and delete) all children beyond the first `new_size`; used for backtracking.
    void CropChildren(size_t new_size) {
      emp_assert(new_size <= children.size(), new_size, children.size());
      for (size_t i = new_size; i < children.size(); ++i) { children[i].Delete(); }
      children.resize(new_size);
    }

    void SetAction(emp::Ptr<const action_t> in_action) { action = in_action; }
    [[nodiscard]] bool HasAction() const { return !action.IsNull() && *action; }

    /// Run this node.  If an action is associated with the matched pattern, call it (the
    /// action may call RunChild() to evaluate sub-nodes -- lazily and in any order).
    /// Otherwise, by default, Run() each rule-node child in order and return the last result.
    VALUE_T Run() {
      if (HasAction()) { return (*action)(*this); }
      if constexpr (std::is_void_v<VALUE_T>) {
        for (auto child : children) {
          if (child->IsRule()) { child->Run(); }
        }
      } else {
        VALUE_T out{};
        for (auto child : children) {
          if (child->IsRule()) { out = child->Run(); }
        }
        return out;
      }
    }

    /// Run the specified child node and return its result.
    VALUE_T RunChild(size_t id) { return GetChild(id).Run(); }

    /// Print out this AST (for debugging).  A name_fun may be provided to convert rule IDs
    /// and leaf token IDs into human-readable names (see Parser::PrintAST).
    void Print(std::ostream & os = std::cout,
               const std::function<String(const ASTNode &)> & name_fun = nullptr,
               size_t indent = 0) const {
      for (size_t i = 0; i < indent; ++i) { os << "  "; }
      if (IsLeaf()) {
        if (name_fun) { os << name_fun(*this); }
        else { os << "token " << token.id; }
        os << " \"" << MakeEscaped(token.lexeme) << "\"\n";
      } else {
        if (name_fun) { os << name_fun(*this); }
        else { os << "rule " << rule_id; }
        os << " [pattern " << pattern_id << "]";
        if (!note.empty()) os << " note=" << note.AsLiteral();
        os << '\n';
        for (auto child : children) { child->Print(os, name_fun, indent + 1); }
      }
    }
  };

  /// An owning wrapper around the root of an abstract syntax tree.
  /// Movable but not copyable; deletes the full tree when destroyed.
  template <typename VALUE_T>
  class AST {
  public:
    using node_t = ASTNode<VALUE_T>;

  private:
    emp::Ptr<node_t> root = nullptr;
    bool succeeded = false;

  public:
    AST() = default;
    explicit AST(emp::Ptr<node_t> in_root, bool success = true) : root(in_root), succeeded(success) {}

    AST(const AST &) = delete;
    AST(AST && in) : root(in.root), succeeded(in.succeeded) { in.root = nullptr; in.succeeded = false; }

    ~AST() { if (!root.IsNull()) { root.Delete(); } }

    AST & operator=(const AST &) = delete;
    AST & operator=(AST && in) {
      if (this == &in) return *this;
      if (!root.IsNull()) { root.Delete(); }
      root    = in.root;
      succeeded = in.succeeded;
      in.root = nullptr;
      in.succeeded = false;
      return *this;
    }

    /// Did the parse that produced this AST succeed?
    [[nodiscard]] bool IsValid() const { return succeeded; }
    [[nodiscard]] bool HasRoot() const { return !root.IsNull(); }
    explicit operator bool() const { return IsValid(); }

    [[nodiscard]] node_t & GetRoot() {
      emp_assert(HasRoot(), "This parse result has no AST root.");
      return *root;
    }
    [[nodiscard]] const node_t & GetRoot() const {
      emp_assert(HasRoot(), "This parse result has no AST root.");
      return *root;
    }

    /// Run the tree from the root, returning the final value.
    VALUE_T Run() {
      emp_assert(HasRoot(), "Cannot Run() a parse result without an AST root.");
      return root->Run();
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_COMPILER_AST_NODE_HPP_impl_GUARD
