#pragma once

#include <algorithm>
#include <ratio>

#include "../../base/vector.h"
#include "../../datastructs/SmallVector.h"

namespace emp {
namespace statics {

/// Returns top N matches within the threshold ThreshRatio.
template<
  typename ThreshRatio = std::ratio<-1,1>, // neg numerator means +infy
  size_t N = 1
>
struct RankedSelector {

  using res_t = emp::SmallVector<size_t, N + 1>;

  inline static constexpr float thresh = (
    ThreshRatio::num < 0
    ? std::numeric_limits<float>::infinity()
    : static_cast<float>(ThreshRatio::num)
      / static_cast<float>(ThreshRatio::den)
  );

  static res_t select_partition( emp::vector< float >& scores ) {

    const auto partition = std::partition(
      std::begin( scores ),
      std::end( scores ),
      [](const float score){ return score <= thresh; }
    );

    scores.resize( std::distance( std::begin(scores), partition ) );

    return res_t( scores );

  }

  static res_t select_traverse( const emp::vector< float >& scores ) {

    res_t res;

    for (size_t idx{}; idx < scores.size(); ++idx) {
      if ( scores[idx] > thresh ) continue;

      res.push_back( idx );

      if ( res.size() <= N ) continue;

      const auto worst_it = std::max_element(
        std::begin( res ),
        std::end( res ),
        [&scores](const size_t a, const size_t b){
          return scores[a] < scores[b];
        }
      );

      // swap 'n' pop
      *worst_it = res.back();
      res.pop_back();

    }

    return res;

  }

  static res_t select_pick( const emp::vector< float>& scores ) {

    res_t res;

    if ( scores.empty() ) return res;

    const auto best_it = std::min_element(
      std::begin( scores ),
      std::end( scores )
    );

    // if threshold is finite
    if constexpr ( ThreshRatio::num < 0 ) if (*best_it > thresh) return res;

    res.push_back( std::distance( std::begin( scores ), best_it ) );

    return res;

  }

  static res_t select( emp::vector< float >& scores ) {
    if constexpr (N == std::numeric_limits<size_t>::max() ) {
      return select_partition( scores );
    } else if constexpr (N == 1) return select_pick( scores );
    else return select_traverse( scores );
  }

};

} // namespace statics
} // namespace empt
