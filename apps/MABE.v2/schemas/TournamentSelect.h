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
    EMP_BUILD_CONFIG( TSConfig,
      GROUP(DEFAULT_GROUP, "Tournament Selection Settings"),
      VALUE(TOURNAMENT_SIZE, size_t, 4, "Number of individuals chosen for each tournament."),
      VALUE(BIRTHS_PER_GENERATION, size_t, 500, "Number of tournaments to run each generation."),
    )  

    TSConfig config;

    using fit_fun_t = std::function< double(mabe::OrganismBase &) >;
    fit_fun_t fit_fun;
  public:
    TournamentSelect(const std::string & name) : SchemaBase(name) { ; }

    std::string GetClassName() const override { return "TournamentSelect"; }

    /// Required accessor for configuration objects.
    emp::Config & GetConfig() override { return config; }

    void SetFitFun( fit_fun_t in_fun ) { fit_fun = in_fun; }
  };

}

#endif

