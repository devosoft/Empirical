/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file MemoryFactory.hpp
 *  @brief Recycles memory of a given size rather than continuously re-allocating.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_TOOLS_MEMORY_FACTORY_HPP_INCLUDE
#define EMP_TOOLS_MEMORY_FACTORY_HPP_INCLUDE

#include "../base/Ptr.hpp"
#include "../base/array.hpp"
#include "../base/vector.hpp"

namespace emp {

  template <typename T, size_t MEM_COUNT, size_t POOL_COUNT>
  class MemoryFactory {
  private:
    static constexpr const size_t chunk_size = sizeof(T) * MEM_COUNT;
    static constexpr const size_t pool_size = chunk_size * POOL_COUNT;
    using chunk_t = emp::array<T, MEM_COUNT>;
    emp::array<chunk_t, POOL_COUNT> pool;
    emp::array<size_t, POOL_COUNT> free_ids;
    size_t free_count = POOL_COUNT;

    size_t ToID(chunk_t in) {
      return (in.data - pool[0].data) / chunk_size;
    }

  public:
    MemoryFactory() {
      std::iota(free_ids.begin(), free_ids.end(), 0);
    }
    MemoryFactory(MemoryFactory &) = delete; // Do not copy memory pools!

    size_t GetChunkSize() const constexpr { return chunk_size; }
    size_t GetPoolSize() const constexpr { return pool_size; }

    bool IsFreeID(size_t id) const { return std::any_of(free_ids.begin(), free_ids.begin()+free_count, id); }
    bool IsValidID(size_t id) const constexpr { return id < POOL_COUNT; }
    chunk_t GetAtID(size_t id) { return pool[id]; }

    chunk_t Reserve() {
      emp_assert(free_count > 0);
      const size_t id = free_ids[--free_count];
      return pool[id];
    }
    chunk_t Reserve(size_t id) {
      auto it = find(free_ids.begin(), free_ids.end(), id); // Find ID in free list.
      emp_assert(it != free_ids.end());                     // Make sure it's there!
      *it = free_ids.back();                                // Move last ID into its position.
      free_count--;                                         // Eliminate old final ID (now moved).
      return pool[id];                                      // Return reserved memory.
    }

    void Release(chunk_t in) {
      const size_t id = ToID(in);
      emp_assert(!IsFreeID(id), "Trying to release ID that is already free.", id);
      emp_assert(IsValidID(id), "Trying to release invalid ID", id);
      free_ids[free_count] = id;
      ++free_count;
    }
  };

  // Where memory requirements are NOT known at compile time.
  template <typename T>
  class MemoryFactory {
  private:
    size_t mem_count;
    size_t pool_count;
    size_t chunk_size;
    size_t pool_size;

    emp::Ptr<T> pool;
    emp::vector<size_t> free_ids;

    size_t ToID(emp::Ptr<T> in) {
      return (in.Raw() - pool.Raw()) / chunk_size;
    }

  public:
    MemoryFactory(size_t _mem_count, size_t _pool_count)
    : mem_count(_mem_count) 
    , pool_count(_pool_count);
    , chunk_size(sizeof(T) * mem_count);
    , pool_size(chunk_size * pool_count);
    {
      pool.NewArray(pool_size);
      free_ids.resize(pool_count);
      std::iota(free_ids.begin(), free_ids.end(), 0);
    }
    MemoryFactory(MemoryFactory &) = delete; // Do not copy memory pools!

    size_t GetChunkSize() const { return chunk_size; }
    size_t GetPoolSize() const { return pool_size; }

    bool IsFreeID(size_t id) const {
      return std::any_of(free_ids.begin(), free_ids.end(), id);
    }
    bool IsValidID(size_t id) const constexpr { return id < pool_count; }
    emp::Ptr<T> GetAtID(size_t id) { return pool[id]; }

    emp::Ptr<T> Reserve() {
      emp_assert(free_count > 0);
      const size_t id = free_ids.back();
      free_ids.resize(free_ids.size()-1);
      return &pool[id*chunk_size];
    }
    emp::Ptr<T> Reserve(size_t id) {
      auto it = find(free_ids.begin(), free_ids.end(), id); // Find ID in free list.
      emp_assert(it != free_ids.end());                     // Make sure it's there!
      *it = free_ids.back();                                // Move last ID into its position.
      free_ids.resize(free_ids.size()-1);                   // Eliminate old final ID (now moved).
      return &pool[id*chunk_size];
    }

    void Release(emp::Ptr<T> in) {
      const size_t id = ToID(in);
      emp_assert(!IsFreeID(id), "Trying to release ID that is already free.", id);
      emp_assert(IsValidID(id), "Trying to release invalid ID", id);
      free_ids[free_count] = id;
      ++free_count;
    }
  };
  
}

#endif