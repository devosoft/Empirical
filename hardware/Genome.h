#ifndef EMP_GENOME_H
#define EMP_GENOME_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This class represents a series of instructions, used as the genetic basis of organisms.
//

#include <vector>

#include "Instruction.h"

namespace emp {
  class InstLib;

  class Genome {
  private:
    std::vector<Instruction> genome;

  public:
    Genome(int length=0) : genome(length) { ; }
    Genome(const Genome & _in) : genome(_in.genome) { ; }
    ~Genome() { ; }

    Genome & operator=(const Genome & _in) { genome = _in.genome; }
    Instruction & operator[](int index) { return genome[index]; }
    const Instruction & operator[](int index) const { return genome[index]; }

    bool operator==(const Genome & _in) const { return genome == _in.genome; }
    bool operator!=(const Genome & _in) const { return !operator==(_in); }

    int GetSize() const { return (int) genome.size(); }
  };

};

#endif
