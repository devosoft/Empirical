#include <iostream>
#include <sstream>

#include "../../config/ConfigParser.h"

int main()
{
  std::stringstream ss;
  ss << "temp_val = 7; temp_val2 = 5 + temp_val * 2; temp_val2 = \"abc\";";
  emp::ConfigParser parser(ss);

}
