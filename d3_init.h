#ifndef D3_INIT_H
#define D3_INIT_H

#include "../Empirical/web/web_init.h"
#include <iostream>
namespace D3 {
  class D3_Base {
  protected:
    int id;
    D3_Base(){
      this->id = EM_ASM_INT_V({return js.objects.length});
    }
    D3_Base(int id){
      this->id = id;
    }

  public:
    int GetID() {
      return this->id;
    }
  };

}

#endif
