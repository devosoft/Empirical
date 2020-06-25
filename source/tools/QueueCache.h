/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  QueueCache.h
 *  @brief A simple implementation of a Least-Recently Used Cache.
 *		It orders elements by access time and removes the stalest ones in case maximum capacity is reached.
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
		size_t DefaultCapacity = std::numeric_limits<size_t>::max(),
		class Hash = std::hash<Key>,
		class Pred = std::equal_to<Key>
	>
	class QueueCache {
		private:
			using cache_list_t = typename std::list<std::pair<Key, Value>>;
			using cache_map_t = std::unordered_map<Key, typename cache_list_t::iterator, Hash, Pred>;

			cache_list_t cache_list;
			cache_map_t cache_map;


			size_t capacity;

			Value& UpdateCache(const typename cache_list_t::iterator it) {
				// update our cache since we are accesing an item
				cache_list.splice(
					cache_list.begin(),
					cache_list,
					it
				);
				return it->second;
			}
			void Shrink() {
				while (Size() > Capacity()) {
					// deal with removing last element
					cache_map.erase(cache_list.back().first);
					cache_list.pop_back();
				}
			}

			static Value DefaultFun(const Key&) {
				throw std::invalid_argument("Key not in cache.");
			}

			void Delete(const typename cache_map_t::iterator it) {
				cache_map.erase(it);
				cache_list.erase(it->second);
			}

		public:
			QueueCache(size_t _capacity = DefaultCapacity) : capacity(_capacity) { ; }
			~QueueCache() = default;

			using const_iterator = typename cache_list_t::const_iterator;
			using iterator = typename cache_list_t::const_iterator;

			/// Returns number of elements in cache.
			size_t Size() const { return cache_list.size(); }

			/// Returns maximum number of elements that will fit in cache.
			size_t Capacity() { return capacity; }

			/// Clears the cache.
			void Clear() {
				cache_list.clear();
				cache_map.clear();
			}

			/// Deletes element from cache.
			void Delete(const Key& key) {
				auto it = cache_map.find(key);
				Delete(it);
			}

			/// Returns true if cache has key.
			bool Contains(const Key& key) {
				return cache_map.count(key);
			}

			/// Stores element in front of cache.
			typename cache_list_t::iterator Put(const Key& key, const Value& val) {
				// try to find element in map
				auto found = cache_map.find(key);
				if (found != cache_map.end()) {
					Delete(found);
				}

				// put element into our cache
				// we use insert because it returns a pointer to our element
				cache_list.emplace_front(key, val);
				// add pointer to this element to our map
				cache_map.emplace(key, cache_list.begin());
				Shrink();

				return cache_list.begin();
			}

			/// Gets an element from cache.
			/// By default, it throws an exception if the element is not in cache.
			/// This behaviour can be changed by specifying a callable as a second parameter,
			/// which must return a Value.
			Value& Get(const Key& key, const std::function<Value(const Key& k)>& fun = DefaultFun) {
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

			const typename cache_t::const_iterator begin() const { return cbegin(); }
			const typename cache_t::const_iterator end() const { return cend(); }

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
