// This is the main function for the NATIVE version of this project.

/*
 * WARNING: the Cookiecutter Empirical Project, which you can find at
 * https://github.com/devosoft/cookiecutter-empirical-project, should be
 * preferred over ProjectTemplate
*/

#include <iostream>

#include "emp/base/vector.hpp"
#include "emp/config/command_line.hpp"

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  std::cout << "Hello World!" << '\n';

  std::cout << "WARNING: the Cookiecutter Empirical Project, which you can find at https://github.com/devosoft/cookiecutter-empirical-project, should be preferred over ProjectTemplate" << '\n';

}
