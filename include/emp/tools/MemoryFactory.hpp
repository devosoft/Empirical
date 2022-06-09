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

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/error.hpp"
#include "../base/notify.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../polyfill/span.hpp"

namespace emp {

  namespace internal {
    template <typename T, size_t MEM_COUNT, size_t POOL_COUNT>
    struct MemFactory_StaticData {
      static constexpr const bool is_resizable = false;

      // Config
      static constexpr const size_t mem_count = MEM_COUNT;
      static constexpr const size_t pool_count = POOL_COUNT;
      static constexpr const size_t total_count = MEM_COUNT * POOL_COUNT;
      static constexpr const size_t chunk_size = sizeof(T) * MEM_COUNT;
      static constexpr const size_t pool_size = chunk_size * POOL_COUNT;

      // Data
      emp::array<T, total_count> pool;
      emp::array<size_t, POOL_COUNT> free_ids;

      // Interface
      emp::Ptr<T> PoolPtr() const { return &pool; }
      emp::Ptr<T> EndPoolPtr() const { return &((&pool)[1]); }
      void Initialize(size_t _mem_count, size_t _pool_count) {
        emp_error("Cannot re-initialize a static MemoryFactory.");
      }
      auto FreeBegin() const { return free_ids.begin(); }
    };

    template <typename T>
    struct MemFactory_DynamicData {
      static constexpr const bool is_resizable = true;

      // Config
      size_t mem_count = 0;
      size_t pool_count = 0;
      size_t pool_size = 0;
      size_t chunk_size = 0;

      // Data
      emp::Ptr<T> pool;
      emp::vector<size_t> free_ids;

      // Interface
      emp::Ptr<T> PoolPtr() const { return pool; }
      emp::Ptr<T> EndPoolPtr() const { return pool + pool_size; }
      void InitPool(size_t _pool_count) {
        pool_count = _pool_count;
        pool_size = chunk_size * pool_count;
        pool.NewArray(pool_size);
        free_ids.resize(pool_count);
        std::iota(free_ids.begin(), free_ids.end(), 0);        
      }
      void Initialize(size_t _mem_count, size_t _pool_count) {
        emp_assert(mem_count == 0, "Cannot (currently) re-initialize a memory factory.");
        mem_count = _mem_count;
        chunk_size = sizeof(T) * mem_count;
        InitPool(_pool_count);
      }
      auto FreeBegin() const { return free_ids.begin(); }
    };

    template <typename T, typename DATA_T>
    class MemFactory_impl {
    protected:
      DATA_T data;
      size_t free_count = data.pool_count;  // Start all positions free.

      struct RetiredMem {
        emp::Ptr<T> begin_ptr; // Where are these memory chunks?
        emp::Ptr<T> end_ptr;   // How far do this memory chunks extend?
        size_t reserve_count;  // How many memory chunks are still outstanding?

        bool Included(emp::Ptr<T> in) { return in >= begin_ptr && in < end_ptr; }
      };
      emp::vector<RetiredMem> retired_mem;  // Track now-unused memory to free it properly as needed.

      size_t ToID(emp::Ptr<T> in) {
        return (in.Raw() - data.PoolPtr().Raw()) / data.mem_count;
      }
    public:
      MemFactory_impl() {
        for (size_t i = 0; i < data.pool_size; ++i) data.pool[i] = i;
      }
      MemFactory_impl(size_t _mem_count, size_t _pool_count=100) {
        Initialize(_mem_count, _pool_count);
      }
      // Do not copy memory factories.
      MemFactory_impl(MemFactory_impl &) = delete;

      size_t GetChunkSize() const { return data.chunk_size; }
      size_t GetPoolSize() const { return data.pool_size; }

      bool IsFreeID(size_t id) const { return std::any_of(data.FreeBegin(), data.FreeBegin()+free_count, id); }
      bool IsValidID(size_t id) const { return id < data.pool_count; }
      emp::Ptr<T> GetAtID(size_t id) { return &data.pool[id]; }
      std::span<T> GetSpanAtID(size_t id) { return std::span<T>(GetAtID(), data.mem_count); }

      bool IsCurrent(emp::Ptr<T> in) {
        if (in < data.PoolPtr()) return false;
        return IsValidID(ToID(in));
      }

      void Initialize(size_t  _mem_count, size_t _pool_count=100) {
        data.Initialize(_mem_count, _pool_count);
        free_count = data.pool_count;
      }

      emp::Ptr<T> Reserve() {
        if constexpr (DATA_T::is_resizable) {
          if (free_count == 0) { // If we are out of memory, retire current back and request more!
            // Backup the old memory and initialize (bigger) replacement.
            retired_mem.push_back(RetiredMem{data.PoolPtr(), data.EndPoolPtr(), data.pool_count});
            data.InitPool(data.pool_count * 2);
          }
        }
        const size_t id = data.free_ids[--free_count];
        return &data.pool[id];
      }
      emp::Ptr<T> Reserve(size_t id) {
        // Find ID in free list.
        auto it = find(data.free_ids.begin(), data.free_ids.begin()+free_count, id);
        emp_assert(it != data.free_ids.begin()+free_count);   // Make sure it's there!
        *it = data.free_ids.back();                           // Move last ID into its position.
        free_count--;                                         // Eliminate old final ID (now moved).
        return &data.pool[id];                                // Return reserved memory.
      }

      void Release(emp::Ptr<T> in) {
        // If data is resizable, we need to worry about retired memory.
        if constexpr (data.is_resizable) {
          if (!IsCurrent(in)) {
            // Determine which retired pool we are in.
            size_t pool_id;
            while (pool_id < retired_mem.size() && !retired_mem[pool_id].Included(in)) ++pool_id;
            if (pool_id == retired_mem.size()) {
              notify::Exception("MemoryFactory::Release", "Trying to release unknown memory.", in);
            }
            // Release this memory and check if this retired pool is now fully released.
            if (--retired_mem[pool_id].reserve_count == 0) {
              retired_mem.erase(retired_mem.begin() + pool_id);
            }
            return;
          }
        }
        
        const size_t id = ToID(in);
        emp_assert(!IsFreeID(id), "Trying to release ID that is already free.", id);
        emp_assert(IsValidID(id), "Trying to release invalid ID", id);
        data.free_ids[free_count] = id;
        ++free_count;
      }
    };
  }

  template <typename T, size_t MEM_COUNT, size_t POOL_COUNT>
  using StaticMemoryFactory =
    internal::MemFactory_impl<T, internal::MemFactory_StaticData<T, MEM_COUNT, POOL_COUNT>>;
  template <typename T>
  using MemoryFactory =
    internal::MemFactory_impl<T, internal::MemFactory_DynamicData<T>>;

}

#endif