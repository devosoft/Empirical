/*
 * Minimal replication of an issue in opening file w/ Emscripten
 */

#include <fstream>
#include <iostream>

int main() {
  std::ofstream output_location; //Where does output go?
  output_location.open("temp/temp.txt");
  if (output_location.good())
  {
    std::cout << "File good!" << std::endl;
  }
  else
  {
    std::cout << "File bad!" << std::endl;
  }
}
