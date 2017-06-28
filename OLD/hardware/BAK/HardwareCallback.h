//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This class collects the callback functions that the hardware will use to interact with the
//  external world.

#ifndef EMP_HARDWARE_CALLBACK_H
#define EMP_HARDWARE_CALLBACK_H

#include <functional>

namespace emp {

  struct HardwareCallback {
    // Some default functions.  @CAO These can be implemented better...
    static int Default1I(int arg1) { (void) arg1; return 0; }
    static int Default2I(int arg1, int arg2) { (void) arg1, (void) arg2; return 0; }

    std::function<int(int)> get_input;        // Args: org ID               ; return new input.
    std::function<int(int,int)> send_output;  // Args: org ID, output value ; return success info.
    std::function<int(int,int)> do_divide;    // Args: org ID, memory space ; return success info.
    std::function<int(int,int)> do_inject;    // Args: org ID, memory space ; return success info.

    // Others that we might need later...
    // std::function<> send_message
    // std::function<> receive_message

    // std::function<> explode
    // std::function<> die
    // std::function<> kill

    // std::function<> get_deme_info
    
    // std::function<> get_facing
    // std::function<> get_speed
    // std::function<> move
    // std::function<> rotate

    // std::function<> get_resources
    // std::function<> add_resource

    // std::function<> update_priority


    HardwareCallback()
      : get_input(&Default1I)
      , send_output(&Default2I)
      , do_divide(&Default2I)
      , do_inject(&Default2I)
    { ; }
  };

};


#endif

