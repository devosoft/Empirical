#ifndef EMP_CALLBACKS_H
#define EMP_CALLBACKS_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  A set of callback objects (functors) to manage interactions across subsystems 
//
//  Classes:
//  emp::Callback - abstract base class that defines a few key methods:
//    operator() - allows for a callback without any arguments
//    DoCallback(int *) - allows for a generic callback with a pointer as an argument.
//    IsDisposible() - Should this callback be deleted automatically after a single use?
//    SetDisposible() - Mark a callback as being disposible.
//
//  emp::MethodCallback - Allows for an object and a method on that object to be passed in.
//
//  emp::MethodCallback_Event - Triggers a method that takes EventInfo as its argument.
//

namespace emp {
  class Callback {
  private:
    bool is_disposible;  // Can this callback be deleted immediately after use?
  public:
    Callback() : is_disposible(false) { ; }
    virtual ~Callback() { ; }
    
    // virtual void operator() = 0;
    virtual void DoCallback(int * arg_ptr=NULL) = 0;
    
    bool IsDisposible() const { return is_disposible; }
    
    void SetDisposible(bool _in=true) { is_disposible = _in; }
  };
  

  template <typename RETURN, typename... Args> class FunctionCallback : public Callback {
  private:
    std::tuple<Args...> default_args;

  public:
    RETURN (*function_ptr)(Args...);
      public:
    FunctionCallback(RETURN (*_fun_ptr)(Args...))
      : function_ptr(_fun_ptr)
    { ; } 
    ~FunctionCallback() { ; }

    void DoCallback(int * arg_ptr) {
      // For the moment, ignore arg_ptr
      // function_ptr(default_args);
    }
  
    RETURN operator()(Args... args) { return function_ptr(args...); }
  };
  
  template <class T> class MethodCallback : public Callback {
  private:
    T * target;
    void (T::*method_ptr)();
  public:
    MethodCallback(T * _target, void (T::*_method_ptr)())
      : target(_target)
      , method_ptr(_method_ptr)
    { ; }
    ~MethodCallback() { ; }
    
    // void operator()() { (target->*(method_ptr))(); }
    void DoCallback(int * arg_ptr) { (void) arg_ptr; (target->*(method_ptr))(); }
  };
  
  
  // @CAO This is really ugly.  Fix this!!!!
  template <class T, class A> class MethodCallbackArg : public Callback {
  private:
    T * target;
    void (T::*method_ptr)(A);
  public:
    MethodCallbackArg(T * _target, void (T::*_method_ptr)(A))
      : target(_target)
      , method_ptr(_method_ptr)
    { ; }
    ~MethodCallbackArg() { ; }
    
    void DoCallback(int * arg_ptr) { (target->*(method_ptr))((A) arg_ptr); }
    void DoArgCallback(A arg) { (target->*(method_ptr))(arg); }
  };
  
  
  struct EventInfo {
    // Mouse / Touch:
    int layer_x;      // Coordinates of mouse in this layer.
    int layer_y;
    int button;       // Which button was pressed?
    
    // Keyboard:
    int key_code;     // Which key was pressed?
    bool alt_key;     // Modifier keys
    bool ctrl_key;
    bool meta_key;
    bool shift_key;  
    
    EventInfo(int lx, int ly, int but, int key, int alt, int ctrl, int meta, int shift)
      : layer_x(lx), layer_y(ly), button(but), key_code(key)
      , alt_key(alt), ctrl_key(ctrl), meta_key(meta), shift_key(shift) { ; }
  };

  
  template <class T> class MethodCallback_Event : public Callback {
  private:
    T * target;
    void (T::*method_ptr)(const EventInfo &);
  public:
    MethodCallback_Event(T * _target, void (T::*_method_ptr)(const EventInfo &))
      : target(_target)
      , method_ptr(_method_ptr)
    { ; }
    
    ~MethodCallback_Event() { ; }
    
    void DoCallback(int * arg_ptr) {
      EventInfo info(arg_ptr[0], arg_ptr[1], arg_ptr[2], arg_ptr[3], arg_ptr[4], arg_ptr[5], arg_ptr[6], arg_ptr[7]);
      (target->*(method_ptr))(info);
      
      // @CAO -- why can we just transfer this directly??
      // (target->*(method_ptr))( *((EventInfo *) arg_ptr) );
    }
  };

};

extern "C" void empJSDoCallback(int cb_ptr, int arg_ptr)
{
  emp::Callback * const cb_obj = (emp::Callback *) (long long) cb_ptr;
  cb_obj->DoCallback((int *) (long long) arg_ptr);
  
  if (cb_obj->IsDisposible()) {
    delete cb_obj;
  }
}



#endif
