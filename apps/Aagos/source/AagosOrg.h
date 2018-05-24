#ifndef AAGOS_ORG_H
#define AAGOS_ORG_H

#include "tools/BitVector.h"
#include "tools/Random.h"
#include "tools/random_utils.h"
#include "tools/string_utils.h"

class AagosOrg {
  friend class AagosWorld;
private:
  emp::BitVector bits;
  emp::vector<size_t> gene_starts;
  size_t gene_size;

public:
  AagosOrg(size_t num_bits=64, size_t num_genes=64, size_t in_gene_size=8)
   : bits(num_bits), gene_starts(num_genes,0), gene_size(in_gene_size)
  {
    emp_assert(num_bits > 0, num_bits);
    emp_assert(num_genes > 0, num_genes);
    emp_assert(gene_size > 0, gene_size);
  }
  AagosOrg(const AagosOrg &) = default;
  AagosOrg(AagosOrg &&) = default;
  ~AagosOrg() { ; }

  AagosOrg & operator=(const AagosOrg &) = default;
  AagosOrg & operator=(AagosOrg &&) = default;

  size_t GetNumBits() const { return bits.size(); }
  size_t GetNumGenes() const { return gene_starts.size(); }

  const emp::BitVector & GetBits() const { return bits; }
  const emp::vector<size_t> & GetGeneStarts() const { return gene_starts; }

  void Randomize(emp::Random & random) {
    emp::RandomizeBitVector(bits, random);
    emp::RandomizeVector<size_t>(gene_starts, random, 0, bits.size());
  }

  void Print(std::ostream & is=std::cout) const {
    is << "Bits: " << bits << '\n';
    is << "Gene Starts: " << emp::to_string(gene_starts) << std::endl;
  }
};

#endif
