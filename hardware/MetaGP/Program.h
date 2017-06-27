#ifndef EMP_META_GP_PROGRAM_H
#define EMP_META_GP_PROGRAM_H

#include <sstream>
#include <iostream>
#include <functional>

#include "Hardware.h"
#include "../../tools/string_utils.h"
#include "../../tools/BitVector.h"
#include "../../tools/Lexer.h"
#include "../../base/vector.h"
#include "../../base/Ptr.h"
#include "../../tools/File.h"


namespace emp {
  namespace MetaGP {

    class FunctionDefinition_Base {
    protected:
      BitVector affinity;
      FunctionType type;

    public:
      FunctionDefinition_Base(BitVector _affinity, FunctionType _type)
        : affinity(_affinity), type(_type) { ; }

      virtual ~FunctionDefinition_Base() { ; }

      BitVector & GetAffinity() { return affinity; }
      const BitVector & GetConstAffinity() const { return affinity; }
      FunctionType GetType() { return type; }

      void SetAffinity(BitVector & aff) { affinity = aff; }
      void SetType(FunctionType t) { type = t; }

      virtual std::string PrettyString() const {
        std::stringstream pretty_str;
        pretty_str << "def unknown " << affinity << std::endl;
        return pretty_str.str();
      }

    };

    class Program {
    protected:
      using func_def_t = FunctionDefinition_Base;

      emp::vector<Ptr<func_def_t>> functions;

    public:
      Program() : functions() { ; }

      // Delete all functions.
      void ClearProgram() {
        for (size_t i = 0; i < functions.size(); i++) {
          delete functions[i];
        } functions.resize(0);
      }

      emp::vector<Ptr<func_def_t>> & GetProgram() { return functions; }
      const emp::vector<Ptr<func_def_t>> & GetConstProgram() const { return functions; }
      size_t GetSize() const { return functions.size(); }

      Ptr<func_def_t> operator[](size_t index) { emp_assert(index < GetSize()); return functions[index]; }

      void AddFunction(Ptr<func_def_t> fun) {
        functions.push_back(fun);
      }

      std::string PrettyString() const {
        std::stringstream pretty_str;
        for (size_t i = 0; i < functions.size(); i++) {
          pretty_str << functions[i]->PrettyString() << std::endl;
        }
        return pretty_str.str();
      }

    };

    // Utility class to read & generate programs from strings/files.
    class ProgramReader {
    protected:
      using reader_func_t = std::function<Ptr<FunctionDefinition_Base>(std::string)>;

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
