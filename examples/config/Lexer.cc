#include <iostream>
#include <sstream>

#include "../../config/ConfigLexer.h"

int main()
{
  emp::Token token(emp::Token::STRING_LIT);
  std::cout << token.TypeName() << std::endl;

  std::stringstream ss;
  ss << "This is_a 23 Test.  return 123.4 as 'a' float + continue!";
  emp::ConfigLexer lexer(ss);

  while ((token = lexer.GetToken())) {
    std::cout << token.ToString() << std::endl;
  }
}
