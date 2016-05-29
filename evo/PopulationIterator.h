#ifndef EMP_EVO_POP_ITER_H
#define EMP_EVO_POP_ITER_H

namespace emp{
namespace evo{

template <typename POP_MANAGER>
class PopulationIterator{
private:
  using ORG = typename POP_MANAGER::value_type;
  POP_MANAGER * pop;
  int curr = 0;
  int size;

public:
  //typedef  iterator;
  //typedef ptrdiff_t difference_type;
  //typedef size_t size_type;
  //typedef T value_type;
  //typedef T * pointer;
  //typedef T & reference;

  PopulationIterator(POP_MANAGER * w, int ind = 0){
    pop  = w;
    curr = ind;
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

  bool operator==(const PopulationIterator<POP_MANAGER>& rhs){
    return curr == rhs.curr;
  }

  bool operator!=(const PopulationIterator<POP_MANAGER>& rhs){
    return curr != rhs.curr;
  }

  ORG & operator*() {return (*pop)[curr];}

  PopulationIterator<POP_MANAGER> begin() {
    return PopulationIterator<ORG>(pop, 0);
  }

  PopulationIterator<POP_MANAGER> end() {
    return PopulationIterator<POP_MANAGER>(pop, size);
  }


};
}
}
#endif
