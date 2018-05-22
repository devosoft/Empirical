#include "source/tools/BitVector.h"

class AagosOrg {
private:
  emp::BitVector bits;
  emp::vector<size_t> gene_starts;

public:
  Aagos() { ; }
  ~Aagos() { ; }

  size_t GetNumBits() const { return genome.size(); }
  size_t GetNumGenes() const { return gen_pos.size(); }

  const emp::BitVector & GetBits() const { return bits; }
  const emp::vector<size_t> & GetGeneStarts() const { return gene_starts; }
};
