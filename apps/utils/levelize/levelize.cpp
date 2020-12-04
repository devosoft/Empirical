#include <fstream>
#include <iostream>
#include <string>

#include "../../../include/emp/base/vector.hpp"
#include "../../../include/emp/tools/string_utils.hpp"

int main(int argc, char * argv[])
{
  // Load in all of the files that we are working with.
  size_t num_files = argc - 1;
  emp::vector<std::string> files(num_files);
  for (size_t i = 0; i < num_files; i++) files[i] = argv[i+1];

  // Simplify to just the filenames (remove paths)
  emp::vector<std::string> filenames;
  for (std::string & file : files) {
    emp::vector<std::string_view> dir_struct = emp::view_slices(file, '/');
    std::string_view fileview = dir_struct.back();
    std::string filename(fileview);
    filenames.push_back( filename );
  }


  for (size_t i = 0; i < num_files; i++) {
    std::cout << files[i] << " : " << filenames[i] << "\n";
  }
}
