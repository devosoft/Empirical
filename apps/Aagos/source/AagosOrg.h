#include "tools/BitVector.h"

class AagosOrg {
private:
  emp::BitVector bits;
  size_t gene_size;
  emp::vector<size_t> gene_starts;

public:
  AagosOrg() { ; }
  AagosOrg(const AagosOrg &) = default;
  AagosOrg(AagosOrg &&) = default;
  ~AagosOrg() { ; }

  AagosOrg & operator=(const AagosOrg &) = default;
  AagosOrg & operator=(AagosOrg &&) = default;

  size_t GetNumBits() const { return bits.size(); }
  size_t GetNumGenes() const { return gene_starts.size(); }

  const emp::BitVector & GetBits() const { return bits; }
  const emp::vector<size_t> & GetGeneStarts() const { return gene_starts; }
};
