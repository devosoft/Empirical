/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file _DepositoryEntry.hpp
 *  @brief Helper struct for MatchDepository.
 *
 */

#ifndef EMP_MATCHING__DEPOSITORYENTRY_HPP_INCLUDE
#define EMP_MATCHING__DEPOSITORYENTRY_HPP_INCLUDE

namespace emp {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace internal {

template<typename Val, typename Tag, typename Regulator>
struct DepositoryEntry {

  Regulator reg;
  Val val;
  Tag tag;

  DepositoryEntry(const Val& val_, const Tag& tag_) : val(val_), tag(tag_) { ; }

};

} // namespace internal
#endif // DOXYGEN_SHOULD_SKIP_THIS
} // namespace emp

#endif // #ifndef EMP_MATCHING__DEPOSITORYENTRY_HPP_INCLUDE
