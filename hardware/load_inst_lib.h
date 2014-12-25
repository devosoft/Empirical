#ifndef EMP_LOAD_INST_LIB_H
#define EMP_LOAD_INST_LIB_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains tools to load instructions sets.
//

#include <functional>

#include "HardwareCPU.h"
#include "InstLib.h"

namespece emp {

  Load4StackDefault(InstLib<HardwareCPU, Instruction> & inst_lib) {
    inst_lib.AddInst("Nop-A", std::bind(&HardwareCPU::Inst_Nop, _1), 1, 0);
    inst_lib.AddInst("Nop-B", std::bind(&HardwareCPU::Inst_Nop, _1), 1, 1);
    inst_lib.AddInst("Nop-C", std::bind(&HardwareCPU::Inst_Nop, _1), 1, 2);
    inst_lib.AddInst("Nop-D", std::bind(&HardwareCPU::Inst_Nop, _1), 1, 3);
    inst_lib.AddInst("Nop-E", std::bind(&HardwareCPU::Inst_Nop, _1), 1, 4);
    inst_lib.AddInst("Nop-F", std::bind(&HardwareCPU::Inst_Nop, _1), 1, 5);
    inst_lib.AddInst("Nop-G", std::bind(&HardwareCPU::Inst_Nop, _1), 1, 6);
    inst_lib.AddInst("Nop-H", std::bind(&HardwareCPU::Inst_Nop, _1), 1, 7);

    inst_lib.AddInst("Inc", std::bind(&HardwareCPU::Inst_AddConst, _1, 1, 1, 1));
    inst_lib.AddInst("Dec", std::bind(&HardwareCPU::Inst_AddConst, _1, -1, 1, 1));
    inst_lib.AddInst("Shift-L", std::bind(&HardwareCPU::Inst_Shift, _1, 1, 1, 1));
    inst_lib.AddInst("Shift-R", std::bind(&HardwareCPU::Inst_Shift, _1, -1, 1, 1));

    inst_lib.AddInst("Nand", std::bind(&HardwareCPU::Inst_Nand, _1, 1, 2, 3));
    inst_lib.AddInst("Add",  std::bind(&HardwareCPU::Inst_Add,  _1, 1, 2, 3));
    inst_lib.AddInst("Sub",  std::bind(&HardwareCPU::Inst_Sub,  _1, 1, 2, 3));
    inst_lib.AddInst("Mult", std::bind(&HardwareCPU::Inst_Mult, _1, 1, 2, 3));
    inst_lib.AddInst("Div",  std::bind(&HardwareCPU::Inst_Div,  _1, 1, 2, 3));
    inst_lib.AddInst("Mod",  std::bind(&HardwareCPU::Inst_Mod,  _1, 1, 2, 3));
  }

};

#endif
