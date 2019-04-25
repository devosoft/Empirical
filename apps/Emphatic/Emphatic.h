/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Emphatic.h
 *  @brief A system to generate dynamic concept code for C++17.
 *  @note Status: ALPHA
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
 *             | '&'
 *             | '*'
 *   - DECLARE: TYPE ID
 *   - FUNCTION: DECLARE '(' PARAMS ')' BLOCK
 *             | DECLARE '(' PARAMS ')' '=' "required" ';'
 *             | DECLARE '(' PARAMS ')' '=' "default" ';'
 *   - PARAMS: (nothing)
 *           | PARAM_LIST
 *   - PARAM_LIST: PARAM
 *               | PARAM ',' PARAM_LIST
 *   - PARAM: DECLARE
 *          | OVERLOAD '(' ID ')'
 *   - MEMBER: DECLARE ';'
 *           | FUNCTION
 *           | "using" ID '=' TYPE ';'
 *           | "using" ID '=' "required" ';'
 */

#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include "AST.h"
#include "EmphaticLexer.h"

class Emphatic {
private:
  std::string filename;             ///< Source for for code to generate.
  EmphaticLexer lexer;              ///< Lexer to process input code.
  emp::vector<emp::Token> tokens;   ///< Tokenized version of input file.
  bool debug = false;               ///< Should we print full debug information?

  AST_Scope ast_root;


  // Helper functions
  bool HasToken(int pos) const { return (pos >= 0) && (pos < tokens.size()); }
  bool IsID(int pos) const { return HasToken(pos) && lexer.IsID(tokens[pos]); }
  bool IsNumber(int pos) const { return HasToken(pos) && lexer.IsNumber(tokens[pos]); }
  bool IsString(int pos) const { return HasToken(pos) && lexer.IsString(tokens[pos]); }
  char AsChar(int pos) const {
    return (HasToken(pos) && lexer.IsSymbol(tokens[pos])) ? tokens[pos].lexeme[0] : 0;
  }
  const std::string & AsLexeme(int pos) const {
    return HasToken(pos) ? tokens[pos].lexeme : emp::empty_string();
  }

  std::string ConcatLexemes(size_t start_pos, size_t end_pos) {
    emp_assert(start_pos <= end_pos);
    emp_assert(end_pos <= tokens.size());
    std::stringstream ss;    
    for (size_t i = start_pos; i < end_pos; i++) {
      if (i > start_pos) ss << " ";
      ss << tokens[i].lexeme;
    }
    return ss.str();
  }

  void Error(int pos, const std::string & msg) {
    std::cout << "Error (token " << pos << "): " << msg << "\nAborting." << std::endl;
    exit(1);
  }

  template <typename... Ts>
  void Debug(Ts... args) {
    if (debug) std::cout << "DEBUG: " << emp::to_string(args...) << std::endl;
  }

  void RequireID(int pos, const std::string & error_msg) {
    if (!IsID(pos)) { Error(pos, error_msg); }
  }
  void RequireNumber(int pos, const std::string & error_msg) {
    if (!IsNumber(pos)) { Error(pos, error_msg); }
  }
  void RequireString(int pos, const std::string & error_msg) {
    if (!IsString(pos)) { Error(pos, error_msg); }
  }
  void RequireChar(char req_char, int pos, const std::string & error_msg) {
    if (AsChar(pos) != req_char) { Error(pos, error_msg); }
  }
  void RequireLexeme(const std::string & req_str, int pos, const std::string & error_msg) {
    if (AsLexeme(pos) != req_str) { Error(pos, error_msg); }
  }

public:
  Emphatic(std::string in_filename) : filename(in_filename) {
    std::ifstream file(filename);
    tokens = lexer.Tokenize(file);
    file.close();
  }

  /// Print out the original state of the code.
  void PrintEcho(std::ostream & os) const {
    ast_root.PrintEcho(os, "");
  }

  /// Print out the original state of the code.
  void PrintOutput(std::ostream & os) const {
    ast_root.PrintOutput(os, "");
  }

  /// Collect a line of code, ending with a semi-colon OR mis-matched bracket.
  /// Always stops at a mis-matched ')' '}' or ']'
  /// If match_angle_bracket is set, will also stop at a mis-matched '>'
  /// If multi_line is set, will NOT stop with a ';'
  size_t ProcessCode(size_t pos, std::string & line, bool match_angle_bracket=false, bool multi_line=false) {
    const size_t start_pos = pos;
    std::vector<char> open_symbols;
    bool finished = false;
    while (!finished && pos < tokens.size()) {
      char cur_char = AsChar(pos++);
      switch (cur_char) {
        case ';':
          if (multi_line == false) finished = true;
          break;
        case '<':
          if (match_angle_bracket == false) break;
          [[fallthrough]]
        case '(':
        case '[':
        case '{':
          open_symbols.push_back(cur_char);  // Store this open bracket to be matched later.
          break;
        case '>':
          if (match_angle_bracket == false) break;
          [[fallthrough]]
        case ')':
        case ']':
        case '}':
          if (open_symbols.size()) {
            // @CAO should check to make sure this is a CORRECT match...
            open_symbols.pop_back();
            break;
          }
          // We will only make it here is this is an unmatched bracket.
          pos--;              // Leave close bracket to still be processed.
          finished = true;
          break;
      }
    }

    line = ConcatLexemes(start_pos, pos);

    return pos;
  }

