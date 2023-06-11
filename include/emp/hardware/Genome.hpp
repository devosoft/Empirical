/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file Genome.hpp
 *  @brief This is a simple, container for a series of instructions.
 *
 */

#ifndef EMP_HARDWARE_GENOME_HPP_INCLUDE
#define EMP_HARDWARE_GENOME_HPP_INCLUDE

#include "../base/Ptr.hpp"


#include "../base/array.hpp"
#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../io/File.hpp"
#include "../math/Random.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  template < typename INST_T, typename INST_LIB_T, typename SEQUENCE_T=emp::vector<INST_T> >
  class Genome : public SEQUENCE_T {
  private:
    emp::Ptr<const INST_LIB_T> inst_lib;

  public:
    Genome() = default;
    Genome(emp::Ptr<const INST_LIB_T> _inst_lib, const SEQUENCE_T & _seq=SEQUENCE_T())
      : SEQUENCE_T(_seq), inst_lib(_inst_lib) { ; }
    Genome(const INST_LIB_T & _inst_lib, const SEQUENCE_T & _seq=SEQUENCE_T())
      : SEQUENCE_T(_seq), inst_lib(&_inst_lib) { ; }
    Genome(const Genome &) = default;
    Genome(Genome &&) = default;
    ~Genome() { ; }

    Genome & operator=(const Genome &) = default;
    Genome & operator=(Genome &&) = default;

    auto GetInstLib() const { return inst_lib; }

    size_t GetSize() const { return SEQUENCE_T::size(); }
    void Resize(size_t new_size) { SEQUENCE_T::resize(new_size); }

    bool operator==(const Genome& other) const {
      if (SEQUENCE_T::size() != other.size()) return false;
      auto it1 = SEQUENCE_T::begin();
      auto it2 = other.begin();
      while (it1 != SEQUENCE_T::end() && it2 != other.end()) {
        if (*it1 != *it2) return false;
        ++it1; ++it2;
      }
      return true;
    }
    bool operator!=(const Genome& other) const { return !(*this == other); }

    /// Compare genomes using an equivilent of alphabetical order.
    bool operator< (const Genome& other) const {
      // Step through the instructions
      auto it1 = SEQUENCE_T::begin();
      auto it2 = other.begin();
      while (it1 != SEQUENCE_T::end() && it2 != other.end()) {
        if (*it1 != *it2) return *it1 < *it2;  // The first difference determines result
        ++it1; ++it2;
      }
      // If sequences have been identical so far, shorter comes first (tie is false)
      return SEQUENCE_T::size() < other.size();
    }

    bool operator> (const Genome& other) const { return other < *this; }
    bool operator<=(const Genome& other) const { return !(other > *this); }
    bool operator>=(const Genome& other) const { return !(other < *this); }

    size_t Hash() const {
      std::size_t seed = GetSize();
      for(auto& i : *this) {
        seed ^= i.id + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
    }
    struct hash_t { size_t operator()(const Genome & g) const { return g.Hash(); } };

  };

}

#endif // #ifndef EMP_HARDWARE_GENOME_HPP_INCLUDE
