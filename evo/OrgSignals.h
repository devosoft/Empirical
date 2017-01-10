//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file manages some basic signals for organisms.

#ifndef ORG_SIGNALS_H
#define ORG_SIGNALS_H

#include <string>

#include "../control/Action.h"
#include "../control/Signal.h"
#include "../tools/string_utils.h"

namespace emp {
namespace evo {

  struct OrgSignals_NONE {
    static std::string GetName() { return "OrgSignals_NONE"; }

    OrgSignals_NONE(const std::string & sig_prefix) { ; }
  };

  struct OrgSignals_Basic {
    static std::string GetName() { return "OrgSignals_Basic"; }

    Signal<size_t> repro_sig;
    std::string prefix;

    OrgSignals_Basic(const std::string & sig_prefix)
      : repro_sig(to_string(sig_prefix, "::do-repro")), prefix(sig_prefix)
    { ; }
  };


  // A more detailed set of signals with ecological interactions of all sorts.
  struct OrgSignals_Eco : public OrgSignals_Basic {
    static std::string GetName() { return "OrgSignals_Eco"; }

    Signal<size_t> symbiont_repro_sig;  // Identify organism with symbiont reproducing.

    OrgSignals_Eco(const std::string & sig_prefix)
      : OrgSignals_Basic(sig_prefix)
      , symbiont_repro_sig(to_string(sig_prefix, "::do-symbiont_repro"))
    { ; }
  };

}
}

#endif