  /// Collect all tokens used to describe a type.
  size_t ProcessType(size_t pos, std::string & type_name) {
    const size_t start_pos = pos;
    // A type may start with a const.
    if (AsLexeme(pos) == "const") pos++;

    // Figure out the identifier (with possible "::" requiring another id)
    bool need_id = true;
    while (need_id) {
      if (AsLexeme(pos) == "typename") pos++;  // May specify a typename is next.
      if (AsLexeme(pos) == "template") pos++;  // May specify a template is next.

      RequireID(pos, emp::to_string("Expecting type, but found '", tokens[pos].lexeme, "'."));
      pos++;
      need_id = false;

      // In case this is a template, we need to evaluate parameters.
      if (AsLexeme(pos) == "<") {
        pos = ProcessCode(pos+1, type_name, true);
        RequireChar('>', pos++, "Templates must end in a close angle bracket.");
      }

      if (AsLexeme(pos) == "::") {
        pos++;
        need_id = true;
      }
    }

    // Type may end in a symbol...
    if (AsLexeme(pos) == "&") pos++;
    if (AsLexeme(pos) == "*") pos++;

    // Collect all of the lexemes
    type_name = ConcatLexemes(start_pos, pos);

    return pos;
  }

  /// Collect all of the parameter definitions for a function.
  size_t ProcessParams(size_t pos, emp::vector<ConceptFunction::Param> & params) {
    while (AsChar(pos) != ')') {
      // If this isn't the first parameter, make sure we have a comma to separate them.
      if (params.size()) {
        RequireChar(',', pos++, "Parameters must be separated by commas.");
      }

      // Start with a type...
      std::string type_name;
      pos = ProcessType(pos, type_name);

      // If an identifier is specified for this parameter, grab it.
      std::string identifier = IsID(pos) ? tokens[pos++].lexeme : "";

      ConceptFunction::Param new_param{type_name, identifier};
      params.emplace_back(new_param);
    }

    return pos;    
  }

  /// Collect a series of identifiers, separated by spaces.
  size_t ProcessIDList(size_t pos, std::set<std::string> & ids) {
    while (IsID(pos)) {
      ids.insert(AsLexeme(pos));
      pos++;
    }
    return pos;
  }

 
  /// Process the tokens starting from the outer-most scope.
  size_t ProcessTop(size_t pos, AST_Scope & cur_scope ) {
    while (pos < tokens.size() && AsChar(pos) != '}') {
      RequireID(pos, emp::to_string("Statements in outer scope must begin with an identifier or keyword.  (Found: ",
                     AsLexeme(pos), ")."));

      std::string cur_lexeme = AsLexeme(pos++);
      if (cur_lexeme == "concept") {
        AST_Concept & new_node = cur_scope.NewChild<AST_Concept>();
        pos = ProcessConcept(pos, new_node);
      }
      else if (cur_lexeme == "struct" || cur_lexeme == "class") {
        AST_Class & new_class = cur_scope.NewChild<AST_Class>();
        new_class.type = cur_lexeme;
        if (IsID(pos)) new_class.name = AsLexeme(pos++);
        RequireChar('{', pos++, emp::to_string("A ", cur_lexeme, " must be defined in braces ('{' and '}')."));
        pos = ProcessCode(pos, new_class.body, false, true);
        RequireChar('}', pos++, emp::to_string("The end of a ", cur_lexeme, " must have a close brace ('}')."));
        RequireChar(';', pos++, emp::to_string("A ", cur_lexeme, " must end with a semi-colon (';')."));
      }
      else if (cur_lexeme == "namespace") {
        auto & new_ns = cur_scope.NewChild<AST_Namespace>();

        // If a name is provided for this namespace, store it.
        if (IsID(pos)) new_ns.name = AsLexeme(pos++);

        RequireChar('{', pos++, emp::to_string("A ", cur_lexeme, " must be defined in braces ('{' and '}')."));
        pos = ProcessTop(pos, new_ns);
        RequireChar('}', pos++, emp::to_string("The end of a ", cur_lexeme, " must have a close brace ('}')."));
      }
      else if (cur_lexeme == "using") {
        RequireID(pos, "A 'using' command must first specify the new type name.");
        auto & new_using = cur_scope.NewChild<AST_Using>();
        pos = ProcessType(pos, new_using.type_name);      // Determine new type name being defined.
        RequireChar('=', pos++, "A using statement must provide an equals ('=') to assign the type.");
        pos = ProcessCode(pos, new_using.type_value);   // Determine code being assigned to.
      }
      // @CAO: Still need to deal with "template", variables and functions.
      else {
        Error( pos-1, emp::to_string("Unknown keyword '", cur_lexeme, "'.  Aborting.") );
      }
    }
    return pos;
  }

