//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in mechanisms for the organisms to interact with the world.
//  (I.e., how is fitness determined?  How are mutations performed?)

#ifndef EMP_EVO_ORGANISM_MANAGER_H
#define EMP_EVO_ORGANISM_MANAGER_H

#include <unordered_map>

#include "../meta/meta.h"

#define EMP_SETUP_EVO_DEFAULT(FUN_VAR, NAME, TEST, ACTION, RTYPE)              \
  std::function<RTYPE(ORG*)> FUN_VAR;                                          \
  template <class T> void Setup_ ## FUN_VAR ## _impl(emp_bool_decoy(TEST)) {   \
    FUN_VAR = [](T* org){ return ACTION; };                                    \
  }                                                                            \
  template <class T> void Setup_ ## FUN_VAR ## _impl(int) { ; }                \
  void Setup_ ## NAME() {                                                      \
    Setup_ ## FUN_VAR ## _impl<ORG>(true);                                     \
  }                                                                            \
  public:                                                                      \
  const std::function<RTYPE(ORG*)> & GetDefault ## NAME ## Fun() const {       \
    return FUN_VAR;                                                            \
  }                                                                            \
  void SetDefault ## NAME ## Fun(const std::function<RTYPE(ORG*)> & f) {       \
    FUN_VAR = f;                                                               \
  }                                                                            \
  protected:


#define EMP_SETUP_EVO_DEFAULT_ARGS(FUN_VAR, NAME, TEST, ACTION, RTYPE, ...)         \
  std::function<RTYPE(ORG*, __VA_ARGS__)> FUN_VAR;                                  \
  template <class T, typename... ARG_TYPES>                                         \
  void Setup_ ## FUN_VAR ## _impl(emp_bool_decoy(TEST)) {                           \
    FUN_VAR = [](T* org, ARG_TYPES... args){ return ACTION; };                      \
  }                                                                                 \
  template <class T, typename... ARG_TYPES>                                         \
  void Setup_ ## FUN_VAR ## _impl(int) { ; }                                        \
  void Setup_ ## NAME() {                                                           \
    Setup_ ## FUN_VAR ## _impl<ORG, __VA_ARGS__>(true);                             \
  }                                                                                 \
  public:                                                                           \
  const std::function<RTYPE(ORG*,__VA_ARGS__)>& GetDefault ## NAME ## Fun() const { \
    return FUN_VAR;                                                                 \
  }                                                                                 \
  void SetDefault ## NAME ## Fun(const std::function<RTYPE(ORG*,__VA_ARGS__)>& f) { \
    FUN_VAR = f;                                                                    \
  }                                                                                 \
  protected:

namespace emp {
namespace evo {

  namespace internal {
    struct DummyOrg {
      double Fitness() { emp_assert(false); return 0.0; }
      bool Mutate() { emp_assert(false); return false; }
    };

    template <typename GENOME>
    struct FitnessCache {
      std::unordered_map<GENOME, double> fit_map;
    };

//    template <typename ORG> auto org_to_genome_t(emp_int_decoy(ORG::genome)) -> decltype(ORG::genome);
  }

  template <typename ORG=int>
  class OrgManager_Base {
  protected:
    using ptr_t = ORG *;

  public:
    OrgManager_Base() { ; }
    ~OrgManager_Base() { ; }

    // Allow this and derived classes to be identified as an organism manager.
    static constexpr bool emp_is_organism_manager = true;

    double Fitness(ORG * org) { return (double) *org; }
    bool Mutate(ORG *, emp::Random&) { return false; }

    std::function<double(ORG*)> GetFitFun() { return [](ORG* org){ return (double) *org; }; }
    std::function<bool(ORG*,Random&)> GetMutFun() { return [](ORG*,Random&){ return false; }; }
  };

  template <typename ORG=internal::DummyOrg>
  class OrgManager_Direct : public OrgManager_Base<ORG> {
  protected:
  public:
    OrgManager_Direct() { ; }
    ~OrgManager_Direct() { ; }

    double Fitness(ORG * org) const { return org->Fitness(); }
    bool Mutate(ORG * org, emp::Random & r) { return org->Mutate(r); }

    std::function<double(ORG*)> GetFitFun() { return [](ORG* org){ return org->Fitness(); }; }
    std::function<bool(ORG * org, Random & r)> GetMutFun() {
      return [](ORG* org,Random& r){ return org->Mutate(r); };
    }
  };

  template <typename ORG=int>
  class OrgManager_Dynamic : public OrgManager_Base<ORG> {
  protected:
    EMP_SETUP_EVO_DEFAULT(default_fit_fun, Fitness, &T::Fitness, org->Fitness(), double)
    EMP_SETUP_EVO_DEFAULT_ARGS(default_mut_fun, Mutate, &T::Mutate, org->Mutate(args...), bool, emp::Random &)

  public:
    OrgManager_Dynamic() {
      Setup_Fitness();
      Setup_Mutate();
    }
    ~OrgManager_Dynamic() { ; }

    const std::function<double(ORG*)> & GetFitFun() { return default_fit_fun; }
    const std::function<bool(ORG * org, Random & r)> & GetMutFun() { return default_mut_fun; }
  };

  using OrgMBasic  = OrgManager_Base<>;
  using OrgMDynamic  = OrgManager_Dynamic<>;
  using OrgMDirect = OrgManager_Direct<>;

}
}

#endif
