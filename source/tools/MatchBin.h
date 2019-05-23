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

#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/IndexMap.h"

namespace emp {

  using selector_t = std::function<
    emp::vector<size_t>
    (emp::vector<size_t>&, std::unordered_map<size_t, double>&, size_t)
  >;

  /// Factory function that generates an exact match-order selector function
  /// configured to filter out matches below a minimum-match threshold.
  selector_t ThreshSelector(double thresh){

    return [thresh](
        emp::vector<size_t>& uids,
        std::unordered_map<size_t, double>& scores,
        size_t n
      ){

        std::sort(
          uids.begin(),
          uids.end(),
          [&scores](size_t const &a, size_t const &b) {
            return scores.at(a) < scores.at(b);
          }
        );

        size_t i = 0;
        while(i < uids.size() && i < n && scores.at(uids[i]) < thresh) ++i;

        return emp::vector<size_t>(uids.begin(), uids.begin()+i);

      };
  }

  /// Factory function that generates a selector function that chooses
  /// probabilisticly based on match quality with replacement.
  selector_t RouletteSelector(
    emp::Random &random,
    const double skew=0.1
  ){

    emp_assert(skew > 0.0);

    return [&random, skew](
        emp::vector<size_t>& uids,
        std::unordered_map<size_t, double>& scores,
        size_t n
      ){

        IndexMap match_index(uids.size());
        for (size_t i = 0; i < uids.size(); ++i) {
          emp_assert(scores[uids[i]] >= 0);
          match_index.Adjust(i, 1.0 / ( skew + scores[uids[i]] ));
        }

        emp::vector<size_t> res;
        res.reserve(n);

        for (size_t j = 0; j < n; ++j) {
          const double match_pos = random.GetDouble(match_index.GetWeight());
          const size_t idx = match_index.Index(match_pos);
          res.push_back(uids[idx]);
        }

        return res;

      };
  }

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
  template <typename Val, typename Tag> class MatchBin {
  private:

    const std::function<double(Tag, Tag)> metric;
    const selector_t selector;

    std::unordered_map<size_t, Val> values;
    std::unordered_map<size_t, double> regulators;
    std::unordered_map<size_t, Tag> tags;
    emp::vector<size_t> uids;

    size_t uid_stepper;

  public:

    MatchBin (
      const std::function<double(Tag, Tag)> metric_,
      const selector_t selector_ = ThreshSelector(0.0)
    ) : metric(metric_)
    , selector(selector_)
    , uid_stepper(0) { ; }

    /// Compare a query tag to all stored tags using the distance metric
    /// function and return a vector of unique IDs chosen by the selector
    /// function.
    emp::vector<size_t> Match(const Tag & query, size_t n=1) {

      // compute distance between query and all stored tags
      std::unordered_map<Tag, double> matches;
      for (const auto &[uid, tag] : tags) {
        if (matches.find(tag) == matches.end()) {
          matches[tag] = metric(query, tag);
        }
      }

      // apply regulation to generate match scores
      std::unordered_map<size_t, double> scores;
      for (auto uid : uids) {
        scores[uid] = matches[tags[uid]] * regulators[uid] + regulators[uid];
      }

      return selector(uids, scores, n);
    }

    /// Put an item and associated tag in the container. Returns the uid for
    /// that entry.
    size_t Put(const Val & v, const Tag & t) {

      const size_t orig = uid_stepper;
      while(values.find(++uid_stepper) != values.end()) {
        // if container is full
        // i.e., wrapped around because all uids already allocated
        if (uid_stepper == orig) throw std::runtime_error("container full");
      }

      values[uid_stepper] = v;
      regulators[uid_stepper] = 1.0;
      tags[uid_stepper] = t;
      uids.push_back(uid_stepper);
      return uid_stepper;
    }


    /// Delete an item and its associated tag.
    void Delete(size_t uid) {
      values.erase(uid);
      regulators.erase(uid);
      tags.erase(uid);
      uids.erase(std::remove(uids.begin(), uids.end(), uid), uids.end());
    }

    /// Clear all items and tags.
    void Clear() {
      values.clear();
      regulators.clear();
      tags.clear();
      uids.clear();
    }

    /// Access a reference single stored value by uid.
    Val & GetVal(const size_t uid) {
      return values.at(uid);
    }

    /// Access a reference to a single stored tag by uid.
    Tag & GetTag(const size_t uid) {
      return tags.at(uid);
    }

    /// Generate a vector of values corresponding to a vector of uids.
    emp::vector<Val> GetVals(const emp::vector<size_t> & uids) {
      emp::vector<Val> res;
      std::transform(
        uids.begin(),
        uids.end(),
        std::back_inserter(res),
        [this](size_t uid) -> Val { return GetVal(uid); }
      );
      return res;
    }

    /// Generate a vector of tags corresponding to a vector of uids.
    emp::vector<Tag> GetTags(const emp::vector<size_t> & uids) {
      emp::vector<Tag> res;
      std::transform(
        uids.begin(),
        uids.end(),
        std::back_inserter(res),
        [this](size_t uid) -> Tag { return GetTag(uid); }
      );
      return res;
    }

    /// Get the number of items stored in the container.
    size_t Size() {
      return values.size();
    }

    /// Add an amount to an item's regulator value. Positive amounts
    /// downregulate the item and negative amounts upregulate it.
    void AdjRegulator(size_t uid, double amt) {
      regulators[uid] = std::max(0.0, regulators.at(uid) + amt);
    }

    /// Set an item's regulator value. Provided value must be greater than or
    /// equal to zero. A value between zero and one upregulates the item, a
    /// value of exactly one is neutral, and a value greater than one
    /// upregulates the item.
    void SetRegulator(size_t uid, double amt) {
      emp_assert(amt >= 0.0);
      regulators.at(uid) = amt;
    }

  };

}

#endif
