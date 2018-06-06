#ifndef OPEN_WORLD_CONFIG_H
#define OPEN_WORLD_CONFIG_H

#include "config/config.h"

EMP_BUILD_CONFIG( OpenWorldConfig,
  GROUP(WORLD_STRUCTURE, "How should each organism's genome be setup?"),
  VALUE(MAX_POP_SIZE, size_t, 4000, "What are the most organisms that should be allowed in pop?"),
  VALUE(MAX_GENS, size_t, 10000, "How many generations should the runs go for?"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
		  
  GROUP(ENVIRONMENT, "How do environmental interactions work?"),
  VALUE(MIN_ORG_SIZE, double, 1.0, "Minimum size for each organism."),
  VALUE(MAX_ORG_SIZE, double, 10.0, "Maximum size of each organism."),
  VALUE(MIN_CONSUME_RATIO, double, 0.25, "A predator cannot consume anything propotionately smaller than this."),
  VALUE(MAX_CONSUME_RATIO, double, 0.5, "A predator cannot consume anything propotionately larger than this."),

  GROUP(MUTATIONS, "Various mutation rates for SignalGP Brains"),
  VALUE(POINT_MUTATE_PROB, double, 0.001, "Probability of instructions being mutated"),
  VALUE(BIT_FLIP_PROB, double, 0.00001, "Probability of each tag bit toggling"),

  GROUP(OUTPUT, "Output rates for OpenWorld"),
  VALUE(PRINT_INTERVAL, size_t, 100, "How many updates between prints?")
)

#endif
