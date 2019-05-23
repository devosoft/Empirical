/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  TODO MatchBin.h
 *  @brief TODO A drop-in replacement for std::bitset, with additional bit magic features.
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

  ///  TODO
  template <typename Val, typename Tag> class MatchBin {
  private:

    const std::function<double(Tag, Tag)> metric;
    const selector_t selector;
    size_t uid_stepper;

    std::unordered_map<size_t, Val> values;
    std::unordered_map<size_t, double> regulators;
    std::unordered_map<size_t, Tag> tags;
    emp::vector<size_t> uids;

  public:

    MatchBin (
      const std::function<double(Tag, Tag)> metric_,
      const selector_t selector_ = ThreshSelector(0.0)
    ) : metric(metric_)
    , selector(selector_)
    , uid_stepper(0) { ; }

    emp::vector<size_t> Match(const Tag & query, size_t n=1) {

      std::unordered_map<Tag, double> matches;
      for (const auto &[uid, tag] : tags) {
        if (matches.find(tag) == matches.end()) {
          matches[tag] = metric(query, tag);
        }
      }

      std::unordered_map<size_t, double> scores;
      for (auto uid : uids) {
        scores[uid] = matches[tags[uid]] * regulators[uid] + regulators[uid];
      }

      return selector(uids, scores, n);
    }

    void Delete(size_t uid) {
      values.erase(uid);
      regulators.erase(uid);
      tags.erase(uid);
      uids.erase(std::remove(uids.begin(), uids.end(), uid), uids.end());
    }

    void Clear() {
      values.clear();
      regulators.clear();
      tags.clear();
      uids.clear();
    }

    void AdjRegulator(size_t uid, double amt) {
      regulators[uid] = std::max(0.0, regulators.at(uid));
    }

    void SetRegulator(size_t uid, double amt) {
      emp_assert(amt >= 0.0);
      regulators.at(uid) = amt;
    }

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

    Val & GetVal(const size_t uid) {
      return values.at(uid);
    }

    Tag & GetTag(const size_t uid) {
      return tags.at(uid);
    }

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


    size_t Size() {
      return values.size();
    }


  };

}

#endif
