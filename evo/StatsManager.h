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


  //Base stats manager - this mostly exists to be extended into custom
  //stats managers (see the OEEStatsManager for an example). The base
  //stats manager also handles data output.
  class StatsManager_Base {
  public:
    std::string delimiter = " "; //Gets inferred from file name
    int resolution = 10; //With what frequency do we record data?
    static constexpr bool emp_is_stats_manager = true;
    std::ofstream output_location; //Where does output go?

    StatsManager_Base(std::string location = "cout"){
        SetOutput(location);
    }
    ~StatsManager_Base(){
        output_location.close();
    }

    //Tells the stats manager where to put output. If location is "cout"
    //(default) or "stdout", stats will get sent to cout. Otherwise, the
    //specified file will be used as the location for output. If the file
    //has the extension "csv" or "tsv", the appropriate delimiter will be used.
    //If the location is invalid, the program will exit with an error.
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

  //A popular type of stats manager is one that prints a set of statistics every
  //so many updates. This is a generic stats manager of that variety, which
  //maintains FunctionSets containing all of the functions to be run.
  //Although functions can be added to this manager on the fly, the goal of
  //this class is that it can be extended to track specific sets of functions.
  //(see StatsManager_DefaultStats for an example)
  template <typename ORG, typename... MANAGERS>
  class StatsManager_FunctionsOnUpdate : StatsManager_Base {
  protected:
    using world_type = World<ORG, MANAGERS...>;
    using fit_fun_type = std::function<double(ORG*)>;
    //Stats calculated on the world
    FunctionSet<double, world_type* > world_stats;
    //Stats calculated on the world that require a fitness function
    FunctionSet<double, std::function<double(ORG * org)>,
                                           world_type* > fitness_stats;
    //Pointer to the world object on which we're calculating stats
    World<ORG, MANAGERS...> * world;
    using StatsManager_Base::resolution;
    using StatsManager_Base::output_location;

  public:

    StatsManager_FunctionsOnUpdate(world_type * w,
                                   std::string location) :
                                   StatsManager_Base(location){
      std::function<void(int)> UpdateFun = [this] (int ud){
        Update(ud);
      };
      world = w;
      w->OnUpdate(UpdateFun); //See if we need to calculate stats every update
    }

    //The fitness function for calculating fitness related stats
    fit_fun_type fit_fun;

    //Function for adding functions that calculate stats to the
    //set to be calculated
    void AddFunction(std::function<double(world_type*)> func) {
      world_stats.Add(func);
    }

    //Version for functions that require a fitness function
    void AddFunction(std::function<double(fit_fun_type, world_type*)> func) {
      fitness_stats.Add(func);
    }

    //If this update matches the resolution, calculate and record all the stats
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

  //Calculates some commonly required information: shannon diversity,
  //max fitness within the population, and average fitness within the population
  template <typename ORG, typename... MANAGERS>
  class StatsManager_DefaultStats : StatsManager_FunctionsOnUpdate<ORG, MANAGERS...> {
  private:
      using world_type = World<ORG, MANAGERS...>;
      using fit_fun_type = std::function<double(ORG*)>;
      using fit_stat_type = std::function<double(fit_fun_type, world_type*)>;
      using StatsManager_FunctionsOnUpdate<ORG, MANAGERS...>::AddFunction;
      using StatsManager_Base::output_location;

  public:
      using StatsManager_FunctionsOnUpdate<ORG, MANAGERS...>::fit_fun;

      StatsManager_DefaultStats(world_type * w, std::string location = "averages.csv")
       : StatsManager_FunctionsOnUpdate<ORG, MANAGERS...>(w, location){
        std::function<double(world_type*)> diversity = [](world_type * world){
            return ShannonDiversity(*world);
        };
        fit_stat_type max_fitness = [](fit_fun_type fit_func, world_type * world){
            return MaxFitness(fit_func, *world);
        };
        fit_stat_type avg_fitness = [](fit_fun_type fit_func, world_type * world){
            return AverageFitness(fit_func, *world);
        };

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
