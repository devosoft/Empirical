#ifndef EMP_EVO_STATS_MANAGER_H
#define EMP_EVO_STATS_MANAGER_H

#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../tools/FunctionSet.h"
#include "../tools/vector.h"
#include "World.h"
#include "Stats.h"

namespace emp{
namespace evo{


  class StatsManager_Base {
  public:
    std::string delimiter = " ";
    int resolution = 10; //With what frequency do we record data?
    static constexpr bool emp_is_stats_manager = true;
    std::ofstream output_location;
    StatsManager_Base(std::string location = "cout"){
        SetOutput(location);
    }
    ~StatsManager_Base(){;}

    void SetOutput(std::string location){
        if (location == "cout" || location == "stdout"){
            output_location.copyfmt(std::cout);
            output_location.clear(std::cout.rdstate());
            output_location.basic_ios<char>::rdbuf(std::cout.rdbuf());
        } else {
            output_location.open(location);
            if (!output_location.good()){
                std::cout << "Invalid output file. Exiting." << std::endl;
                exit(0);
            }
            string_pop(location, ".");
            if (location == "csv") {
                delimiter = ", ";
            } else if (location == "tsv") {
                delimiter = "\t";
            }
        }
    }

  };

  template <typename ORG, typename... MANAGERS>
  class StatsManager_FunctionsOnUpdate : StatsManager_Base {
  protected:
    FunctionSet<double, World<ORG, MANAGERS...>* > world_stats;
    FunctionSet<double, std::function<double(ORG * org)>, World<ORG, MANAGERS...>* > fitness_stats;
    World<ORG, MANAGERS...> * world;
    using StatsManager_Base::resolution;
    using StatsManager_Base::output_location;

  public:

      StatsManager_FunctionsOnUpdate(World<ORG, MANAGERS...> * w,
          std::string location) : StatsManager_Base(location){
        std::function<void(int)> UpdateFun = [this] (int ud){
          Update(ud);
        };
        world = w;
        w->OnUpdate(UpdateFun);
      }

    std::function<double(ORG * org)> fit_fun;

    void AddFunction(std::function<double(World<ORG, MANAGERS...>*)> func) {
      world_stats.Add(func);
    }

    void AddFunction(std::function<double(std::function<double(ORG * org)>, World<ORG, MANAGERS...>*)> func) {
      fitness_stats.Add(func);
    }

    void Update(int update) {

      if (update % resolution == 0){

        output_location << update;

        emp::vector<double> world_results = world_stats.Run(world);
        for (double d : world_results) {
          output_location << delimiter << d;
        }

        emp::vector<double> fitness_results = fitness_stats.Run(fit_fun, world);
        for (double d : fitness_results){
            output_location << delimiter << d;
        }
        output_location << std::endl;
      }
    }

  };

  template <typename ORG, typename... MANAGERS>
  class StatsManager_DefaultStats : StatsManager_FunctionsOnUpdate<ORG, MANAGERS...> {
  private:
      using world_type = World<ORG, MANAGERS...>;
      using StatsManager_FunctionsOnUpdate<ORG, MANAGERS...>::AddFunction;
      using StatsManager_Base::output_location;
  public:
      using StatsManager_FunctionsOnUpdate<ORG, MANAGERS...>::fit_fun;

      StatsManager_DefaultStats(world_type * w, std::string location = "averages.csv")
       : StatsManager_FunctionsOnUpdate<ORG, MANAGERS...>(w, location){
        std::function<double(world_type*)> diversity = [](world_type * world){return ShannonDiversity(*world);};
        std::function<double(std::function<double(ORG * org)>, world_type*)> max_fitness = [](std::function<double(ORG * org)> fit_func, world_type * world){return MaxFitness(fit_func, *world);};
        std::function<double(std::function<double(ORG * org)>, world_type*)> avg_fitness = [](std::function<double(ORG * org)> fit_func, world_type * world){return AverageFitness(fit_func, *world);};
        AddFunction(diversity);
        AddFunction(max_fitness);
        AddFunction(avg_fitness);

        //Print header
        output_location << "update, shannon_diversity, max_fitness, avg_fitness" << std::endl;
      }

};

}
}

#endif
