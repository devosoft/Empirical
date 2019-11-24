/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file MatchBin.h
 *  @brief A container that supports flexible tag-based lookup. .
 *
 */


#ifndef EMP_MATCH_BIN_H
#define EMP_MATCH_BIN_H

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <ratio>
#include <math.h>

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/IndexMap.h"
#include "../tools/BitSet.h"
#include "../tools/matchbin_utils.h"

namespace emp {

  /// Internal state packet for MatchBin
  template<typename Val, typename Tag, typename Regulator>
  struct MatchBinState {

    using tag_t = Tag;
    using uid_t = size_t;

    std::unordered_map<uid_t, Val> values;
    std::unordered_map<uid_t, Regulator> regulators;
    std::unordered_map<uid_t, tag_t> tags;
    emp::vector<uid_t> uids;

    template <class Archive>
    void serialize( Archive & ar )
    {
      ar(
        CEREAL_NVP(values),
        CEREAL_NVP(regulators),
        CEREAL_NVP(tags),
        CEREAL_NVP(uids)
      );
    }

  };

  /// Abstract base class for MatchBin
  template<typename Val, typename Query, typename Tag, typename Regulator>
  class BaseMatchBin {

  public:
    using query_t = Query;
    using tag_t = Tag;
    using uid_t = size_t;

    virtual ~BaseMatchBin() {};
    virtual emp::vector<uid_t> Match(const query_t & query, size_t n=0) = 0;
    virtual emp::vector<uid_t> MatchRaw(const query_t & query, size_t n=0) = 0;
    virtual uid_t Put(const Val & v, const tag_t & t) = 0;
    virtual uid_t Set(const Val & v, const tag_t & t, const uid_t uid) = 0;
    virtual void Delete(const uid_t uid) = 0;
    virtual void Clear() = 0;
    virtual void ClearCache() = 0;
    virtual void SetCacheOn(const bool state = true) = 0;
    virtual Val & GetVal(const uid_t uid) = 0;
    virtual const tag_t & GetTag(const uid_t uid) const = 0;
    virtual void SetTag(const uid_t uid, tag_t tag) = 0;
    virtual emp::vector<Val> GetVals(const emp::vector<uid_t> & uids) = 0;
    virtual emp::vector<tag_t> GetTags(const emp::vector<uid_t> & uids) = 0;
    virtual size_t Size() const = 0;
    virtual void AdjRegulator(
      uid_t uid,
      const typename Regulator::adj_t & amt
    ) = 0;
    virtual void SetRegulator(
      uid_t uid,
      const typename Regulator::set_t & set
    ) = 0;
    virtual void SetRegulator(
      uid_t uid,
      const Regulator & set
    ) = 0;
    virtual Regulator & GetRegulator(const uid_t uid) = 0;
    virtual const typename Regulator::view_t ViewRegulator(
      const uid_t uid
    ) const = 0;
    virtual void ImprintRegulators(const BaseMatchBin & target) = 0;
    virtual std::string name() const = 0;
    virtual const emp::vector<uid_t>& ViewUIDs() const = 0;
  };


