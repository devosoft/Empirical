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

namespace emp {
namespace internal {

template<typename Val, typename Tag, typename Regulator>
struct DepositoryEntry {

  Regulator reg;
  Val val;
  Tag tag;

  DepositoryEntry(const Val& val_, const Tag& tag_) : val(val_), tag(tag_) { ; }

};

} // namespace internal
} // namespace emp

#endif // #ifndef EMP_DEPOSITORY_ENTRY_HPP