  /// We know we are in a concept definition.  Collect appropriate information.
  size_t ProcessConcept(size_t pos, AST_Concept & concept) {
    // A concept must begin with its name.
    RequireID(pos, "Concept declaration must be followed by name identifier.");
    concept.name = tokens[pos++].lexeme;

    // Next, must be a colon...
    RequireChar(':', pos++, "Concept names must be followed by a colon (':').");

    // And then a base-class name.
    RequireID(pos, "Concept declaration must include name of base class.");
    concept.base_name = tokens[pos++].lexeme;

    Debug("Defining concept '", concept.name, "' with base class '", concept.base_name, "'.");

    // Next, must be an open brace...
    RequireChar('{', pos++, "Concepts must be defined in braces ('{' and '}').");

    // Loop through the full definition of concept, incorporating each entry.
    while ( AsChar(pos) != '}' ) {
      // Entries can be a "using" statement, a function definition, or a variable definition.
      RequireID(pos, "Concept members can be either functions, variables, or using-statements.");

      if (tokens[pos].lexeme == "using") {              // ----- USING!! -----
        pos++;  // Move past "using"
        RequireID(pos, "A 'using' command must first specify the new type name.");

        ConceptTypedef new_typedef;
        pos = ProcessType(pos, new_typedef.type_name);      // Determine new type name being defined.

        Debug("...adding a type '", new_typedef.type_name, "'.");

        RequireChar('=', pos++, "A using statement must provide an equals ('=') to assign the type.");

        pos = ProcessCode(pos, new_typedef.type_value);   // Determine code being assigned to.

        Debug("   value: ", new_typedef.type_value);

        concept.typedefs.push_back(new_typedef);
      }
      else {
        // Start with a type...
        std::string type_name;
        pos = ProcessType(pos, type_name);

        // Then an identifier.
        RequireID(pos, "Functions and variables in concept definition must provide identifier after type name.");
        std::string identifier = tokens[pos++].lexeme;

        // If and open-paren follows the identifier, we are defining a function, otherwise it's a variable.
        if (AsChar(pos) == '(') {                              // ----- FUNCTION!! -----
          pos++;  // Move past paren.

          // Setup an AST Node for a function definition.
          ConceptFunction new_function;
          new_function.return_type = type_name;
          new_function.fun_name = identifier;

          pos = ProcessParams(pos, new_function.params);       // Read the parameters for this function.

          RequireChar(')', pos++, "Function arguments must end with a close-parenthesis (')')");

          Debug("...adding a function '", type_name, " ", identifier, "(", new_function.ParamString(), ")'");

          pos = ProcessIDList(pos, new_function.attributes);   // Read in each of the function attributes, if any.

          Debug("   with attributes: ", new_function.AttributeString());

          char fun_char = AsChar(pos++);

          if (fun_char == '=') {  // Function is "= default;" or "= required;"
            RequireID(pos, "Function must be assigned to 'required' or 'default'");
            std::string fun_assign = AsLexeme(pos++);
            if (fun_assign == "required") new_function.is_required = true;
            else if (fun_assign == "default") new_function.is_default = true;
            else Error(pos, "Functions can only be set to 'required' or 'default'");
            RequireChar(';', pos++, emp::to_string(fun_assign, "functions must end in a semi-colon."));
          }
          else if (fun_char == '{') {  // Function is defined in place.
            pos = ProcessCode(pos, new_function.default_code, false, true);  // Read the default function body.

            Debug("   and code: ", new_function.default_code);

            RequireChar('}', pos++, emp::to_string("Function body must end with close brace ('}') not '",
                                                  AsLexeme(pos-1), "'."));
          }
          else {
            Error(pos-1, "Function body must begin with open brace or assignment ('{' or '=')");
          }

          concept.functions.push_back(new_function);

        } else {                                                 // ----- VARIABLE!! -----
          ConceptVariable new_var;
          new_var.var_type = type_name;
          new_var.var_name = identifier;

          if (AsChar(pos) == ';') {  // Does the variable declaration end here?
            pos++;
          }
          else {                     // ...or is there a default value for this variable?
            // Determine code being assigned from.
            pos = ProcessCode(pos, new_var.default_code);
          }

          concept.variables.push_back(new_var);

        }
      }
    }

    pos++;  // Skip closing brace.
    RequireChar(';', pos++, "Concept definitions must end in a semi-colon.");

    return pos;
  }
  
  void Process() {
    ProcessTop(0, ast_root);
  }

  /// Print the state of the lexer used for code generation.
  void PrintLexerState() { lexer.Print(); }

  /// Print the set of tokens loaded in from the input file.
  void PrintTokens() {
    for (size_t pos = 0; pos < tokens.size(); pos++) {
      std::cout << pos << ": "
                << lexer.GetTokenName(tokens[pos])
                << " : \"" << AsLexeme(pos) << "\""
                << std::endl;
    }
  }

  /// Setup debug mode (with verbose printing).
  void SetDebug(bool in_debug=true) { debug = in_debug; }

};