  /// A data container that allows lookup by tag similarity. It can be templated
  /// on different tag types and is configurable on construction for (1) the
  /// distance metric used to compute similarity between tags and (2) the
  /// selector that is used to select which matches to return. Regulation
  /// functionality is also provided, allowing dynamically adjustment of match
  /// strength to a particular item (i.e., making all matches stronger/weaker).
  /// A unique identifier is generated upon tag/item placement in the container.
  /// This unique identifier can be used to view or edit the stored items and
  /// their corresponding tags. Tag-based lookups return a list of matched
  /// unique identifiers.
  template <
    typename Val,
    typename Metric,
    typename Selector,
    typename Regulator=LinearRegulator
  >
  class MatchBin
  : public BaseMatchBin<
    Val,
    typename Metric::query_t,
    typename Metric::tag_t,
    Regulator
  > {

  using base_t = BaseMatchBin<
    Val,
    typename Metric::query_t,
    typename Metric::tag_t,
    Regulator
  >;

  public:
    using query_t = typename base_t::query_t;
    using tag_t = typename base_t::tag_t;
    using uid_t = typename base_t::uid_t;
    using state_t = MatchBinState<Val, tag_t, Regulator>;

  protected:
    state_t state;

    uid_t uid_stepper;

    std::unordered_map<query_t, typename Selector::cache_state_type_t> cache;
    std::unordered_map<
      query_t,
      typename Selector::cache_state_type_t
    > cache_raw;
    static constexpr bool cacheEnabled = std::is_base_of<CacheStateBase, typename Selector::cache_state_type_t>::value;
    bool cacheOn = cacheEnabled;

  public:
    Metric metric;
    Selector selector;

    MatchBin() : uid_stepper(0) { ; }

    MatchBin(emp::Random & rand)
    : uid_stepper(0)
    , selector(rand)
    { ; }

    /// Compare a query tag to all stored tags using the distance metric
    /// function and return a vector of unique IDs chosen by the selector
    /// function.
    /// Calling with n = 0 means delegate choice for how many values to return
    /// to the Selector.
    emp::vector<uid_t> Match(const query_t & query, size_t n=0) override {
      if constexpr(cacheEnabled){
        if (cacheOn){
          if (cache.find(query) != cache.end()){
            std::optional<emp::vector<uid_t>> res = cache[query](n);
            if (res != std::nullopt){ return res.value(); }
          }
        }
      }
      // compute distance between query and all stored tags
      std::unordered_map<tag_t, double> matches;
      for (const auto &[uid, tag] : state.tags) {
        if (matches.find(tag) == matches.end()) {
          matches[tag] = metric(query, tag);
        }
      }

      // apply regulation to generate match scores
      std::unordered_map<uid_t, double> scores;
      for (auto uid : state.uids) {
        scores[uid] = state.regulators[uid](matches[state.tags[uid]]);
      }

      typename Selector::cache_state_type_t cacheResult = selector(state.uids, scores, n);
      if constexpr(cacheEnabled){
        if (cacheOn){
          cache[query] = cacheResult;
        }
        return cacheResult(n).value();
      }
      else{
        return cacheResult(n).value();
      }
    }

    /// Compare a query tag to all stored tags using the distance metric
    /// function and return a vector of unique IDs chosen by the selector
    /// function. Ignore regulators.
    emp::vector<uid_t> MatchRaw(const query_t & query, size_t n=1) override {
      if constexpr(cacheEnabled){
        if (cacheOn){
          if (cache_raw.find(query) != cache_raw.end()){
            std::optional<emp::vector<uid_t>> res = cache_raw[query](n);
            if (res != std::nullopt){ return res.value(); }
          }
        }
      }
      // compute distance between query and all stored tags
      std::unordered_map<tag_t, double> matches;
      for (const auto &[uid, tag] : state.tags) {
        if (matches.find(tag) == matches.end()) {
          matches[tag] = metric(query, tag);
        }
      }

      // apply regulation to generate match scores
      std::unordered_map<uid_t, double> scores;
      for (auto uid : state.uids) {
        scores[uid] = matches[state.tags[uid]];
      }

      typename Selector::cache_state_type_t cacheResult = selector(state.uids, scores, n);
      if constexpr(cacheEnabled){
        if (cacheOn){
          cache_raw[query] = cacheResult;
        }
        return cacheResult(n).value();
      }
      else{
        return cacheResult(n).value();
      }
    }

    /// Put an item and associated tag in the container. Returns the uid for
    /// that entry.
    uid_t Put(const Val & v, const tag_t & t) override {
      const uid_t orig = uid_stepper;
      while(state.values.find(++uid_stepper) != state.values.end()) {
        // if container is full
        // i.e., wrapped around because all uids already allocated
        if (uid_stepper == orig) throw std::runtime_error("container full");
      }
      return Set(v, t, uid_stepper);
    }

    /// Put with a manually-chosen UID.
    /// (Caller is responsible for ensuring UID is unique
    /// or calling Delete beforehand.)
    uid_t Set(const Val & v, const tag_t & t, const uid_t uid) override {
      emp_assert(state.values.find(uid) == state.values.end());

      ClearCache();

      state.values[uid] = v;
      state.regulators.insert({{uid},{}});
      state.tags[uid] = t;
      state.uids.push_back(uid);
      return uid;
    }


    /// Delete an item and its associated tag.
    void Delete(const uid_t uid) override {
      ClearCache();
      state.values.erase(uid);
      state.regulators.erase(uid);
      state.tags.erase(uid);
      state.uids.erase(std::remove(state.uids.begin(), state.uids.end(), uid), state.uids.end());
    }

    /// Clear all items and tags.
    void Clear() override {
      ClearCache();
      state.values.clear();
      state.regulators.clear();
      state.tags.clear();
      state.uids.clear();
    }

    void ClearCache() override {
      if constexpr(cacheEnabled){
        cache.clear();
        cache_raw.clear();
      }
    }

    void SetCacheOn(const bool state = true) override {
      emp_assert(cacheEnabled);
      cacheOn = state;
      ClearCache();
    }

    /// Access a reference single stored value by uid.
    Val & GetVal(const uid_t uid) override {
      emp_assert(state.values.find(uid) != state.values.end());
      return state.values.at(uid);
    }

    /// Access a const reference to a single stored tag by uid.
    const tag_t & GetTag(const uid_t uid) const override {
      emp_assert(state.tags.find(uid) != state.tags.end());
      return state.tags.at(uid);
    }

    /// Change the tag at a given uid and clear the cache.
    void SetTag(const uid_t uid, tag_t tag) override {
      emp_assert(state.tags.find(uid) != state.tags.end());
      ClearCache();
      state.tags.at(uid) = tag;
    }

    /// Generate a vector of values corresponding to a vector of uids.
    emp::vector<Val> GetVals(const emp::vector<uid_t> & uids) override {
      emp::vector<Val> res;
      std::transform(
        uids.begin(),
        uids.end(),
        std::back_inserter(res),
        [this](uid_t uid) -> Val { return GetVal(uid); }
      );
      return res;
    }

    /// Generate a vector of tags corresponding to a vector of uids.
    emp::vector<tag_t> GetTags(const emp::vector<uid_t> & uids) override {
      emp::vector<tag_t> res;
      std::transform(
        uids.begin(),
        uids.end(),
        std::back_inserter(res),
        [this](uid_t uid) -> tag_t { return GetTag(uid); }
      );
      return res;
    }

    /// Get the number of items stored in the container.
    size_t Size() const override {
      return state.values.size();
    }

    /// Adjust an item's regulator value.
    void AdjRegulator(
      const uid_t uid,
      const typename Regulator::adj_t & amt
    ) override {
      emp_assert(state.regulators.find(uid) != state.regulators.end());

      if (state.regulators.at(uid).Adj(amt)) ClearCache();

    }

    /// Set an item's regulator value.
    void SetRegulator(
      const uid_t uid,
      const typename Regulator::set_t & set
    ) override {
      emp_assert(state.regulators.find(uid) != state.regulators.end());

      if (state.regulators.at(uid).Set(set)) ClearCache();

    }

    /// Set an item's regulator value.
    void SetRegulator(
      const uid_t uid,
      const Regulator & set
    ) override {
      emp_assert(state.regulators.find(uid) != state.regulators.end());

      if (
        set != std::exchange(state.regulators.at(uid), set)
      ) ClearCache();

    }

    /// View an item's regulator value.
    const typename Regulator::view_t ViewRegulator(
      const uid_t uid
    ) const override {
      emp_assert(state.regulators.find(uid) != state.regulators.end());

      return state.regulators.at(uid).View();
    }

    /// Get a regulator.
    Regulator & GetRegulator(const uid_t uid) override {
      emp_assert(state.regulators.find(uid) != state.regulators.end());

      return state.regulators.at(uid);
    }

    /// View the UIDs currently associated with the MatchBin.
    void ImprintRegulators(
      const BaseMatchBin<Val, query_t, tag_t, Regulator> & target
    ) override {

      for (uid_t uid : state.uids) {

        std::unordered_map<uid_t, double> scores;
        std::transform(
          std::cbegin(target.ViewUIDs()),
          std::cend(target.ViewUIDs()),
          std::inserter(scores, std::begin(scores)),
          [&](size_t target_uid){
            return std::pair{
              target_uid,
              metric(target.GetTag(target_uid), GetTag(uid))
            };
          }
        );
        SetRegulator(
          uid,
          target.ViewRegulator(
            std::min_element(
              std::cbegin(scores),
              std::cend(scores),
              [](const auto& l, const auto& r ){ return l.second < r.second; }
            )->first
          )
        );

      }

    }

    /// View UIDs associated with this MatchBin
    const emp::vector<uid_t>& ViewUIDs() const override {
      return state.uids;
    }

    /// Get selector, metric name
    std::string name() const override {
      Regulator reg{};
      return emp::to_string(
        "Selector: ",
        selector.name(),
        " / ",
        "Metric: ",
        metric.name(),
        " / ",
        "Regulator: ",
        reg.name()
      );
    }

    /// Extract MatchBin state
    const state_t & GetState() const { return state; }

    /// Load MatchBin state
    void SetState(const state_t & state_) {
      state = state_;
      ClearCache();
    }

  };

}

#endif
