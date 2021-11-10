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

#ifndef EMP_MATCHING_MATCHDEPOSITORY_HPP_INCLUDE
#define EMP_MATCHING_MATCHDEPOSITORY_HPP_INCLUDE

#include <algorithm>
#include <limits>

#include "../datastructs/SmallFifoMap.hpp"
#include "../datastructs/SmallVector.hpp"

#include "_DepositoryEntry.hpp"

namespace emp {

template<
  typename Val,
  typename Metric,
  typename Selector,
  typename Regulator,
  size_t RawCacheSize=0,
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
  emp::SmallFifoMap< query_t, res_t, RawCacheSize > cache_raw;

  // Cache of match results with regulation.
  emp::SmallFifoMap< query_t, res_t, RegulatedCacheSize > cache_regulated;

  /// Perform matching with regulation.
  res_t DoRegulatedMatch( const query_t& query ) {

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

    return Selector::select( scores );
  }

  /// Return ptr to cached regulated result on success, nullptr on failure.
  res_t* DoRegulatedLookup( const query_t& query ) {
    return cache_regulated.get( query );
  }

  /// Perform matching without regulation.
  res_t DoRawMatch( const query_t& query ) {

    thread_local emp::vector< float > scores;
    scores.reserve( data.size() );
    scores.clear();

    std::transform(
      std::begin( data ),
      std::end( data ),
      std::back_inserter( scores ),
      [&](const auto& entry){ return Metric::calculate(query, entry.tag); }
    );

    return Selector::select( scores );
  }

  /// Return ptr to cached raw result on success, nullptr on failure.
  res_t* DoRawLookup( const query_t& query ) { return cache_raw.get( query ); }

  /// Clear cached raw, regulated results.
  void ClearCache() {
    if constexpr ( RawCacheSize > 0 ) cache_raw.clear();
    if constexpr ( RegulatedCacheSize > 0 ) cache_regulated.clear();
  }

public:

  /// Compare a query tag to all stored tags using the distance metric
  /// function and return a vector of unique IDs chosen by the selector
  /// function.
  __attribute__ ((hot))
  res_t MatchRegulated( const query_t& query ) {

    if constexpr ( RegulatedCacheSize > 0 ) {
      if (const auto res = DoRegulatedLookup( query ); res != nullptr) {
        return *res;
      }
    }

    return DoRegulatedMatch( query );

  }

  /// Compare a query tag to all stored tags using the distance metric
  /// function and return a vector of unique IDs chosen by the selector
  /// function. Ignore regulators.
  __attribute__ ((hot))
  res_t MatchRaw( const query_t& query ) {

    if constexpr ( RawCacheSize > 0 ) {
      if (const auto res = DoRawLookup( query ); res != nullptr) return *res;
    }

    return DoRawMatch( query );

  }

  /// Access a reference to a single stored value by uid.
  const Val& GetVal( const size_t uid ) const { return data[uid].val; }

  /// Store a value.
  uid_t Put(const Val &v, const tag_t& t) {
    ClearCache();
    data.emplace_back(v, t);
    return data.size() - 1;
  }

  /// Get number of stored values.
  size_t GetSize() const { return data.size(); }

  /// Clear stored values.
  void Clear() { ClearCache(); data.clear(); }

  using adj_t = typename Regulator::adj_t;
  void AdjRegulator( const uid_t uid, const adj_t amt ) {
    if ( data.at(uid).reg.Adj(amt) ) {
      if constexpr ( RegulatedCacheSize > 0 ) cache_regulated.clear();
    }
  }

  using set_t = typename Regulator::set_t;
  void SetRegulator( const uid_t uid, const set_t set ) {
    if ( data.at(uid).reg.Set(set) ) {
      if constexpr ( RegulatedCacheSize > 0 ) cache_regulated.clear();
    }
  }

  void SetRegulator( const uid_t uid, const Regulator& set ) {
    if (set != std::exchange( data.at(uid).reg, set )) {
      if constexpr ( RegulatedCacheSize > 0 ) cache_regulated.clear();
    }
  }

  const Regulator& GetRegulator( const uid_t uid ) { return data.at(uid).reg; }

  using view_t = typename Regulator::view_t;
  const view_t& ViewRegulator( const uid_t uid ) const {
    return data.at(uid).reg.View();
  }

  /// Apply decay to a regulator.
  void DecayRegulator(const uid_t uid, const int steps=1) {
    if ( data.at(uid).reg.Decay(steps) ) {
      if constexpr ( RegulatedCacheSize > 0 ) cache_regulated.clear();
    }
  }

  /// Apply decay to all regulators.
  void DecayRegulators(const int steps=1) {
    for (auto & pack : data ) {
      if ( pack.reg.Decay(steps) ) {
        if constexpr ( RegulatedCacheSize > 0 ) cache_regulated.clear();
      }
    }
  }

  bool HasVal( const Val& val ) const {
    const auto found = std::find_if(
      std::begin( data ),
      std::end( data ),
      [&val]( const auto& entry ) { return entry.val == val; }
    );

    return found != std::end( data );
  }

  uid_t GetUid( const Val& val ) const {

    const auto found = std::find_if(
      std::begin( data ),
      std::end( data ),
      [&val]( const auto& entry ) { return entry.val == val; }
    );

    emp_assert( found != std::end( data ) );

    return std::distance( std::begin( data ), found );
  }

  uid_t GetUid( const tag_t& tag ) const {

    const auto found = std::find_if(
      std::begin( data ),
      std::end( data ),
      [&tag]( const auto& entry ) { return entry.tag == tag; }
    );

    emp_assert( found != std::end( data ) );

    return std::distance( std::begin( data ), found );
  }

};

} // namespace emp

#endif // #ifndef EMP_MATCHING_MATCHDEPOSITORY_HPP_INCLUDE
