/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020
 *
 *  @file  QueueCache.h
 *  @brief
 *  @note
 */

#include <list>
#include <unordered_map>
#include <utility>
#include <stdexcept>
#include <functional>
#include <limits>

#include "../base/assert.h"

 namespace emp {
	template <
		class Key,
		class Value,
		class Hash = std::hash<Key>,
		class Pred = std::equal_to<Key>
	>
	class QueueCache {
		private:
			using cache_t = typename std::list<std::pair<Key, Value>>;
			using cache_map_t = std::unordered_map<Key, typename cache_t::iterator, Hash, Pred>;

			cache_t cache;
			cache_map_t cache_map;


			size_t capacity;

			Value& UpdateCache(typename cache_t::iterator it) {
				// update our cache since we are accesing an item
				cache.splice(
					cache.begin(),
					cache,
					it
				);
				return it->second;
			}
			void Shrink() {
				while (Size() > Capacity()) {
					// deal with removing last element
					cache_map.erase(cache.back().first);
					cache.pop_back();
				}
			}

			static Value DefaultFun(const Key&) {
				throw std::invalid_argument("Key not in cache.");
			}

			void Delete(typename cache_map_t::iterator it) {
				std::cout << "key: " << it->second->first << " val: " << it->second->second << std::endl;
				cache_map.erase(it);
				cache.erase(it->second);
			}

		public:
			QueueCache(size_t _capacity = std::numeric_limits<size_t>::max()) : capacity(_capacity) { ; }
			~QueueCache() = default;

			/// Returns number of elements in cache.
			size_t Size() { return cache.size(); }

			/// Returns maximum number of elements that will fit in cache.
			size_t Capacity() { return capacity; }

			/// Clears the cache.
			void Clear() {
				cache.clear();
				cache_map.clear();
			}

			/// Stores element in front of cache.
			typename cache_t::iterator Put(const Key& key, const Value& val) {
				// put element into our cache
				// we use insert because it returns a pointer to our element
				auto it = cache.emplace(cache.begin(), key, val);
				// add pointer to this element to our map
				cache_map.emplace(key, it);

				Shrink();

				return it;
			}

			/// Gets an element from cache.
			/// By default, it throws an exception if the element is not in cache.
			/// This behaviour can be changed by specifying a callable as a second parameter,
			/// which must return a Value.
			Value& Get(const Key& key, const std::function<Value(const Key& k)>& fun = default_fun) {
				// attempt to find key in our iterator map
				if (!cache_map.count(key)) {
					// value is not in cache, so we call function
					Put(key, fun(key));
				}
				// value is in cache, so we find it
				return UpdateCache(cache_map.find(key)->second);
			}

			/// Resizes the cache.
			void SetCapacity(size_t _capacity) {
				capacity = _capacity;
				Shrink();
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
			Value& operator[](const Key& index) {
				// Check whether Value has a default constructor
				emp_assert(std::is_trivially_constructible<Value>::value);

				// attempt to find key in our iterator map
				if (!cache_map.count(index)) {
					Put(index, Value());
				}
				return Get(index);
			}
	};
}
