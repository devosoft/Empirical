/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  TournamentSelect.h
 *  @brief Setup up configuration options to run tournament selection.
 */

#ifndef MABE_TOURNAMENT_SELECT_H
#define MABE_TOURNAMENT_SELECT_H

#include "core/SchemaBase.h"

namespace mabe {

  class TournamentSelect : public SchemaBase {
  private:
  public:
    TournamentSelect(const std::string & name) : SchemaBase(name) { ; }
  };

}

#endif

