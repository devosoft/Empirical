#ifndef EMP_META_GP_PROGRAM_H
#define EMP_META_GP_PROGRAM_H

#include <sstream>
#include <iostream>
#include <functional>

#include "Hardware.h"
#include "Machines.h"
#include "../../tools/string_utils.h"
#include "../../tools/BitVector.h"
#include "../../tools/Lexer.h"
#include "../../base/vector.h"
#include "../../base/Ptr.h"
#include "../../tools/File.h"


namespace emp {
  namespace MetaGP {

    // A program consists of a set of functions or machines.
    // Machines are of arbitrary types (e.g. linear GP, ANN, MNB, etc).
    class Program {
    protected:
      using machine_t = Machine_Base;

      emp::vector<Ptr<machine_t>> machines;

    public:
      Program() : machines() { ; }

      // Delete all functions.
      void ClearProgram() {
        for (size_t i = 0; i < machines.size(); i++) {
          delete machines[i];
        } machines.resize(0);
      }

      emp::vector<Ptr<machine_t>> & GetProgram() { return machines; }
      const emp::vector<Ptr<machine_t>> & GetConstProgram() const { return machines; }
      size_t GetSize() const { return machines.size(); }

      Ptr<machine_t> operator[](size_t index) { emp_assert(index < GetSize()); return machines[index]; }

      void AddFunction(Ptr<machine_t> fun) {
        machines.push_back(fun);
      }

      std::string PrettyString() const {
        std::stringstream pretty_str;
        for (size_t i = 0; i < machines.size(); i++) {
          pretty_str << machines[i]->PrettyString() << std::endl;
        }
        return pretty_str.str();
      }

    };

    // Utility class to read & generate programs from strings/files.
    class ProgramReader {
    protected:
      using reader_func_t = std::function<Ptr<Machine_Base>(std::string)>;

      std::unordered_map<std::string, reader_func_t> function_readers;

    public:
      ProgramReader() : function_readers() { ; }

      void RegisterFunctionReader(std::string type_name, reader_func_t read_fun) {
        function_readers[type_name] = read_fun;
      }

      void ParseProgramFromFile(std::string & file_name) {
        emp::File prgm_file(file_name);
        ParseProgramFromFile(prgm_file);

      }
      void ParseProgramFromFile(emp::File & prgm_file) {
        // @amlalejini - TODO: clean up ways of reading programs from files.
        // Use lexer to process each line.
        Program prgm;
        std::string cur_function = "";
        std::string cur_function_name = "";
        for (size_t i = 0; i < prgm_file.GetNumLines(); i++) {
          // Process tokens in line.
          std::string line = prgm_file[i]; // Copy out line to work with.
          compress_whitespace(line);
          if (string_get_word(line) == "DEF") {
            // Clip previous function
            if (cur_function_name != "") {
              if (function_readers.count(cur_function_name) == 0)
                std::cerr << "No function reader registered for \"" << cur_function_name << "\". Skipping..." << std::endl;
              else
                prgm.AddFunction(function_readers[cur_function_name](cur_function));
            }
            // Get function name.
            cur_function = line + "\n";
            string_pop_word(line); cur_function_name = string_get_word(line);
          } else {
            cur_function += line + "\n";
          }
        }
        // Clip previous function.
        if (cur_function_name != "") {
          if (function_readers.count(cur_function_name) == 0)
            std::cerr << "No function reader registered for \"" << cur_function_name << "\". Skipping..." << std::endl;
          else
            prgm.AddFunction(function_readers[cur_function_name](cur_function));
        }
      }
    };

  }
}

#endif
