/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University; MIT license; see doc/LICENSE.md
 *
 * @brief A small application using an Empala-generated parser.
 *
 * From demos/ParserGenerator: make dice && ./dice 3d6 - 2d4 + 10
 * Read dice.empala first, then the handler below, then main().
 */
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "emp/math/Random.hpp"
#include "emp/tools/String.hpp"

// Created by Empala at build time. The Makefile supplies -Ibuild; angle brackets
// ensure an older header beside this source cannot shadow the freshly generated one.
#include <dice_parser.hpp>

using Parser = dice_example::Parser;
using Node = Parser::ASTNode;
using Value = Parser::value_t;  // long long, selected by :value_type in dice.empala.

// The grammar names methods, not free functions. Empala calls these methods on
// the object passed to root->Run(handler). This lets actions share application
// state, such as a random generator, without global variables.
struct DiceHandler {
  emp::Random & random;

  // @Number: the sole child is an INTEGER token. Its lexeme is the original text.
  Value Number(Node & node) { return std::stoll(node.GetChildLexeme(0)); }

  // @Roll: children are INTEGER, 'd', INTEGER (the literal is also a child).
  // INTEGER?:1 supplies a synthetic count for d6, so these indices never change.
  Value Roll(Node & node) {
    const Value count = std::stoll(node.GetChildLexeme(0));
    const Value sides = std::stoll(node.GetChildLexeme(2));

    // Syntax alone cannot enforce these domain rules. Keep this teaching example
    // quick to run and its dice subtotal comfortably within the Value range.
    if (count > 100'000) throw std::runtime_error("Use at most 100,000 dice per roll.");
    if (sides < 1 || sides > 1'000'000) {
      throw std::runtime_error("Each die must have between 1 and 1,000,000 sides.");
    }

    Value total = 0;
    for (Value i = 0; i < count; ++i) {
      // GetUInt(sides) returns [0, sides), so add 1 to roll in [1, sides].
      total += static_cast<Value>(random.GetUInt(sides)) + 1;
    }
    return total;  // Zero dice give zero; one-sided dice always give one.
  }

  // @Sum: anonymous (...) groups are flattened into the parent AST node.
  // RunChild evaluates a term using this same handler, rolling dice if needed.
  Value Sum(Node & node) {
    Value total = node.RunChild(0, *this);
    for (size_t i = 1; i < node.NumChildren(); i += 2) {
      const bool subtract = node.GetChildLexeme(i) == "-";
      const Value term = node.RunChild(i + 1, *this);

      // Terms are nonnegative, but subtraction can make the running total negative.
      // Report overflow instead of invoking undefined signed-integer arithmetic.
      if ((!subtract && total > std::numeric_limits<Value>::max() - term)
          || (subtract && total < std::numeric_limits<Value>::min() + term)) {
        throw std::overflow_error("The expression result is outside the integer range.");
      }
      total = subtract ? total - term : total + term;
    }
    return total;
  }
};

int main(int argc, char * argv[]) {
  // if (argc < 2 || (argc == 2 && (std::string(argv[1]) == "--help"
  //                              || std::string(argv[1]) == "-h"))) {
  //   std::cout << "Usage: dice EXPRESSION\n"
  //             << "Examples: dice 3d6 - 2d4 + 10\n"
  //             << "          dice \"20 - 5 - 3\"\n"
  //             << "Use XdY (or dY for one die), whole numbers, +, and -. Operators apply left to right.\n"
  //             << "Dice counts: 0..100000; die sizes: 1..1000000.\n";
  //   return argc < 2 ? 1 : 0;
  // }

  // Join shell arguments: a quoted expression and separate arguments both work.
  emp::String expression;
  for (int i = 1; i < argc; ++i) {
    if (i > 1) expression += ' ';
    expression += argv[i];
  }

  // 1. Tokenize using the generated lexer. It owns the token vector; keep it alive.
  dice_example::Lexer lexer;
  const auto & tokens = lexer.Tokenize(expression);

  // 2. Parse the COMPLETE input before evaluating anything. Bad syntax rolls no dice.
  Parser parser;
  auto root = parser.Parse(tokens);
  if (!root) {
    parser.PrintError(std::cerr);
    return 1;
  }

  // 3. Supply application state and evaluate the AST. Empala dispatches to Sum,
  // Roll, and Number above. Use emp::Random random(42) for a repeatable sequence.
  emp::Random random;
  DiceHandler handler{random};
  try {
    const Value result = root->Run(handler);
    std::cout << result << '\n';
  } catch (const std::out_of_range &) {
    std::cerr << "dice: An integer is too large (maximum "
              << std::numeric_limits<Value>::max() << ").\n";
    return 1;
  } catch (const std::exception & error) {
    std::cerr << "dice: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
