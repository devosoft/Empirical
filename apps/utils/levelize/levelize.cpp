#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "../../../include/emp/base/vector.hpp"
#include "../../../include/emp/io/File.hpp"
#include "../../../include/emp/tools/string_utils.hpp"

struct FileInfo {
  std::string filename;
  std::string path;
  std::set<std::string> depends; // Which OTHER files does this one depend on?
};

int main(int argc, char * argv[])
{
  // Load in all of the files that we are working with.
  size_t num_files = argc - 1;
  emp::vector<std::string> files(num_files);
  for (size_t i = 0; i < num_files; i++) files[i] = argv[i+1];

  // Simplify to just the filenames (remove paths)
  std::map<std::string, FileInfo> file_map;
  emp::vector<std::string> filenames;
  for (std::string & file : files) {
    emp::vector<std::string_view> dir_struct = emp::view_slices(file, '/');
    std::string filename(dir_struct.back());
    file_map[filename].filename = filename;
    file_map[filename].path = file;
    filenames.push_back(filename);
  }

  // For each file, scan for its dependencies.
  for (auto & [filename, info] : file_map) {
    emp::File file(info.path);
    file.KeepIf( [](const std::string & line){ return line.find("#include") != std::string::npos; } );
  }

  // List out the files.
  for (auto [filename, info] : file_map) {
    std::cout << info.path << " : " << filename << "\n";
  }
}
