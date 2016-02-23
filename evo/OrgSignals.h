//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file manages some basic signals for organisms.

#ifndef ORG_SIGNALS_H
#define ORG_SIGNALS_H

#include <string>

#include "../tools/signal.h"
#include "../tools/string_utils.h"

namespace emp {
namespace evo {

  struct OrgSignals {    
    Signal<> repro_sig;

    OrgSignals(const std::string & sig_prefix)
      : repro_sig(to_string(sig_prefix, ":repro"))
    { ; }
  };

}
}

#endif
