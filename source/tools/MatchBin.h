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

#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/IndexMap.h"
#include "../tools/BitSet.h"
#include "../tools/matchbin_utils.h"

namespace emp {

  /// Abstract base class for MatchBin
  template<typename Val, typename Query, typename Tag>
  class BaseMatchBin {

  public:
    using query_t = Query;
    using tag_t = Tag;
    using uid_t = size_t;

    virtual ~BaseMatchBin() {};
    virtual emp::vector<uid_t> Match(const query_t & query, size_t n=1) = 0;
    virtual uid_t Put(const Val & v, const tag_t & t) = 0;
    virtual uid_t Set(const Val & v, const tag_t & t, const uid_t uid) = 0;
    virtual void Delete(const uid_t uid) = 0;
    virtual void Clear() = 0;
    virtual void ClearCache() = 0;
    virtual void SetCacheOn(const bool state = true) = 0;
    virtual Val & GetVal(const uid_t uid) = 0;
    virtual const tag_t & GetTag(const uid_t uid) = 0;
    virtual void SetTag(const uid_t uid, tag_t tag) = 0;
    virtual emp::vector<Val> GetVals(const emp::vector<uid_t> & uids) = 0;
    virtual emp::vector<tag_t> GetTags(const emp::vector<uid_t> & uids) = 0;
    virtual size_t Size() = 0;
    virtual void AdjRegulator(uid_t uid, double amt) = 0;
    virtual void SetRegulator(uid_t uid, double amt) = 0;
    virtual double ViewRegulator(const uid_t uid) const = 0;
    virtual std::string name() const = 0;

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
  template <typename Val, typename Metric, typename Selector>
  class MatchBin
  : public BaseMatchBin<Val, typename Metric::query_t, typename Metric::tag_t> {

  using base_t = BaseMatchBin<
    Val,
    typename Metric::query_t,
    typename Metric::tag_t
  >;

  public:
    using query_t = typename base_t::query_t;
    using tag_t = typename base_t::tag_t;
    using uid_t = typename base_t::uid_t;

  private:
    std::unordered_map<uid_t, Val> values;
    std::unordered_map<uid_t, double> regulators;
    std::unordered_map<uid_t, tag_t> tags;
    emp::vector<uid_t> uids;
    uid_t uid_stepper;

  protected:
    std::unordered_map<query_t, typename Selector::cache_state_type_t> cache;
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
    emp::vector<uid_t> Match(const query_t & query, size_t n=1) override {
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
      for (const auto &[uid, tag] : tags) {
        if (matches.find(tag) == matches.end()) {
          matches[tag] = metric(query, tag);
        }
      }

      // apply regulation to generate match scores
      std::unordered_map<uid_t, double> scores;
      for (auto uid : uids) {
        scores[uid] = matches[tags[uid]] * regulators[uid] + regulators[uid];
      }

      typename Selector::cache_state_type_t cacheResult = selector(uids, scores, n);
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

    /// Put an item and associated tag in the container. Returns the uid for
    /// that entry.
    uid_t Put(const Val & v, const tag_t & t) override {
      const uid_t orig = uid_stepper;
      while(values.find(++uid_stepper) != values.end()) {
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
      emp_assert(values.find(uid) == values.end());

      ClearCache();

      values[uid_stepper] = v;
      regulators[uid_stepper] = 1.0;
      tags[uid_stepper] = t;
      uids.push_back(uid_stepper);
      return uid_stepper;
    }


    /// Delete an item and its associated tag.
    void Delete(const uid_t uid) override {
      ClearCache();
      values.erase(uid);
      regulators.erase(uid);
      tags.erase(uid);
      uids.erase(std::remove(uids.begin(), uids.end(), uid), uids.end());
    }

    /// Clear all items and tags.
    void Clear() override {
      ClearCache();
      values.clear();
      regulators.clear();
      tags.clear();
      uids.clear();
    }

    void ClearCache() override {
      if constexpr(cacheEnabled){
        cache.clear();
      }
    }

    void SetCacheOn(const bool state = true) override {
      emp_assert(cacheEnabled);
      cacheOn = state;
      ClearCache();
    }

    /// Access a reference single stored value by uid.
    Val & GetVal(const uid_t uid) override {
      emp_assert(values.find(uid) != values.end());
      return values.at(uid);
    }

    /// Access a const reference to a single stored tag by uid.
    const tag_t & GetTag(const uid_t uid) override {
      emp_assert(tags.find(uid) != tags.end());
      return tags.at(uid);
    }

    /// Change the tag at a given uid and clear the cache.
    void SetTag(const uid_t uid, tag_t tag) override {
      emp_assert(tags.find(uid) != tags.end());
      ClearCache();
      tags.at(uid) = tag;
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
    size_t Size() override {
      return values.size();
    }

    /// Add an amount to an item's regulator value. Positive amounts
    /// downregulate the item and negative amounts upregulate it.
    void AdjRegulator(const uid_t uid, const double amt) override {
      emp_assert(regulators.find(uid) != regulators.end());

      const double prev = regulators.at(uid);
      const double nxt = std::max(0.0, prev + amt);

      if (prev != nxt) ClearCache();

      regulators.at(uid) = nxt;

    }

    /// Set an item's regulator value. Provided value must be greater than or
    /// equal to zero. A value between zero and one upregulates the item, a
    /// value of exactly one is neutral, and a value greater than one
    /// upregulates the item.
    void SetRegulator(const uid_t uid, const double amt) override {
      emp_assert(regulators.find(uid) != regulators.end());
      emp_assert(amt >= 0.0);

      if (regulators.at(uid) != amt) ClearCache();

      regulators.at(uid) = amt;
    }

    /// View an item's regulator value.
    double ViewRegulator(const uid_t uid) const override {
      emp_assert(regulators.find(uid) != regulators.end());

      return regulators.at(uid);
    }

    std::string name() const override {
      return emp::to_string(
        "Selector: ",
        selector.name(),
        " / ",
        "Metric: ",
        metric.name()
      );
    }

  };

}

#endif
