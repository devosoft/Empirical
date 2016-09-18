//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines iterators for use with a PopulationManager in an emp::evo::World

#ifndef EMP_EVO_POP_ITER_H
#define EMP_EVO_POP_ITER_H

namespace emp{
namespace evo{

  template <typename POP_MANAGER>
  class PopulationIterator {
  private:
    using ORG = typename POP_MANAGER::value_type;

    POP_MANAGER * pop;
    int curr = 0;
    int size;

  public:

    PopulationIterator(POP_MANAGER * pm, int ind=0) : pop(pm), curr(ind) {
      size = pop->size();
      while ((curr < size) && (((*pop)[curr]) == nullptr)){
        ++curr;
      }
    }

    PopulationIterator(PopulationIterator * w){
      pop  = w->pop;
      curr = w->curr;
      size = w->size;
      while (curr < size && (((*pop)[curr]) == nullptr)) {
        ++curr;
      }
    }

    PopulationIterator<POP_MANAGER> & operator++() {
      ++curr;
      while (curr < size && (((*pop)[curr]) == nullptr)) {
        ++curr;
      }
      return *this;
    }

    PopulationIterator<POP_MANAGER> & operator--() {
      --curr;
      while (curr < size && (((*pop)[curr]) == nullptr)) {
        --curr;
      }
      return *this;
    }

    bool operator==(const PopulationIterator<POP_MANAGER>& rhs) {
      return curr == rhs.curr;
    }

    bool operator!=(const PopulationIterator<POP_MANAGER>& rhs) {
      return curr != rhs.curr;
    }

    ORG & operator*() { return (*pop)[curr]; }

    PopulationIterator<POP_MANAGER> begin() {
      return PopulationIterator<POP_MANAGER>(pop, 0);
    }

    PopulationIterator<POP_MANAGER> end() {
      return PopulationIterator<POP_MANAGER>(pop, size);
    }

  };
}
}
#endif
