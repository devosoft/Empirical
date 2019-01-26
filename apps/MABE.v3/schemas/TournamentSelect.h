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

    void RunTournament(EvolverBase & world, size_t t_size, size_t tourny_count=1) {
      const size_t fitness_id = world.GetOrgDataID("fitness");

      emp::vector<size_t> entries;
      for (size_t T = 0; T < tourny_count; T++) {
        entries.resize(0);
        // Choose organisms for this tournament (with replacement!)
        for (size_t i=0; i < t_size; i++) entries.push_back( world.GetRandomOrgID() );

        double best_fit = world.GetOrgData<double>(world.GetOrg(entries[0]), fitness_id);
        size_t best_id = entries[0];

        // Search for a higher fit org in the tournament.
        for (size_t i = 1; i < t_size; i++) {
          const double cur_fit = world.GetOrgData<double>(world.GetOrg(entries[i]), fitness_id);
          if (cur_fit > best_fit) {
            best_fit = cur_fit;
            best_id = entries[i];
          }
        }

        // Place the highest fitness into the next generation!
        //world.DoBirth( world.GetGenomeAt(best_id), best_id, 1 );
        (void) best_id;
      }
    }

  };

}

#endif

