#ifndef EMP_EVO_STATS_MANAGER_H
#define EMP_EVO_STATS_MANAGER_H

#include <functional>

#include "../tools/FunctionSet.h"
#include "../tools/vector.h"
#include "World.h"

namespace emp{
namespace evo{


  class StatsManager_Base {
  public:
    int resolution = 10; //With what frequency do we record data?
    static constexpr bool emp_is_stats_manager = true;
    StatsManager_Base(){;}
    ~StatsManager_Base(){;}
  };

  template <typename ORG, typename... MANAGERS>
  class StatsManager_FunctionsOnUpdate : StatsManager_Base {
  private:
    FunctionSet<double, World<ORG, MANAGERS...>* > world_stats;
    FunctionSet<double, std::function<double(ORG * org)>, World<ORG, MANAGERS...>* > fitness_stats;

    World<ORG, MANAGERS...> * world;
    using StatsManager_Base::resolution;
  public:
    StatsManager_FunctionsOnUpdate(World<ORG, MANAGERS...> * w){
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
          std::cout << d << " ";
        }

        emp::vector<double> fitness_results = fitness_stats.Run(fit_fun, world);
        for (double d : fitness_results){
          std::cout << d << " ";
        }
        std::cout << std::endl;
      }
    }

  };

}
}

#endif
