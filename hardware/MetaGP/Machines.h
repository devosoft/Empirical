#ifndef EMP_META_GP_MACHINES_H
#define EMP_META_GP_MACHINES_H

#include <iostream>

namespace emp {
  namespace MetaGP {

    enum class BlockType { NONE=0, BASIC, LOOP };

    struct Block {
      size_t begin;
      size_t end;
      BlockType type;

      Block(size_t _begin = 0, size_t _end = 0, BlockType _type = BlockType::BASIC)
        : begin(_begin), end(_end), type(_type) { ; }
    };

  }
}

#endif
