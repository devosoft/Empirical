/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file DepositoryEntry.hpp
 *  @brief Helper struct for MatchDepository.
 *
 */

#ifndef EMP_DEPOSITORY_ENTRY_HPP
#define EMP_DEPOSITORY_ENTRY_HPP

#include <tuple>

namespace emp {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace internal {

template<typename Val, typename Tag, typename Regulator>
struct DepositoryEntry {

  Regulator reg;
  Val val;
  Tag tag;

  DepositoryEntry(const Val& val_, const Tag& tag_) noexcept
  : val(val_), tag(tag_) { ; }

  bool operator==(const DepositoryEntry& other) const {
    return std::tuple{
      reg,
      val,
      tag
    } == std::tuple{
      other.reg,
      other.val,
      other.tag
    };
  }

};

} // namespace internal
#endif // DOXYGEN_SHOULD_SKIP_THIS
} // namespace emp

#endif // #ifndef EMP_DEPOSITORY_ENTRY_HPP
