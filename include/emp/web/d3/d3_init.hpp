#ifndef D3_INIT_H
#define D3_INIT_H

#include "../web.h"

namespace D3 {
namespace internal {

  // This forces the javascript array that holds
  // d3 objects to get initialized
  /// @cond __EMSCRIPTEN___STUFF
  extern "C" {
    extern void get_emp_d3();
  }
  /// @endcond


  /// Create Javascript function called NextD3ID()
  /// that increments the ID of the next d3 object to
  /// be created
  EM_JS(size_t, NextD3ID, (), {
    const id = emp_d3.next_id++;
    emp_d3.counts[id] = 0;
    emp_d3.objects[id] = -1;
    return id;
  });

}

  /// This is a convenience function used to initialize the Empirical web/D3 layout so the user doesn't have to
  static void InitializeEmpD3() {
    static bool init = false;      // Make sure we only initialize once!
    if (!init) {
      internal::get_emp_d3();      // Call JS initializations
      emp::Initialize();
      init = true;
    }
  };

  /// A base object that all D3 objects inherit from. Handles storing the object in Javascript
  /// You probably don't want to instantiate this directly
  class D3_Base {
  protected:
    int id;

    /// Default constructor - adds placeholder to emp_d3.objects array in Javascript
    D3_Base() {
      InitializeEmpD3();
      this->id = internal::NextD3ID();
      MAIN_THREAD_EM_ASM({
        emp_d3.counts[$0] = 1;
      }, this->id);
    }

    /// Construct an object pointing to a pre-determined location in emp_d3.objects.
    /// Warning: This trusts that you know what you're doing in choosing an id.
    D3_Base(int id){
      InitializeEmpD3();
      this->id = id;

      emp_assert(MAIN_THREAD_EM_ASM_INT({ return $0 in emp_d3.counts;}, this->id));

      MAIN_THREAD_EM_ASM({
        emp_d3.counts[$0]++;
      }, this->id);
    }

    D3_Base(const D3_Base & other) {
      InitializeEmpD3();
      // make deep copy?
      this->id = other.id;
      MAIN_THREAD_EM_ASM({
        emp_d3.counts[$0]++;
      }, this->id);
    }

    D3_Base & operator=(const D3_Base & other) {
      this->id = other.id;
      MAIN_THREAD_EM_ASM({
        emp_d3.counts[$0]++;
      }, this->id);
      return (*this);
    }

    ~D3_Base() {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        emp_d3.counts[id]--;
        if(emp_d3.counts[id] == 0) {
          delete emp_d3.objects[id];
          delete emp_d3.counts[id];
        }
      }, this->id);
    }

  public:

    /// Get this object's ID (i.e. it's location in the emp_d3.objects array in Javascript)
    int GetID() const {
      return this->id;
    }

    void Log() const {
      MAIN_THREAD_EM_ASM({
          console.log($0+":", emp_d3.objects[$0]);
        }, this->id);
    }
  };

  /// Catch-all object for storing references to things created in JS
  class JSObject : public D3_Base {
  public:
      JSObject() {;};
  };

}

#endif