/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  Genome.hpp
 *  @brief This is a simple, container for a series of instructions.
 *
 */

#ifndef EMP_GENOME_H
#define EMP_GENOME_H

#include "../base/Ptr.hpp"


#include "../base/array.hpp"
#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../io/File.hpp"
#include "../math/Random.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  template < typename INST_T, typename INST_LIB_T, typename SEQUENCE_T=emp::vector<INST_T> >
  class Genome {
  private:
    emp::Ptr<const INST_LIB_T> inst_lib;
    SEQUENCE_T sequence;

  public:
    Genome() = default;
    Genome(emp::Ptr<const INST_LIB_T> _inst_lib, const SEQUENCE_T & _seq=SEQUENCE_T())
      : inst_lib(_inst_lib), sequence(_seq) { ; }
    Genome(const INST_LIB_T & _inst_lib, const SEQUENCE_T & _seq=SEQUENCE_T())
      : inst_lib(&_inst_lib), sequence(_seq) { ; }
    Genome(const Genome &) = default;
    Genome(Genome &&) = default;
    ~Genome() { ; }

    Genome & operator=(const Genome &) = default;
    Genome & operator=(Genome &&) = default;

    auto GetInstLib() const { return inst_lib; }
    INST_T & operator[](size_t id) { return sequence[id]; }
    const INST_T & operator[](size_t id) const { return sequence[id]; }

    bool operator==(const Genome& other) const { return sequence == other.sequence; }
    bool operator!=(const Genome& other) const { return sequence != other.sequence; }
    bool operator< (const Genome& other) const { return sequence <  other.sequence; }
    bool operator<=(const Genome& other) const { return sequence <= other.sequence; }
    bool operator> (const Genome& other) const { return sequence >  other.sequence; }
    bool operator>=(const Genome& other) const { return sequence >= other.sequence; }

    size_t Hash() const {
      std::size_t seed = sequence.size();
      for(auto& i : sequence) {
        seed ^= i.id + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
    }
    struct hash_t { size_t operator()(const Genome & g) const { return g.Hash(); } };

  };

}

#endif
