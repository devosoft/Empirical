/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file MatchDepository.hpp
 *  @brief A container for tag-based lookup, optimized for situations where
 *  tags are not removed from the lookup set.
 *
 */

#ifndef EMP_MATCH_DEPOSITORY_HPP
#define EMP_MATCH_DEPOSITORY_HPP

#include <algorithm>
#include <limits>

#include "../../../third-party/robin-hood-hashing/src/include/robin_hood.h"

#include "../datastructs/SmallFifoMap.hpp"
#include "../datastructs/SmallVector.hpp"

#include "_DepositoryEntry.hpp"

namespace emp {

template<
  typename Val,
  typename Metric,
  typename Selector,
  typename Regulator,
  bool UseRawCache=1,
  size_t RegulatedCacheSize=0
> class MatchDepository {

public:

  using query_t = typename Metric::query_t;
  using tag_t = typename Metric::tag_t;

  using uid_t = size_t;

  using res_t = typename Selector::res_t;

private:

  // Stored entities to match against.
  emp::vector< emp::internal::DepositoryEntry<Val, tag_t, Regulator> > data;

  // Cache of match results without regulation.
  robin_hood::unordered_flat_map< query_t, res_t > cache_raw;

  // Cache of match results with regulation.
  robin_hood::unordered_flat_map< query_t, res_t> cache_regulated;

  robin_hood::unordered_flat_set<query_t> has_shifted;

  /// Perform matching with regulation.
  res_t DoRegulatedMatch( const query_t& query ) noexcept {

    thread_local emp::vector< float > scores;
    scores.reserve( data.size() );
    scores.clear();

    std::transform(
      std::begin( data ),
      std::end( data ),
      std::back_inserter( scores ),
      [&](const auto& entry){
        return entry.reg( Metric::calculate(query, entry.tag) );
      }
    );

    const auto res = Selector::select( scores );

    if constexpr (RegulatedCacheSize > 0) cache_regulated[query] = res;

    return res;

  }

  /// Return ptr to cached regulated result on success, nullptr on failure.
  res_t* DoRegulatedLookup( const query_t& query ) noexcept {
    const auto res = cache_regulated.find( query );
    if (res == std::end(cache_regulated)) return nullptr;
    else return &(res->second);
  }

  /// Perform matching without regulation.
  res_t DoRawMatch( const query_t& query ) noexcept {

    thread_local emp::vector< float > scores;
    scores.reserve( data.size() );
    scores.clear();

    std::transform(
      std::begin( data ),
      std::end( data ),
      std::back_inserter( scores ),
      [&](const auto& entry){ return Metric::calculate(query, entry.tag); }
    );

    const auto res = Selector::select( scores );

    if constexpr ( UseRawCache ) cache_raw.emplace( query, res );

    return res;
  }

  /// Return ptr to cached raw result on success, nullptr on failure.
  res_t* DoRawLookup( const query_t& query ) noexcept {
    const auto res = cache_raw.find( query );
    return (res == std::end( cache_raw )) ? nullptr : &(res->second);
  }

  /// Clear cached raw, regulated results.
  void ClearCache() noexcept {
    // clear is an expensive operation on robin_hood::unordered_flat_map
    if constexpr ( UseRawCache ) if ( cache_raw.size() ) cache_raw.clear();
    if constexpr ( RegulatedCacheSize > 0 ) cache_regulated.clear();
  }

  void UpdateCacheDiffUid(const float diff, const uid_t uid) {
    if ( diff < 0.0 ) { // was upregulated
      // need to register self as possibly better
      // so needs to be checked
      // for (const auto& query : has_shifted) {
      //   cache_regulated.erase(query);
      // }
      // faster than has_shifted.clear() ?
      // for (
      //   auto it = std::begin(has_shifted);
      //   it != std::end(has_shifted);
      //   it = has_shifted.erase(it)
      // );
      // has_shifted.clear();
      // TODO just check and update cache directly

      const auto& competing_entry = data[uid];

      for (const auto& query : has_shifted) {

        if (cache_regulated.count(query) == 0) continue;
        auto& existing_response = cache_regulated.at(query);
        // for now, assume this only iterates zero times or one time
        for (const auto& uid_ : existing_response) {
          const auto& existing_entry = data[uid_];
          const auto existing_dist = existing_entry.reg(
            Metric::calculate(query, existing_entry.tag)
          );

          const auto competing_dist = competing_entry.reg(
            Metric::calculate(query, competing_entry.tag)
          );

          if (competing_dist <  existing_dist) {
            existing_response[0] = uid;
          }

          if (cache_regulated[query] == cache_raw[query]) has_shifted.erase(
            query
          );

        }

      }

    } else if ( diff > 0.0 ) { // was downregulated
      // remove all results where this is returned from res_t cache
      // https://github.com/martinus/robin-hood-hashing/issues/18#issuecomment-461340159
      // adapted from https://en.cppreference.com/w/cpp/container/unordered_map/erase
      for (
        auto it = std::begin(cache_regulated);
        it != std::end(cache_regulated);
      ) {
          if (
            const auto& res = it->second;
            std::find(std::begin(res), std::end(res), uid) != std::end(res)
          ) it = cache_regulated.erase(it);
          else ++it;
      }
    }
  }

public:

  /// Compare a query tag to all stored tags using the distance metric
  /// function and return a vector of unique IDs chosen by the selector
  /// function.
  __attribute__ ((hot))
  res_t MatchRegulated( const query_t& query ) noexcept {

    if constexpr ( RegulatedCacheSize ) {
      if (const auto res = DoRegulatedLookup( query ); res != nullptr) {
        return *res;
      }
    }

    const auto reg_res = DoRegulatedMatch( query );

    if constexpr ( RegulatedCacheSize ) {
      const auto raw_res = MatchRaw(query);
      if (raw_res != reg_res) has_shifted.insert(query);
    }

    return reg_res;

  }

  /// Compare a query tag to all stored tags using the distance metric
  /// function and return a vector of unique IDs chosen by the selector
  /// function. Ignore regulators.
  __attribute__ ((hot))
  res_t MatchRaw( const query_t& query ) noexcept {

    if constexpr ( UseRawCache ) {
      if (const auto res = DoRawLookup( query ); res != nullptr) return *res;
    }

    return DoRawMatch( query );

  }

  /// Access a reference to a single stored value by uid.
  const Val& GetVal( const size_t uid ) const { return data[uid].val; }

  /// Store a value.
  uid_t Put(const Val &v, const tag_t& t) noexcept {
    ClearCache();
    data.emplace_back(v, t);
    return data.size() - 1;
  }

  /// Get number of stored values.
  size_t GetSize() const noexcept { return data.size(); }

  /// Clear stored values.
  void Clear() noexcept { ClearCache(); data.clear(); }

  using adj_t = typename Regulator::adj_t;
  void AdjRegulator( const uid_t uid, const adj_t amt ) noexcept {
    const auto diff = data.at(uid).reg.Adj(amt);
    if constexpr ( RegulatedCacheSize ) UpdateCacheDiffUid(diff, uid);
  }

  using set_t = typename Regulator::set_t;
  void SetRegulator( const uid_t uid, const set_t set ) noexcept {
    const auto diff = data.at(uid).reg.Set(set);
    if constexpr ( RegulatedCacheSize ) UpdateCacheDiffUid(diff, uid);
  }

  // void SetRegulator( const uid_t uid, const Regulator& set ) noexcept {
  //   if (set != std::exchange( data.at(uid).reg, set )) {
  //     if constexpr ( RegulatedCacheSize > 0 ) cache_regulated.clear();
  //   }
  // }

  const Regulator& GetRegulator( const uid_t uid ) noexcept {
    return data.at(uid).reg;
  }

  using view_t = typename Regulator::view_t;
  const view_t& ViewRegulator( const uid_t uid ) const noexcept {
    return data.at(uid).reg.View();
  }

  /// Apply decay to a regulator.
  void DecayRegulator(const uid_t uid, const int32_t steps=1) noexcept {
    const auto diff = data.at(uid).reg.Decay(steps);
    if constexpr ( RegulatedCacheSize ) UpdateCacheDiffUid(diff, uid);
  }

  /// Apply decay to all regulators.
  void DecayRegulators(const int steps=1) noexcept {
    for (size_t uid{}; uid < data.size(); ++uid) {
      const auto diff = data[uid].reg.Decay(steps);
      if constexpr ( RegulatedCacheSize ) UpdateCacheDiffUid(diff, uid);
    }
  }

  bool HasVal( const Val& val ) const noexcept {
    const auto found = std::find_if(
      std::begin( data ),
      std::end( data ),
      [&val]( const auto& entry ) { return entry.val == val; }
    );

    return found != std::end( data );
  }

  uid_t GetUid( const Val& val ) const noexcept {

    const auto found = std::find_if(
      std::begin( data ),
      std::end( data ),
      [&val]( const auto& entry ) { return entry.val == val; }
    );

    emp_assert( found != std::end( data ) );

    return std::distance( std::begin( data ), found );
  }

  uid_t GetUid( const tag_t& tag ) const noexcept {

    const auto found = std::find_if(
      std::begin( data ),
      std::end( data ),
      [&tag]( const auto& entry ) { return entry.tag == tag; }
    );

    emp_assert( found != std::end( data ) );

    return std::distance( std::begin( data ), found );
  }

  bool operator==(const MatchDepository& other) const {
    return data == other.data;
  }

};

} // namespace emp

#endif // #ifndef EMP_MATCH_DEPOSITORY_HPP
