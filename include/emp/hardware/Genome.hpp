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

    // INST_T & operator[](size_t id) { return sequence[id]; }
    // const INST_T & operator[](size_t id) const { return sequence[id]; }

    size_t GetSize() const { return SEQUENCE_T::size(); }
    // size_t size() const { return sequence.size(); }    // For compatability with std.

    void Resize(size_t new_size) { SEQUENCE_T::resize(new_size); }
    // size_t resize(size_t new_size) { return sequence.resize(new_size); }    // For compatability with std.

    bool operator==(const Genome& other) const { return SEQUENCE_T::operator==(*this, other); }
    bool operator!=(const Genome& other) const { return SEQUENCE_T::operator!=(*this, other); }
    bool operator< (const Genome& other) const { return SEQUENCE_T::operator<(*this,  other); }
    bool operator<=(const Genome& other) const { return SEQUENCE_T::operator<=(*this, other); }
    bool operator> (const Genome& other) const { return SEQUENCE_T::operator>(*this,  other); }
    bool operator>=(const Genome& other) const { return SEQUENCE_T::operator>=(*this, other); }

    // auto begin() { return sequence.begin(); }
    // auto begin() const { return sequence.begin(); }
    // auto end() { return sequence.end(); }
    // auto end() const { return sequence.end(); }
    
    // auto rbegin() { return sequence.rbegin(); }
    // auto rbegin() const { return sequence.rbegin(); }
    // auto rend() { return sequence.rend(); }
    // auto rend() const { return sequence.rend(); }

    // auto cbegin() const { return sequence.cbegin(); }
    // auto cend() const { return sequence.cend(); }
    // auto crbegin() const { return sequence.crbegin(); }
    // auto crend() const { return sequence.crend(); }

    // template <typename... Ts>
    // void emplace_back(Ts... args) { sequence.emplace_back(std::forward<Ts>(args)...); }

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

#endif
