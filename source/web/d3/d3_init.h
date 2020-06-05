#ifndef D3_INIT_H
#define D3_INIT_H

namespace D3 {
namespace internal {

  /// @cond __EMSCRIPTEN___STUFF
  extern "C" {
    extern int get_emp_d3();
  }
  /// @endcond

  EM_JS(size_t, NextD3ID, (), {
    const id = emp_d3.next_id++;
    emp_d3.counts[id] = 0;
    emp_d3.objects[id] = -1;
    return id;
  });

}
}

#endif