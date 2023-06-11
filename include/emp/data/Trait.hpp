/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file Trait.hpp
 *  @brief Directly measure a target quality about a type of object.
 *
 *  These objects are able to measure a specific trait on another object.  They
 *  (will eventually) interface smoothly with DataNodes for data collection.
 */

#ifndef EMP_DATA_TRAIT_HPP_INCLUDE
#define EMP_DATA_TRAIT_HPP_INCLUDE

#include <string>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../math/Range.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  // Pre-declaration of traits.
  template <typename TARGET_T, typename VALUE_T> class Trait;

  template <typename TARGET_T>
  class BaseTrait {
  public:
    virtual ~BaseTrait() { ; }

    virtual std::string EvalString(TARGET_T & target) const = 0;
    virtual double EvalValue(TARGET_T & target) const = 0;
    virtual size_t EvalBin(TARGET_T & target, size_t num_bins) const = 0;

    virtual Ptr<BaseTrait<TARGET_T>> Clone() const = 0;

    template <typename VALUE_T>
    bool IsType() { return (bool) dynamic_cast<Trait<TARGET_T,VALUE_T>>(this); }
  };

  template <typename TARGET_T, typename VALUE_T=double>
  class Trait : public BaseTrait<TARGET_T> {
  public:
    using this_t = Trait<TARGET_T, VALUE_T>;
    using target_t = TARGET_T;
    using value_t = VALUE_T;
    using fun_t = std::function<value_t(target_t &)>;
    using range_t = emp::Range<value_t>;

  protected:
    std::string name;   ///< Unique name for this trait.
    std::string desc;   ///< Full description explaining this trait.
    fun_t fun;          ///< Function to retrieve trait value from target.
    range_t range;      ///< Limits on legal values (defaults to extremes)

  public:
    Trait(const std::string & _n, const fun_t & _f)
      : name(_n), desc(""), fun(_f), range() { ; }
    Trait(const std::string & _n, const fun_t & _f, value_t min, value_t max)
      : name(_n), desc(""), fun(_f), range(min, max) { ; }
    Trait(const std::string & _n, const fun_t & _f, const range_t & _r)
      : name(_n), desc(""), fun(_f), range(_r) { ; }
    Trait(const Trait &) = default;
    Trait(Trait &&) = default;

    Trait & operator=(const Trait &) = default;
    Trait & operator=(Trait &&) = default;

    const std::string & GetName() const { return name; }
    const std::string & GetDesc() const { return desc; }
    const fun_t & GetFun() const { return fun; }
    const range_t & GetRange() const { return range; }

    void SetName(const std::string & _in) { name = _in; }
    void SetDesc(const std::string & _in) { desc = _in; }
    void SetFun(const fun_t & _in) { fun = _in; }
    void SetRange(const range_t & _in) { range = _in; }
    void SetRange(value_t min, value_t max) { range.Set(min, max); }
    void SetMin(value_t min) { range.SetLower(min); }
    void SetMax(value_t max) { range.SetUpper(max); }

    value_t Eval(target_t & target) const { return fun(target); }
    value_t EvalLimit(target_t & target) const { return range.Limit(fun(target)); }
    std::string EvalString(target_t & target) const { return std::to_string(EvalLimit(target)); }
    double EvalValue(target_t & target) const { return (double) EvalLimit(target); }

    // Determine which bin a trait fits in based on the number of bins and the range.
    size_t EvalBin(target_t & target, size_t num_bins) const {
      const value_t val = fun(target);
      return range.CalcBin(val, num_bins);
    }

    Ptr<BaseTrait<TARGET_T>> Clone() const {
      return NewPtr<this_t>(*this);
    };
  };

  /// A TraitSet houses a collection of traits and can trigger them to all be evaluated at once.
  template <typename TARGET_T>
  class TraitSet {
  public:
    using target_t = TARGET_T;
    using trait_t = BaseTrait<target_t>;

  protected:
    emp::vector<emp::Ptr<trait_t>> traits;

  public:
    TraitSet() : traits(0) { ; }
    TraitSet(TraitSet && in) : traits(in.traits) { in.traits.resize(0); }
    TraitSet(const TraitSet & in) : traits(ClonePtrs(in.traits)) { ; }
    ~TraitSet() { Clear(); }

    TraitSet & operator=(TraitSet && in) {
      Clear();
      traits = in.traits;
      in.traits.resize(0);
      return *this;
    }
    TraitSet & operator=(const TraitSet & in) {
      Clear();
      traits = ClonePtrs(in.traits);
      return *this;
    }

    trait_t & operator[](size_t id) { return *(traits[id]); }
    const trait_t & operator[](size_t id) const { return *(traits[id]); }

    size_t GetSize() const { return traits.size(); }

    void Clear() { for (auto & ptr : traits) ptr.Delete(); traits.resize(0); }

    size_t Find(const std::string & name) const {
      for (size_t i = 0; i < traits.size(); i++) {
        if (traits[i]->GetName() == name) return i;
      }
      return (size_t) -1;
    }
    bool Has(const std::string & name) const { return Find(name) < traits.size(); }

    template <typename VALUE_T, typename... EXTRA>
    void AddTrait(const std::string & name,
                  const std::function<VALUE_T(target_t &)> & fun,
                  EXTRA &&... extra) {
      auto ptr = emp::NewPtr< Trait<target_t, VALUE_T> >(name, fun, std::forward<EXTRA>(extra)...);
      traits.push_back(ptr);
    }

    std::string EvalString(size_t id, target_t & target) const { return traits[id]->EvalString(target); }
    double EvalValue(size_t id, target_t & target) const { return traits[id]->EvalValue(target); }

    emp::vector<std::string> EvalStrings(target_t & target) const {
      emp::vector<std::string> results(traits.size());
      for (size_t i = 0; i < traits.size(); i++) results[i] = traits[i]->EvalString(target);
      return results;
    }
    emp::vector<double> EvalValues(target_t & target) const {
      emp::vector<double> results(traits.size());
      for (size_t i = 0; i < traits.size(); i++) results[i] = traits[i]->EvalValue(target);
      return results;
    }

    // Determine which bin a trait fits in based on the number of bins and the range.
    size_t EvalBin(target_t & target, emp::vector<size_t> bin_counts) const {
      size_t mult = 1;
      size_t id = 0;
      for (size_t i = 0; i < traits.size(); i++) {
        id += traits[i]->EvalBin(target, bin_counts[i]) * mult;
        mult *= bin_counts[i];
      }
      return id;
    }

    emp::vector<double> CalcOffsets(target_t & target1, target_t & target2) const {
      emp::vector<double> offsets(traits.size());
      for (size_t i = 0; i < traits.size(); i++) {
        offsets[i] = traits[i]->EvalValue(target2) - traits[i]->EvalValue(target1);
      }
      return offsets;
    }
  };

}

#endif // #ifndef EMP_DATA_TRAIT_HPP_INCLUDE
