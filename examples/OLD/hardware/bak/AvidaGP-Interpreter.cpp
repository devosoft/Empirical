#include <iostream>
#include <string>

#include "../../hardware/AvidaGP.h"
#include "../../hardware/InstLib.h"
#include "../../base/vector.h"
#include "../../tools/string_utils.h"



int main() {

    emp::AvidaGP cpu;
    cpu.PrintState(std::cout);

    emp::Ptr<const emp::InstLib<emp::AvidaGP> > lib = cpu.GetInstLib();
    std::cout << ">> ";
    std::string line; //stores the most recent line of input
    while(true) {
        std::getline(std::cin, line);
        if (line == "exit") {
            break;
        }

        emp::vector<std::string> command = emp::slice(line, ' ');

        if (!lib->IsInst(command[0])) {
            std::cout << "Unknown instruction" << std::endl;
            std::cout << ">> ";
            continue;
        }

        emp::vector<std::string> string_args(command.begin()+1, command.end());
        emp::vector<int> args = emp::from_strings<int>(string_args);
        size_t inst = lib->GetID(command[0]);

        if (lib->GetNumArgs(inst) != args.size()) {
            std::cout << "Wrong number of args" << std::endl;
            std::cout << ">> ";
            continue;
        }

        // There's probably a clever way to do this, but I'm not going to try right now
        switch(args.size()) {
        case 0:
            cpu.PushInst(inst);
            break;
        case 1:
            cpu.PushInst(inst, args[0]);
            break;
        case 2:
            cpu.PushInst(inst, args[0], args[1]);
            break;
        case 3:
            cpu.PushInst(inst, args[0], args[1], args[2]);
            break;
        default:
            std::cout << "Not implemented error. This inst has more than 3 args." <<
                         " Fix the swtich statement in the interpreter" << std::endl;
        }

        cpu.Process(1);
        cpu.PrintState(std::cout);
        std::cout << ">> ";
    }
}
