#pragma once

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
