//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in statistics managers for use with emp::evo::World

#ifndef EMP_EVO_STATS_MANAGER_H
#define EMP_EVO_STATS_MANAGER_H

#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdarg>

#include "../base/vector.h"
#include "../config/config.h"
#include "../tools/FunctionSet.h"
#include "../tools/stats.h"

#include "PopulationManager.h"
#include "EvoStats.h"
#include "LineageTracker.h"

namespace emp{
namespace evo{

  EMP_BUILD_CONFIG( StatsManagerConfig,
    VALUE(RESOLUTION, int, 10, "How often should stats be calculated (updates)"),
    VALUE(DELIMITER, std::string, " ", "What should fields be separated by in the output")
  )

  // Base stats manager - this mostly exists to be extended into custom
  // stats managers (see the OEEStatsManager for an example). The base
  // stats manager also handles data output.
  template <typename POP_MANAGER=PopulationManager_Base<int>>
  class StatsManager_Base {
  public:
    static constexpr bool emp_is_stats_manager = true;

    std::string delimiter = " ";      // Gets inferred from file name
    int resolution = 10;              // With what frequency do we record data?
    std::ofstream output_location;    // Where does output go?
    emp::vector<std::string> col_map; // Vector for tracking variables

    StatsManager_Base(std::string location = "cout") {
      StatsManagerConfig config;
      config.Read("StatsConfig.cfg");
      resolution = config.RESOLUTION();
      delimiter = config.DELIMITER();
      config.Write("StatsConfig.cfg");
      SetOutput(location);
    }

    StatsManager_Base(StatsManagerConfig & config, std::string config_location,
                      std::string location = "cout") {
      config.Read(config_location);
      resolution = config.RESOLUTION();
      delimiter = config.DELIMITER();
      SetOutput(location);
    }

    ~StatsManager_Base() {
      output_location.close();
    }

    // No setup to do in base stats manager...
    template <typename WORLD> void Setup(WORLD * w) {;}
    template <typename T> void SetDefaultFitnessFun(const T &) {;}

    // Tells the stats manager where to put output. If location is "cout"
    // (default) or "stdout", stats will get sent to cout. Otherwise, the
    // specified file will be used as the location for output. If the file
    // has the extension "csv" or "tsv", the appropriate delimiter will be used.
    // If the location is invalid, the program will exit with an error.
    void SetOutput(std::string location) {
      if (location == "cout" || location == "stdout") {
        output_location.copyfmt(std::cout);
        output_location.clear(std::cout.rdstate());
        output_location.basic_ios<char>::rdbuf(std::cout.rdbuf());
      } else {
        if (output_location.is_open()) output_location.close();
        output_location.open(location);
        if (!output_location.good()) {
          std::cout << "Invalid output file. Exiting." << std::endl;
          exit(0);  // @CAO: We should run this through a proper error-processing system...
        }
        string_pop(location, ".");
        if (location == "csv")      delimiter = ", ";
        else if (location == "tsv") delimiter = "\t";
        else if (location == "dat") delimiter = " ";
      }
    }

  };


  // This is currently an interface for sending a population pointer to a visualization
  // Eventually: call all organisms' serialization function?
  // Eventually: create a way to turn off outputting to files?

  template <typename POP_MANAGER = PopulationManager_Base<int> >
  class StatsManager_WholePopulation : StatsManager_Base<POP_MANAGER> {
  protected:
    POP_MANAGER * pop;
    using base_t = StatsManager_Base<POP_MANAGER>;
    using base_t::resolution;
    using base_t::output_location;
    using base_t::delimiter;
    using base_t::col_map;
  public:
    using org_ptr = typename POP_MANAGER::value_type;
    using ORG = typename org_ptr::value_type;
    using base_t::SetDefaultFitnessFun;

    // Constructor for creating a stand-alone stats manager.
    template <typename WORLD>
    StatsManager_WholePopulation(WORLD * w, std::string location = "stats.csv")
     : StatsManager_Base<decltype(w->popM)>(location) {
      Setup(w);
    }

    // Constructor for use by World object
    StatsManager_WholePopulation(std::string location = "stats.csv") : base_t(location){;}

    // The fitness function for calculating fitness related stats
    template <typename WORLD>
    void Setup(WORLD * w){
      pop = &(w->popM);
      std::function<void(int)> UpdateFun = [&] (int ud){ Update(ud); };
      w->OnUpdate(UpdateFun);
    }

    void Update(int update) {}
  };

  // @ELD: Should be templated so the function can return things other than double
  //       ...But that will currently screw up the templating on world
  template <typename POP_MANAGER = PopulationManager_Base<int> >
  class StatsManager_WholePopulation_Function : StatsManager_WholePopulation<POP_MANAGER> {
  protected:
    POP_MANAGER * pop;
    using base_t = StatsManager_Base<POP_MANAGER>;
    using base_t::resolution;
    using base_t::output_location;
    using base_t::delimiter;
    using base_t::col_map;

