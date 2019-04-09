/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  CodeGen.h
 *  @brief A system to generate dynamic concept code for C++17.
 *  @note Status: PLANNING
 *
 *  NOTES:
 *   - TYPE_OR_EXPRESSION: Collect everything until you hit an unmatched close-mark: ')', ']', '}', or '>'
 *   - STATEMENT: Collect everything until you hit a ';' outside of parens.
 *   - STATEMENT_LIST: (nothing)                  (an unmatched close-mark next requires this option)
 *                   | STATEMENT STATEMENT_LIST
 *   - BLOCK: '{' STATEMENT_LIST '}'
 * 
 *   - TYPE: ID TYPE_END
 *   - TYPE_END: (nothing)
 *             | "::" TYPE
 *             | "<" TYPE_OR_EXPRESSION ">" TYPE_END
 *   - DECLARE: TYPE ID
 *   - FUNCTION: DECLARE '(' PARAMS ')'
 *   - PARAMS: (nothing)
 *           | PARAM_LIST
 *   - PARAM_LIST: PARAM
 *               | PARAM ',' PARAM_LIST
 *   - PARAM: DECLARE
 *          | OVERLOAD '(' ID ')'
 *   - MEMBER: DECLARE ';'
 *           | FUNCTION BLOCK
 *           | "using" ID '=' TYPE ';'
 *           | "using" ID '=' "REQUIRE" '(' STRING ')' ';'
 */

#include <fstream>
#include <iostream>
#include <string>

#include "../../source/base/Ptr.h"
#include "../../source/tools/Lexer.h"

class CodeGen {
private:
  std::string filename;
  emp::Lexer lexer;
  emp::vector<emp::Token> tokens;

  int token_id = -1;
  int token_number = -1;
  int token_string = -1;
  int token_other = -1;

  // All AST Nodes have a common base class.
  struct AST_Node {
    emp::vector<emp::Ptr<AST_Node>> children;
    ~AST_Node() { for (auto x : children) x.Delete(); }
    void AddChild(emp::Ptr<AST_Node> node_ptr) { children.push_back(node_ptr); }
  };

  // Misc. Code that should just be echoed back out.
  struct AST_Code : AST_Node {
    std::string code;
  };

  // Full concept information.
  struct AST_Concept : AST_Node {
    
  };

  AST_Node ast_root;

public:
  CodeGen(std::string in_filename) : filename(in_filename) {
    // Whitespace and comments should always be dismissed (top priority)
    lexer.AddToken("Whitespace", "[ \t\n\r]+", false, false);                // Any form of whitespace.
    lexer.AddToken("Comment", "//.*", true, false);                          // Any '//'-style comment.

    // Meaningful tokens have next priority.
    token_id = lexer.AddToken("ID", "[a-zA-Z_][a-zA-Z0-9_]+", true, true);   // Identifiers
    token_number = lexer.AddToken("Number", "[0-9]+(.[0-9]+)?", true, true); // Literal numbers.
    token_string = lexer.AddToken("String", "\\\"[^\"]*\\\"", true, true);   // Literal strings.

    // Other tokens should have least priority.
    token_other = lexer.AddToken("Other", ".", true, true);                  // Symbols


    std::ifstream file(filename);
    tokens = lexer.Tokenize(file);
    file.close();
  }

  // Process the tokens starting from the outer-most scope.
  size_t ProcessTop(size_t pos=0) {
    while (pos < tokens.size()) {
      if (tokens[pos] != token_id) {
        std::cerr << "Statements in outer scope must being with an identifier or keyword.  Aborting."
                  << std::endl;
        exit(1);
      }

      if (tokens[pos].lexeme == "concept") {
        auto node_ptr = NewPtr<AST_Concept>();
        ast_root.AddChild(node_ptr);
        pos = ProcessConcept(pos + 1, *node_ptr);
      }
      else {
        std::cerr << "Unknown keyword '" << tokens[pos].lexeme << "'.  Aborting." << std::endl;
        exit(1);
      }
    }
    return pos;
  }

  // We know we are in a concept definition.  Collect appropriate information.
  size_t ProcessConcept(size_t pos, AST_Concept & concept) {
    return pos;
  }
  
  void PrintLexerState() { lexer.Print(); }

  void PrintTokens() {
    for (auto token : tokens) {
      std::cout << lexer.GetTokenName(token) << " : \"" << token.lexeme << "\"" << std::endl;
    }
  }
};