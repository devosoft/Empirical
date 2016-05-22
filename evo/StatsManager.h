#ifndef EMP_EVO_STATS_MANAGER_H
#define EMP_EVO_STATS_MANAGER_H

#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../tools/FunctionSet.h"
#include "../tools/vector.h"
#include "World.h"

namespace emp{
namespace evo{


  class StatsManager_Base {
  public:
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
        }
    }

  };

  template <typename ORG, typename... MANAGERS>
  class StatsManager_FunctionsOnUpdate : StatsManager_Base {
  private:
    FunctionSet<double, World<ORG, MANAGERS...>* > world_stats;
    FunctionSet<double, std::function<double(ORG * org)>, World<ORG, MANAGERS...>* > fitness_stats;
    World<ORG, MANAGERS...> * world;
    using StatsManager_Base::resolution;
    using StatsManager_Base::output_location;

  public:

    StatsManager_FunctionsOnUpdate(World<ORG, MANAGERS...> * w,
        std::string location = "averages.csv") : StatsManager_Base(location){
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
        emp::vector<double> world_results = world_stats.Run(world);
        for (double d : world_results){
          output_location << d << " ";
        }

        emp::vector<double> fitness_results = fitness_stats.Run(fit_fun, world);
        for (double d : fitness_results){
            output_location << d << " ";
        }
        output_location << std::endl;
      }
    }

  };

}
}

#endif
