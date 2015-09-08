#include <iostream>
#include <string>
#include <array>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/tuple_struct.h"

struct TestStruct {
  int myvar;

  EMP_BUILD_TUPLE( int, MyInt,
                   int, MyInt2,
                   int, MyInt3,
                   int, MyInt4,
                   int, MyInt5,
                   int, MyInt6,
                   int, MyInt7,
                   int, MyInt8,
                   int, MyInt9,
                   int, MyInt10,
                   std::string, MyString, 
                   std::string, MyString2, 
                   std::string, MyString3, 
                   std::string, MyString4, 
                   std::string, MyString5, 
                   std::string, MyString6, 
                   std::string, MyString7, 
                   std::string, MyString8, 
                   std::string, MyString9, 
                   std::string, MyString10, 
                   char, MyChar,
                   char, MyChar2,
                   char, MyChar3,
                   char, MyChar4,
                   char, MyChar5,
                   char, MyChar6,
                   char, MyChar7,
                   char, MyChar8,
                   char, MyChar9,
                   char, MyChar10)
};

struct TestStoredVarStruct {
  EMP_BUILD_TUPLE_STORE_VAR_NAMES( int, MyInt,
                   int, MyInt2,
                   int, MyInt3,
                   int, MyInt4,
                   int, MyInt5,
                   int, MyInt6,
                   int, MyInt7,
                   int, MyInt8,
                   int, MyInt9,
                   int, MyInt10,
                   std::string, MyString, 
                   std::string, MyString2, 
                   std::string, MyString3, 
                   std::string, MyString4, 
                   std::string, MyString5, 
                   std::string, MyString6, 
                   std::string, MyString7, 
                   std::string, MyString8, 
                   std::string, MyString9, 
                   std::string, MyString10, 
                   char, MyChar,
                   char, MyChar2,
                   char, MyChar3,
                   char, MyChar4,
                   char, MyChar5,
                   char, MyChar6,
                   char, MyChar7,
                   char, MyChar8,
                   char, MyChar9,
                   char, MyChar10)
};

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  std::cout << "Hello!" << std::endl;

  TestStruct test;
  test.myvar = 12;
  test.MyInt(15);
  test.MyInt() = 20;

  std::cout << "myvar = " << test.myvar << std::endl;
  std::cout << "MyInt = " << test.MyInt() << std::endl;

  TestStoredVarStruct test2;
  test2.MyInt(8);
  std::cout << "MyInt = " << test2.MyInt() << std::endl;
  std::cout << "First variable name: " << test2.var_names[0] << std::endl;
  
}
