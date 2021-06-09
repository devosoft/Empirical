/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  SmallFifoMap.hpp
 *  @brief Store key value pairs in a fixed-sized array, bumping out the oldest
 *  value when full. Optimized for small N. Requires N < 256.
 *
 */

#ifndef EMP_ASSOCIATIVE_ARRAY_CACHE_HPP
#define EMP_ASSOCIATIVE_ARRAY_CACHE_HPP

#include <algorithm>
#include <utility>

#include "../base/array.hpp"

// adapted from https://stackoverflow.com/a/30938947

namespace emp {

template<class Key, class Value, size_t N>
class SmallFifoMap {

  using value_type = std::pair<Key, Value>;

private:

  using storage_t = emp::array<value_type, N>;

  storage_t storage;

  unsigned char size_{};

  // index of stalest element in cache, according to insertion order
  unsigned char oldest{};

  static_assert( N < 256 );

public:

  using iterator = typename storage_t::iterator;

  using const_iterator = typename storage_t::const_iterator;

  iterator begin() noexcept { return storage.begin(); }

  const_iterator begin() const noexcept { return std::cbegin( storage ); }

  const_iterator cbegin() const noexcept { return std::cbegin( storage ); }

  iterator end() noexcept { return begin() + size(); }

  const_iterator end() const noexcept { return cbegin() + size(); }

  const_iterator cend() const noexcept { return cbegin() + size(); }

  /// How many key-value pairs are in the cache?
  size_t size() const noexcept { return size_; }

  /// Does the cache contain any key-value pairs?
  bool empty() const noexcept { return size() == 0; }

  /// How many key-value pairs can the cache contain?
  static constexpr size_t capacity() noexcept { return N; }

  /// Clear the cache.
  void clear() noexcept { size_ = 0; oldest = 0; }

  /// Find key-value pair iterator in cache.
  iterator find(const Key& key) noexcept { return std::find_if(
    begin(),
    end(),
    [&key](const auto& kv){ const auto& [k, v] = kv; return k == key; }
  ); }

  /// Find key-value pair iterator in cache.
  const_iterator find(const Key& key) const noexcept {
    return const_cast<SmallFifoMap*>(this)->find(key);
  }

  /// Get corresponding value from cache. Return nullptr if key not in cache.
  Value* get(const Key& key) noexcept {
    const auto it = find( key );
    if ( it == end() ) return nullptr;
    return std::addressof( it->second );
  }

  /// Get corresponding value from cache. Return nullptr if key not in cache.
  Value const* get(const Key& key) const noexcept {
    return const_cast<SmallFifoMap*>(this)->get( key );
  }

  /// Get corresponding value from cache.
  Value& operator[](const Key& key) noexcept {
    const auto it = find( key );
    emp_assert( it != end() );
    return it->second;
  }

  /// Get corresponding value from cache.
  const Value& operator[](const Key& key) const noexcept {
    return const_cast<SmallFifoMap*>(this)->operator[]( key );
  }

  /// Put a key-value pair in the cache.
  template<
    class K,
    class V,
    class=std::enable_if_t<
      std::is_convertible< K, Key >{}&&
      std::is_convertible< V, Value >{}
    >
  >
  void set( K&& key, V&& val ) noexcept {

    emp_assert( find(key) == end() );

    storage[oldest].first = std::forward<K>(key);
    storage[oldest].second = std::forward<V>(val);

    ++oldest %= N;
    size_ += (size_ < N );

  }

};

} // namespace emp

#endif // #ifndef EMP_ASSOCIATIVE_ARRAY_CACHE_HPP
