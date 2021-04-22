/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  QueueCache.hpp
 *  @brief A simple implementation of a Least-Recently Used Cache.
 *		It orders elements by access time and removes the stalest ones in case maximum capacity is reached.
 */

#ifndef EMP_QUEUECACHE_H
#define EMP_QUEUECACHE_H

#include <list>
#include <unordered_map>
#include <utility>
#include <stdexcept>
#include <functional>
#include <limits>

#include "../base/assert.hpp"

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

			// this list keeps track of the order of the elements in cache
			cache_list_t cache_list;
			// this map associates a key with an iterator to the cache_list, facilitating lookups
			cache_map_t cache_map;
			// maximum number of elements the cache can hold
			size_t capacity;

			// Put the iterator at the beginning of the list, and returns its value
			// @param it Iterator to element to update
			// @return Reference to value of updated element
			Value& UpdateCache(const typename cache_list_t::iterator it) {
				// update our cache since we are accesing an item
				cache_list.splice(
					cache_list.begin(),
					cache_list,
					it
				);
				return it->second;
			}

			// Shrink cache to its capacity by removing elements at the end of it
			void Shrink() {
				while (Size() > Capacity()) {
					// deal with removing last element
					cache_map.erase(cache_list.back().first);
					cache_list.pop_back();
				}
			}

			// Default function to be run by Get() if none is provided
			// @param key Key of element passed to Get(). Unused.
			static Value DefaultFun(const Key&) {
				throw std::invalid_argument("Key not in cache.");
			}

			// Delete given iterator from cache
			// @param it cache_map iterator to element to be deleted from cache
			void Delete(const typename cache_map_t::iterator it) {
				cache_list.erase(it->second);
				cache_map.erase(it);
			}

		public:
			QueueCache(const size_t _capacity = DefaultCapacity) : capacity(_capacity) { ; }
			~QueueCache() = default;

			using const_iterator = typename cache_list_t::const_iterator;
			using iterator = typename cache_list_t::const_iterator;

			/// Return number of elements in cache.
			/// @return number of elements in the cache
			size_t Size() const { return cache_list.size(); }

			/// Test if cache has any elements.
			/// @return true if cache is empty
			bool Empty() const { return cache_list.size() == 0; }

			/// Return maximum number of elements that will fit in cache.
			/// @return maximum number of elements that the cache can contain
			size_t Capacity() const { return capacity; }

			/// Clear the cache.
			void Clear() {
				cache_list.clear();
				cache_map.clear();
			}

			/// Delete element from cache.
			/// @param key Key to delete from cache
			void Delete(const Key& key) {
				Delete(
					cache_map.find(key)
				);
			}

			/// Does cache contain key?
			/// @param key Key to check presence of
			/// @return whether cache contains key
			bool Contains(const Key& key) const {
				return cache_map.count(key);
			}

			/// Store element in front of cache.
			/// @param key Key of element to store
			/// @param val Value of element to store
			/// @return Iterator to newly-added element in cache queue
			typename cache_list_t::iterator Put(const Key& key, const Value& val) {
				// try to find element in map
				const auto found = cache_map.find(key);
				if (found != cache_map.end()) {
					Delete(found);
				}
				// put element into our cache
				cache_list.emplace_front(key, val);
				// add pointer to this element to our map
				cache_map.emplace(key, cache_list.begin());
				// make sure we don't have more elements than our capacity
				Shrink();

				return cache_list.begin();
			}

			/// Get an element from cache.
			/// By default, this method throws an exception if the element is not in cache.
			/// This behaviour can be changed by specifying a callable as a second parameter,
			/// which must return a Value.
			/// @param key Key of element to get
			/// @param fun Optional function to be executed in case the key is not in cache
			Value& Get(const Key& key, const std::function<Value(const Key& k)>& fun = DefaultFun) {
				// attempt to find key in our iterator map
				if (!cache_map.count(key)) {
					// value is not in cache, so we call function
					Put(key, fun(key));
				}
				// value is in cache, so we find it
				return UpdateCache(cache_map.find(key)->second);
			}

			/// Resize the cache.
			/// @param _capacity New capacity of the cache.
			void SetCapacity(const size_t _capacity) {
				capacity = _capacity;
				Shrink();
			}

			/// Return a constant iterator to the beginning of the cache queue.
			/// @return Constant const_iterator to the beginning of cache queue
			const typename cache_list_t::const_iterator cbegin() const {
				return cache_list.begin();
			}

			/// Return a constant iterator to one past the end of the cache queue.
			/// @return Constant const_iterator to one past the end of cache queue
			const typename cache_list_t::const_iterator cend() const {
				return cache_list.end();
			}

			/// Return a constant iterator to the beginning of the cache queue.
			/// Alias of cbegin()
			/// @return Constant const_iterator to the beginning of cache queue
			const typename cache_list_t::const_iterator begin() const { return cbegin(); }

			/// Return a constant iterator to one past the end of the cache queue.
			/// Alias of cend()
			/// @return Constant const_iterator to one past the end of cache queue
			const typename cache_list_t::const_iterator end() const { return cend(); }

			/// Get an element from cache if found, and creates it otherwise.
			/// Value type must have a default constructor and an assignment operator.
			/// @param index Key of element to add/retrieve
			/// @return Reference to value of element of given key
			Value& operator[](const Key& index) {
				// Use of this method requires a Value that is default constructible.
				static_assert(std::is_trivially_constructible<Value>::value,
					"Operator subscript requires a default constructor");
				// add key if it's not in cache
				if (!cache_map.count(index)) {
					Put(index, Value());
				}

				return Get(index);
			}
	};
}
#endif
