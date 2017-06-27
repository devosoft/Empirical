
#include <iostream>
#include <functional>
#include <string>
#include "../../hardware/MetaGP/Program.h"
#include "../../hardware/MetaGP/Machines.h"
#include "../../base/Ptr.h"

namespace emp {
  namespace MetaGP {
    class XFunctionDefinition : public FunctionDefinition_Base {
    protected:
      using FunctionDefinition_Base::affinity;
      using FunctionDefinition_Base::type;

      emp::vector<std::string> sequence;

    public:
      XFunctionDefinition(BitVector _affinity, emp::vector<std::string> _sequence)
        : FunctionDefinition_Base(_affinity, FunctionType::GP_X), sequence(_sequence)
      { ; }

    };

    class YFunctionDefinition : public FunctionDefinition_Base {
    protected:
      using FunctionDefinition_Base::affinity;
      using FunctionDefinition_Base::type;

      emp::vector<std::string> sequence;

    public:
      YFunctionDefinition(BitVector _affinity, emp::vector<std::string> _sequence)
        : FunctionDefinition_Base(_affinity, FunctionType::GP_Y), sequence(_sequence)
      { ; }

    };
  }
}

int main() {
  std::string prgm_fname = "./MetaPrograms/meta_program.txt";
  emp::MetaGP::ProgramReader reader;

  reader.RegisterFunctionReader("X",
                                [](std::string fun_def_str) {
                                  // Parse function definition.
                                  auto def = emp::string_pop_line(fun_def_str);
                                  emp::remove_punctuation(def);
                                  auto affinity_str = emp::slice(def, ' ')[2];
                                  emp::BitVector affinity(affinity_str.size());
                                  for (size_t c = 0; c < affinity_str.size(); c++) {
                                    if (affinity_str[c] == '0') affinity[c] = 0;
                                    else affinity[c] = '1';
                                  }
                                  emp::vector<std::string> seq;
                                  auto lines = emp::slice(fun_def_str);
                                  for (auto line : lines) {
                                    seq.push_back(line);
                                  }
                                  return new emp::MetaGP::XFunctionDefinition(affinity, seq);
                                });

  reader.RegisterFunctionReader("Y",
                                [](std::string fun_def_str) {
                                  // Parse function definition.
                                  auto def = emp::string_pop_line(fun_def_str);
                                  emp::remove_punctuation(def);
                                  auto affinity_str = emp::slice(def, ' ')[2];
                                  emp::BitVector affinity(affinity_str.size());
                                  for (size_t c = 0; c < affinity_str.size(); c++) {
                                    if (affinity_str[c] == '0') affinity[c] = 0;
                                    else affinity[c] = '1';
                                  }
                                  emp::vector<std::string> seq;
                                  auto lines = emp::slice(fun_def_str);
                                  for (auto line : lines) {
                                    seq.push_back(line);
                                  }
                                  return new emp::MetaGP::YFunctionDefinition(affinity, seq);
                                });

  reader.ParseProgramFromFile(prgm_fname);
}
