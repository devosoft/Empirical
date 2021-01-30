// Adapted from Emily's memic_model project
// https://github.com/emilydolson/memic_model

#pragma once

#include "emp/config/config.hpp"

EMP_BUILD_CONFIG( Config,
  GROUP(MAIN, "Global settings"),
  VALUE(BOOL_EX, bool, true, "example description"),
  VALUE(SEED, int, -1, "Random number generator seed"),
  VALUE(TIME_STEPS, int, 1000, "Number of time steps to run for"),
  VALUE(PLATE_LENGTH, double, 10.0, "Length of plate in mm"),
  VALUE(PLATE_WIDTH, double, 6.0, "Width of plate in mm"),
  VALUE(PLATE_DEPTH, double, 1.45, "Depth of plate in mm"),
  VALUE(CELL_DIAMETER, double, 20.0, "Cell length and width in microns"),
  VALUE(INIT_POP_SIZE, int, 100, "Number of cells to seed population with"),
  VALUE(DATA_RESOLUTION, int, 10, "How many updates between printing data?"),

  GROUP(CELL, "Cell settings"),
  VALUE(NEUTRAL_MUTATION_RATE, double, .05, "Probability of a neutral mutation (only relevant for phylogenetic signature)"),
  VALUE(ASYMMETRIC_DIVISION_PROB, double, 0, "Probability of a change in stemness"),
  VALUE(MITOSIS_PROB, double, .5, "Probability of mitosis"),
  VALUE(HYPOXIA_DEATH_PROB, double, .25, "Probability of dieing, given hypoxic conditions"),
  VALUE(AGE_LIMIT, int, 100, "Age over which non-stem cells die"),
  VALUE(BASAL_OXYGEN_CONSUMPTION, double, .00075, "Base oxygen consumption rate"),
  VALUE(OXYGEN_CONSUMPTION_DIVISION, double, .00075*5, "Amount of oxygen a cell consumes on division"),

  GROUP(OXYGEN, "Oxygen settings"),
  VALUE(INITIAL_OXYGEN_LEVEL, double, .5, "Initial oxygen level (will be placed in all cells)"),
  VALUE(OXYGEN_DIFFUSION_COEFFICIENT, double, .1, "Oxygen diffusion coefficient"),
  VALUE(DIFFUSION_STEPS_PER_TIME_STEP, int, 100, "Rate at which diffusion is calculated relative to rest of model"),
  VALUE(OXYGEN_THRESHOLD, double, .1, "How much oxygen do cells need to survive?"),
  VALUE(KM, double, 0.01, "Michaelis-Menten kinetic parameter"),

  GROUP(TREATMENT, "Treatment settings"),
  VALUE(RADIATION_DOSES, int, 1, "Number of radiation doses to apply (for use in web interface - use a radiation prescription file for command-line)"),
  VALUE(RADIATION_DOSE_SIZE, double, 2, "Dose size (Gy) (for use in web interface - use a radiation prescription file for command-line)"),
  VALUE(RADIATION_PRESCRIPTION_FILE, std::string, "none", "File containing radiation prescription"),
  VALUE(K_OER, double, 3.28, "Effective OER constant"),
  VALUE(OER_MIN, double, 1, "OER min constant"),
  VALUE(OER_ALPHA_MAX, double, 1.75, "OER alpha max constant"),
  VALUE(OER_BETA_MAX, double, 3.25, "OER alpha (? this is what the paper says but I feel like it's supposed to be beta) max constant"),
);
