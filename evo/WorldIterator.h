#ifndef EMP_EVO_WORLD_ITER_H
#define EMP_EVO_WORLD_ITER_H

#include "World.h"

namespace emp{
namespace evo{

template <typename ORG, typename... MANAGERS> class World;

template <typename ORG, typename... MANAGERS>
class WorldIterator{
private:
  World<ORG, MANAGERS...>* world;
  int curr = 0;
  int size;

public:
  //typedef  iterator;
  //typedef ptrdiff_t difference_type;
  //typedef size_t size_type;
  //typedef T value_type;
  //typedef T * pointer;
  //typedef T & reference;

  WorldIterator(World<ORG, MANAGERS...> * w, int ind = 0){
    world  = w;
    curr = ind;
    size = w->GetSize();
    while (curr < size && !world->IsOccupied(curr)){
      ++curr;
    }
  }

  WorldIterator(WorldIterator * w){
    world  = w->world;
    curr = w->curr;
    size = w->size;
    while (curr < size && !world->IsOccupied(curr)){
      ++curr;
    }
  }

  WorldIterator<ORG, MANAGERS...> & operator++() {
    ++curr;
    while (curr < size && !world->IsOccupied(curr)){
      ++curr;
    }
    return *this;
  }

  WorldIterator<ORG, MANAGERS...> & operator--() {
    --curr;
    while (curr < size && !world->IsOccupied(curr)){
      --curr;
    }
    return *this;
  }

  bool operator==(const WorldIterator<ORG, MANAGERS...>& rhs){
    return curr == rhs.curr;
  }

  bool operator!=(const WorldIterator<ORG, MANAGERS...>& rhs){
    return curr != rhs.curr;
  }

  ORG & operator*() {return (*world)[curr];}

  WorldIterator<ORG, MANAGERS...> begin() {
    return WorldIterator<ORG, MANAGERS...>(world, 0);
  }

  WorldIterator<ORG, MANAGERS...> end() {
    return WorldIterator<ORG, MANAGERS...>(world, size);
  }


};
}
}
#endif
