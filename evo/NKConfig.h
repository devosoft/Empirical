#ifndef NK_CONFIG_H
#define NK_CONFIG_H

#include "../config/config.h"

EMP_BUILD_CONFIG( NKConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  VALUE(K, int, 0, "Level of epistasis in the NK model"),
  VALUE(N, int, 100, "Number of bits in each organisms (must be > K)"), ALIAS(GENOME_SIZE),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, int, 100, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, int, 2000, "How many generations should we process?"),
  VALUE(TOURNAMENT_SIZE, int, 10, "How many orgs are chosen per tournament?"),
  VALUE(MUT_RATE, double, .0001, "Per-site mutation rate"),
  VALUE(FIT_SHARE, bool, false, "Per-site mutation rate"),
  VALUE(CHANGE_ENV, bool, false, "Per-site mutation rate")
)

#endif
