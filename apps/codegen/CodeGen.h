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

  int token_identifier = -1;
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

  struct AST_Block : AST_Node {
    // Children are a series of statements.
  };

  struct AST_Using : AST_Node {
    std::string type_name;
    std::string type_value;
  };

  struct AST_VarDeclare : AST_Node {
    std::string var_name;
    // Child is an AST_Code expression.
  };

  // Full concept information.
  struct AST_Concept : AST_Node {
    std::string name;
    std::string base_name;
    // Children a Using, Variable Declaration, or Function Declaration
  };

  AST_Node ast_root;


  // Helper functions
  bool HasToken(int pos) const { return (pos >= 0) && (pos < tokens.size()); }
  bool IsID(int pos) const { return HasToken(pos) && tokens[pos].token_id == token_identifier; }
  bool IsNumber(int pos) const { return HasToken(pos) && tokens[pos].token_id == token_number; }
  bool IsString(int pos) const { return HasToken(pos) && tokens[pos].token_id == token_string; }
  char AsChar(int pos) const {
    if (HasToken(pos) && tokens[pos].token_id == token_other) return tokens[pos].lexeme[0];
    return 0;
  }

  void Error(const std::string & msg, int pos = -1) {
    std::cout << "Error: " << msg << "\nAborting." << std::endl;
    exit(1);
  }

  void RequireID(int pos, const std::string & error_msg) {
    if (!IsID(pos)) { Error(error_msg, pos); }
  }
  void RequireNumber(int pos, const std::string & error_msg) {
    if (!IsNumber(pos)) { Error(error_msg, pos); }
  }
  void RequireString(int pos, const std::string & error_msg) {
    if (!IsString(pos)) { Error(error_msg, pos); }
  }
  void RequireChar(char req_char, int pos, const std::string & error_msg) {
    if (AsChar(pos) != req_char) { Error(error_msg, pos); }
  }
  void RequireLexeme(const std::string & req_str, int pos, const std::string & error_msg) {
    if (!HasToken(pos) || tokens[pos].lexeme != req_str) { Error(error_msg, pos); }
  }

public:
  CodeGen(std::string in_filename) : filename(in_filename) {
    // Whitespace and comments should always be dismissed (top priority)
    lexer.AddToken("Whitespace", "[ \t\n\r]+", false, false);                // Any form of whitespace.
    lexer.AddToken("Comment", "//.*", true, false);                          // Any '//'-style comment.

    // Meaningful tokens have next priority.
    token_identifier = lexer.AddToken("ID", "[a-zA-Z_][a-zA-Z0-9_]+", true, true);   // Identifiers
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
      RequireID(pos, "Statements in outer scope must begi with an identifier or keyword.");

      if (tokens[pos].lexeme == "concept") {
        auto node_ptr = emp::NewPtr<AST_Concept>();
        ast_root.AddChild(node_ptr);
        pos = ProcessConcept(pos + 1, *node_ptr);
      }
      // @CAO: Technically we can have a whole list of special keywords, but for now its just "concept".
      else {
        Error( emp::to_string("Unknown keyword '", tokens[pos].lexeme, "'.  Aborting."), pos )
      }
    }
    return pos;
  }

  // We know we are in a concept definition.  Collect appropriate information.
  size_t ProcessConcept(size_t pos, AST_Concept & concept) {
    // A concept must begin with its name.
    RequireID(pos, "Concept declaration must be followed by name identifier.");
    concept.name = tokens[pos++].lexeme;

    // Next, must be a colon...
    RequireChar(':', pos, "Concept names must be followed by a colon (':').");
    pos++;

    // And then a base-class name.
    RequireID(pos, "Concept declaration must include name of base class.");
    concept.base_name = tokens[pos++].lexeme;

    // Next, must be an open brace...
    RequireChar('{', pos, "Concepts must be defined in braces ('{' and '}').");
    pos++;

    // Loop through the full definition of concept, incorporating each entry.
    while ( AsChar(pos) != '{' ) {
      // Entries can be a "using" statement, a function definition, or a variable definition.
      RequireID(pos, "Concept members can be either functions, variables, or using-statements.")
    }

    return pos;
  }
  
  void PrintLexerState() { lexer.Print(); }

  void PrintTokens() {
    for (auto token : tokens) {
      std::cout << lexer.GetTokenName(token) << " : \"" << token.lexeme << "\"" << std::endl;
    }
  }
};