  public:
    using org_ptr = typename POP_MANAGER::value_type;
    using ORG = typename org_ptr::value_type;
    using fun_type = std::function<double(ORG *)>;
    using base_t::SetDefaultFitnessFun;
    fun_type func;

    // Constructor for creating this as a stand-alone object
    template <typename WORLD>
    StatsManager_WholePopulation_Function(WORLD * w, std::string location = "stats.csv")
      : StatsManager_WholePopulation<decltype(w->popM)>(location) {
      Setup(w);
    }

    // Constructor for use by World object
    StatsManager_WholePopulation_Function(std::string location = "stats.csv")
     : StatsManager_WholePopulation<POP_MANAGER>(location) {;}

    // The fitness function for calculating fitness related stats
    template <typename WORLD>
    void Setup(WORLD * w){
      pop = &(w->popM);
      std::function<void(int)> UpdateFun = [&] (int ud){ Update(ud); };
      w->OnUpdate(UpdateFun);
    }

    template <typename T>
    void SetFunc(std::function<double(T)> f){func = f;}

    void Update(int update) {;}
  };


  // Stats managers will often need to print a set of statistics every so many updates.
  // This generic stats manager maintains FunctionSets containing all of the functions to be run.
  // Functions can be added dynamically -or- this class can be extended to track specific functions.
  // (see StatsManager_DefaultStats for an example)

  template <typename POP_MANAGER = PopulationManager_Base<int> >
  class StatsManager_FunctionsOnUpdate : StatsManager_Base<POP_MANAGER> {
  protected:
    using org_ptr = typename POP_MANAGER::value_type;
    using ORG = typename org_ptr::value_type;
    // using world_type = World<ORG, MANAGERS...>;
    using fit_fun_type = std::function<double(ORG *)>;
    FunctionSet<double()> stats;  // Stats calculated on the world

    // Pointer to the world object on which we're calculating stats
    POP_MANAGER * pop;
    using base_t = StatsManager_Base<POP_MANAGER>;
    using base_t::resolution;
    using base_t::output_location;
    using base_t::delimiter;
    using base_t::col_map;
    bool header_printed = false;
    std::string header = "update";

  public:
    using base_t::emp_is_stats_manager;
    fit_fun_type fit_fun;

    // Constructor for creating this as a stand-alone object
    template <typename WORLD>
    StatsManager_FunctionsOnUpdate(WORLD * w, std::string location = "stats.csv")
     : StatsManager_Base<decltype(w->popM)>(location) {;}

    // Constructor for use by World object
    StatsManager_FunctionsOnUpdate(std::string location = "stats.csv") : base_t(location) {;}

    // The fitness function for calculating fitness related stats.
    // Not called by constructor. Must be called by user.
    template <typename WORLD>
    void Setup(WORLD * w) {
      pop = &(w->popM);
      std::function<void(int)> UpdateFun = [&] (int ud) { Update(ud); };
      w->OnUpdate(UpdateFun);
    }

    // Function for adding functions that calculate stats to the set to be calculated
    void AddFunction(std::function<double()> func, std::string label) {
      stats.Add(func);
      std::string header_label = label;
      remove_whitespace(header_label);
      col_map.push_back(label);
      if (header_printed) NotifyWarning("Function added to stats manager after initialization.");
      header += delimiter + header_label;
    }

    // If this update matches the resolution, calculate and record all the stats
    void Update(int update) {
      if (!header_printed) {
        output_location << header << std::endl;
        header_printed = true;
      }

      if (update % resolution == 0) {
        output_location << update;
        emp::vector<double> results = stats.Run();
        for (double d : results) output_location << delimiter << d;
        output_location << std::endl;
      }
    }

    void SetDefaultFitnessFun(const std::function<double(ORG *)> & fit) {
      fit_fun = fit;
    }

  };

  // Calculate standard information: shannon diversity, max pop fitness, and average pop fitness

  template <typename POP_MANAGER = PopulationManager_Base<int> >
  class StatsManager_DefaultStats : public StatsManager_FunctionsOnUpdate<POP_MANAGER> {
  protected:
    using org_ptr = typename POP_MANAGER::value_type;
    using ORG = typename org_ptr::value_type;
    using fit_fun_type = std::function<double(ORG *)>;
    using fit_stat_type = std::function<double(fit_fun_type, POP_MANAGER*)>;
    using base_t = StatsManager_Base<POP_MANAGER>;
    using base_t::output_location;
    using parent_t = StatsManager_FunctionsOnUpdate<POP_MANAGER>;
    using parent_t::AddFunction;
    using parent_t::pop;
    using parent_t::Update;

  public:
    using parent_t::fit_fun;
    using parent_t::SetDefaultFitnessFun;

