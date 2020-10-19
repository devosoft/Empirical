#pragma once

#include <algorithm>
#include <utility>

#include "../base/array.h"

// adapted from https://stackoverflow.com/a/30938947

namespace emp {

template<class Key, class Value, size_t N>
class AssociativeArrayCache {

  using value_type = std::pair<Key, Value>;

private:

  using storage_t = emp::array<value_type, N>;

  storage_t storage;

  unsigned char size_{};

  unsigned char oldest{};

  static_assert( N < 256 );

public:

  using iterator = decltype(std::begin(std::declval<storage_t>()));

  using const_iterator = decltype(std::cbegin(std::declval<storage_t>()));

  iterator begin() { return std::begin( storage ); }

  const_iterator begin() const { return std::begin( storage ); }

  const_iterator cbegin() const { return std::cbegin( storage ); }

  iterator end() { return begin() + size(); }

  const_iterator end() const { return cbegin() + size(); }

  const_iterator cend() const { return cbegin() + size(); }

  size_t size() const { return size_; }

  bool empty() const { return size() == 0; }

  static constexpr size_t capacity() { return N; }

  void clear() { size_ = 0; }

public:

  iterator find(const Key& key) { return std::find_if(
    begin(),
    end(),
    [&key](const auto& kv){ const auto& [k, v] = kv; return k == key; }
  ); }

  const_iterator find(const Key& key) const {
    return const_cast<AssociativeArrayCache*>(this)->find(key);
  }

  Value* get(const Key& key) {
    const auto it = find( key );
    if ( it == end() ) return nullptr;
    return std::addressof( it->second );
  }

  Value const* get(const Key& key) const {
    return const_cast<AssociativeArrayCache*>(this)->get( key );
  }

  template<
    class K,
    class V,
    class=std::enable_if_t<
      std::is_convertible< K, Key >{}&&
      std::is_convertible< V, Value >{}
    >
  >
  void set( K&& key, V&& val ) {

    emp_assert( find(key) == end() );

    storage[oldest].first = std::forward<K>(key);
    storage[oldest].second = std::forward<V>(val);

    ++oldest %= N;
    size_ += (size_ < N );

  }

};

} // namespace emp
