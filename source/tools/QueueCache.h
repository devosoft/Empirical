/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020
 *
 *  @file  QueueCache.h
 *  @brief
 *  @note
 */

#ifndef EMP_QUEUECACHE_H
#define EMP_QUEUECACHE_H

#include <list>
#include <unordered_map>
#include <utility>
#include <stdexcept>

namespace emp {
	template <
		class Key,
		class Value,
		class Allocator = std::allocator<std::pair<Key, Value>>
	>
	class QueueCache {
		private:
			typedef typename std::list<typename std::pair<Key, Value>::iterator> cache_t;

			cache_t cache;
			std::unordered_map<Key, typename cache_t::iterator> cache_map;

			const Value& update_cache(typename cache_t::iterator it) {
				// update our cache since we are accesing an item
				cache.splice(
					cache.begin(),
					cache,
					it->second
				);
				return it->second->second;
			}
		public:
			QueueCache(size_t _capacity) {
				cache.resize(_capacity);
			}
			~QueueCache() = default;

			/// Returns number of elements in cache.
			size_t size() { return cache.size(); }

			/// Returns maximum number of elements that will fit in cache.
			size_t capacity() { return cache.max_size(); }

			/// Clears the cache.
			void clear() { cache.clear(); }

			/// Stores element in front of cache.
			void put(const Key& key, const Value& val) {
				// put element into our cache
				// we use insert because it returns a pointer to our element
				auto it = cache.insert(cache.begin(), std::make_pair(key, val));
				// add pointer to this element to our map
				cache_map[key] = it;
			}

			/// Gets an element from cache.
			/// By default, it throws an exception if the element is not in cache.
			/// This behaviour can be changed by specifying a callable as a second parameter.
			template <typename Function>
			const Value& get(const Key& key, const Function&& fun = [](){ throw std::invalid_argument("Key not in cache"); }) {
				// attempt to find key in our iterator map
				auto it = cache_map.find(key);

				// check whether value is in cache
				if (it != cache_map.end()) {
					// value is in cache, so we update it
					update_cache(it);
				} else {
					// value is not in cache, so we call function
					fun();
				}
			}

			/// Resizes the cache.
			/// Returns true if the cache shrunk (elements were lost).
			bool resize(size_t new_size) {
				cache.resize(new_size);
				return new_size < cache.size();
			}

			/// Returns a constant iterator to the beginning of the cache.
			const typename cache_t::const_iterator cbegin() const {
				return cache.begin();
			}

			/// Returns a constant iterator to one past the end of the cache.
			const typename cache_t::const_iterator cend() const {
				return cache.end();
			}

			/// Gets an element from cache if found, and creates it otherwise.
			/// Value type must have a default constructor and an assignment operator.
			std::pair<Key, Value>& operator[](const Key& index) {
				// Check whether Value has a default constructor
				emp_assert(std::is_trivially_constructible<Value>::value);

				// attempt to find key in our iterator map
				auto it = cache_map.find(index);

				// check whether value is in cache
				if (it != cache_map.end()) {
					// value is in cache, so we update it
					update_cache(it);
				} else {
					// value is not in cache, so we constuct it
					auto val = cache.insert(cache.begin(), std::make_pair(index, Value()));
					cache_map[index] = val;

					return *val;
				}
			}
	};
}
#endif