    //Constructor for use as a stand-alone object
    template <typename WORLD>
    StatsManager_DefaultStats(WORLD * w, std::string location = "averages.csv")
     : StatsManager_FunctionsOnUpdate<decltype(w->popM)>(w, location) {
      Setup(w);
    }

    //Constructor for use as a template parameter for the world
    StatsManager_DefaultStats(std::string location = "averages.csv")
     : StatsManager_FunctionsOnUpdate<POP_MANAGER>(location) {;}

    //Add appropriate functions to function sets
    template <typename WORLD>
    void Setup(WORLD * w) {
      pop = &(w->popM);

      //Create std::function object for all of the stats
      std::function<double()> diversity = [this]() {
        return ShannonEntropy(*pop);
      };
      std::function<double()> max_fitness = [this]() {
        return MaxFunctionReturn(fit_fun, *pop);
      };
      std::function<double()> avg_fitness = [this]() {
        return AverageFunctionReturn(fit_fun, *pop);
      };

      std::function<void(int)> UpdateFun = [&] (int ud) {
        Update(ud);
      };


      //Add functions to manager
      AddFunction(diversity, "Shannon Diversity");
      AddFunction(max_fitness, "Max Fitness");
      AddFunction(avg_fitness, "Avg Fitness");

      w->OnUpdate(UpdateFun);

    }

  };

  using NullStats = StatsManager_Base<PopBasic>;
  using DefaultStats = StatsManager_DefaultStats<PopBasic>;

  // Calculates Default stats plus some other less frequently used stats: Non-Inferiority,
  // Benefitial/Neutral/Detremental Mutational Landscape average, max benefit/max detremental
  // mutation, and last coalescence depth

  template <typename POP_MANAGER = PopulationManager_Base<int> >
  class StatsManager_AdvancedStats : public StatsManager_FunctionsOnUpdate<POP_MANAGER> {
  protected:
      using org_ptr = typename POP_MANAGER::value_type;
      using ORG = typename org_ptr::value_type;
      using fit_fun_type = std::function<double(ORG *)>;
      using fit_stat_type = std::function<double(fit_fun_type, POP_MANAGER*)>;
      using StatsManager_Base<POP_MANAGER>::output_location;
      using parent_t = StatsManager_FunctionsOnUpdate<POP_MANAGER>;
      using parent_t::AddFunction;
      using parent_t::pop;
      using parent_t::Update;
      using lineage_type = LineageTracker_Pruned<POP_MANAGER>;
      lineage_type * lin_ptr;

  public:
    using parent_t::fit_fun;

    // Constructor for use as a stand alone object
    template<typename WORLD>
    StatsManager_AdvancedStats(WORLD * w, std::string location = "averages.csv")
     : parent_t(w, location) {
         Setup(w);
     }

    // Constructor for use as a template parameter for the world
    StatsManager_AdvancedStats(std::string location = "averages.csv")
     : parent_t(location) {;}

    // Add appropriate functions to function set
    template<typename WORLD>
    void Setup(WORLD * w) {
      pop = &(w->popM);
      lin_ptr = &(w->lineageM);
      MLandscape * data = new MLandscape();

      // Create std::function object for all stats
      std::function<double()> diversity=[this](){ return ShannonEntropy(*pop); };
      std::function<double()> max_fitness=[this](){ return MaxFunctionReturn(fit_fun, *pop); };
      std::function<double()> avg_fitness=[this](){ return AverageFunctionReturn(fit_fun, *pop); };

      std::function<double()> non_inf = [this]() { return NonInf(fit_fun, *pop); };
      std::function<double()> ben_mut = [data, this]() {
        *data = MutLandscape(fit_fun, *pop);
        return data->benefit_avg;
      };
      std::function<double()> neu_mut = [data]() {
        return data->neutral_avg;
      };
      std::function<double()> det_mut = [data]() {
        return data->det_avg;
      };
      std::function<double()> max_ben = [data]() {
        return data->max_ben;
      };
      std::function<double()> max_det = [data]() {
        return data->max_det;
      };
      std::function<double()> last_coal = [this]() {
        int a_id = this->lin_ptr->last_coalesence;
        emp::vector<int> depth = this->lin_ptr->TraceLineageIDs(a_id);
        return (double)depth.size();
      };


      std::function<void(int)> UpdateFun = [&] (int ud) {
        Update(ud);
      };

      // Add functions to mananger
      AddFunction(diversity, "shannon_diversity");
      AddFunction(last_coal, "last_coal");
      AddFunction(max_fitness, "max_fitness");
      AddFunction(avg_fitness, "avg_fitness");
      AddFunction(non_inf, "non_inf");
      AddFunction(ben_mut, "ben_mut");
      AddFunction(neu_mut, "neu_mut");
      AddFunction(det_mut, "det_mut");
      AddFunction(max_ben, "max_ben");
      AddFunction(max_det, "max_det");

      w->OnUpdate(UpdateFun);

    }
  };

}
}

#endif
