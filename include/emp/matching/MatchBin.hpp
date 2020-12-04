/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file MatchBin.hpp
 *  @brief A container that supports flexible tag-based lookup. .
 *
 */

#ifndef EMP_MATCH_BIN_HPP
#define EMP_MATCH_BIN_HPP

// the default log filename can be set by passing
// '-D filename.csv' to the compiler
#ifndef EMP_LOG_MATCHBIN_FILENAME
#define EMP_LOG_MATCHBIN_FILENAME "matchbin_log.csv"
#endif

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <ratio>
#include <math.h>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <atomic>

#include "../../../third-party/robin-hood-hashing/src/include/robin_hood.h"

#include "../base/assert.hpp"
#include "../base/optional.hpp"
#include "../base/vector.hpp"
#include "../datastructs/IndexMap.hpp"
#include "../bits/BitSet.hpp"
#include "../matching/matchbin_utils.hpp"
#include "../data/DataFile.hpp"
#include "../data/DataNode.hpp"
#include "../base/errors.hpp"

namespace emp {
  template <
    typename Val,
    typename Metric,
    typename Selector,
    typename Regulator
  > class MatchBin;
}

namespace emp::internal {
  // Every time Match or MatchRaw is called on a Matchbin, and logging is enabled,
  // this class will log the query-tag and result-tag (if any).
  // User is responsible for calling FlushLogBuffer() to write data to a file.
  template<typename Query, typename Tag>
  class MatchBinLog {
    using query_t = Query;
    using tag_t = Tag;

    template <
      typename Val,
      typename Metric,
      typename Selector,
      typename Regulator
    >
    friend class emp::MatchBin;


    struct LogEntry {
      query_t query;
      // when a match results in no tags, this is a std::nullopt, and we write an empty string
      emp::optional<tag_t> maybe_tag;
      std::string buffer;

      operator size_t() const { return emp::CombineHash(query, maybe_tag, buffer); }
    };

    using logbuffer_t = robin_hood::unordered_map<
      LogEntry,
      size_t,
      TupleHash<
        LogEntry
      >
    >;

    // stores the number of times we wrote to file
    size_t log_counter;
    // stores the unique ID of this log instance
    size_t instance_id;
    // stores the actual log buffer
    logbuffer_t logbuffer;

    #ifdef EMP_LOG_MATCHBIN
    static constexpr bool logging_enabled = true;
    #else
    static constexpr bool logging_enabled = false;
    #endif
    bool logging_activated;

    // shared file stream for each Matchbin
    // this is done to prevent too many file handles.
    static thread_local inline std::ofstream filestream
    #ifdef EMP_LOG_MATCHBIN
      { EMP_LOG_MATCHBIN_FILENAME }
    #endif
    ;

    // a ContaierDataFile is a DataFile that runs a function on every element of a container before every write.
    // in this case, we simply return the data from our logbuffer.
    emp::ContainerDataFile<logbuffer_t> datafile;

    using datapoint_t = robin_hood::pair<const LogEntry, size_t>;
    // setup getter functions
    std::function<query_t(const datapoint_t)> get_query_log = [](const datapoint_t datapoint){
      return datapoint.first.query;
    };

    std::function<std::string(const datapoint_t)> get_tag_log = [](const datapoint_t datapoint){
      if (datapoint.first.maybe_tag) {
        return emp::to_string(*datapoint.first.maybe_tag);
      } else {
        return emp::to_string("");
      }
    };

    std::function<size_t(const datapoint_t)> get_hit_count_log = [](const datapoint_t datapoint){
      return datapoint.second;
    };

    std::function<std::string(const datapoint_t)> get_logbuffer_type = [](const datapoint_t datapoint) {
      return datapoint.first.buffer;
    };

    /// Creates a unique ID for this logging instance.
    size_t MakeID() {
      static std::atomic<int> counter{0};
      return counter++;
    }

    /// Sets up the various functions and variables we keep track of.
    void SetupDatafile() {
      datafile.SetUpdateContainerFun([this](){ return logbuffer; });

      datafile.AddVar(instance_id, emp::to_string(instance_id), "Instance ID");
      datafile.AddVar(log_counter, "epoch", "Counter for how many times this file was written to");
      datafile.AddContainerFun(get_query_log, "query", "Query");
      datafile.AddContainerFun(get_tag_log, "tag", "Tag");
      datafile.AddContainerFun(get_hit_count_log, "hit_count", "Counter for how many times this match occured");
      datafile.AddContainerFun(get_logbuffer_type, "matchtype", "Type of match");
      datafile.PrintHeaderKeys();
    }

    /// Logs a match into the logbuffer.
    void LogMatch(const query_t& query, const tag_t& tag, const std::string& buffer) {
      if constexpr (logging_enabled) {
        if (logging_activated) {
          LogEntry logentry{query, tag, buffer};
          ++logbuffer[logentry];
        }
      }
    }
    /// Logs a miss (no tag resulted from Match) into the logbuffer.
    void LogMiss(const query_t& query, const std::string& buffer) {
      if constexpr (logging_enabled) {
        if (logging_activated) {
          LogEntry logentry{query, std::nullopt, buffer};
          ++logbuffer[logentry];
        }
      }
    }

    /// Write log buffer to file
    void WriteLogBuffer() {
      if constexpr (logging_enabled) {
        datafile.Update();
        ++log_counter;
      }
    }

    public:
      MatchBinLog()
      : log_counter(0)
      , instance_id(MakeID())
      , logging_activated(false)
      , datafile(filestream)
      { if constexpr (logging_enabled) SetupDatafile(); }

      ~MatchBinLog() {
        if constexpr (logging_enabled) {
          if (!logbuffer.empty() && log_counter == 0) {
            emp::LibraryWarning(
              emp::to_string(
                "Match log buffer was not empty before destructing.\n",
                "Be sure to call ClearLogBuffer() or FlushLogBuffer().\n"
                "Match log buffer was never written to file."
              )
            );
          }
        }
      }

      /// Clear the logbuffer
      void ClearLogBuffer() {
        if constexpr (logging_enabled) {
          logbuffer.clear();
        }
      };

      /// Write logbuffer to file and then clear it
      void FlushLogBuffer() {
        if constexpr (logging_enabled) {
          WriteLogBuffer();
          ClearLogBuffer();
        }
      }

      /// Enable logging
      /// Will have no effect if EMP_MATCHBIN_LOG was not set at compile-time
      void Activate() { logging_activated = true; }

      /// Disable logging
      /// Will have no effect if EMP_MATCHBIN_LOG was not set at compile-time
      void Deactivate() { logging_activated = false; }

      /// Set logging to given argument
      /// Will have no effect if EMP_MATCHBIN_LOG was not set at compile-time
      void Set(bool log) { logging_activated = log; }

      /// Returns whether logging was enabled at compile-time
      /// (aka whether EMP_MATCHBIN_LOG was set)
      constexpr bool IsEnabled() const { return logging_enabled; }

      /// Returns whether logging is activated for this particular instance.
      bool IsActivated() const { return logging_activated; }

      /// Access the data in logbuffer.
      logbuffer_t GetLogBuffer() { return logbuffer; }

      /// Get this log instance's ID
      int GetID() const { return instance_id; }

      /// Constucts a ContainerDataFile in place (without copy) by
      /// forwarding the arguments to the ContainerDataFile constructor,
      /// and then setting up the variables we keep track of.
      /// Look in the ContainerDataFile constructor for this function's type signature.
      template <typename ...ARGS>
      void EmplaceDataFile(ARGS&&... arguments) {
        datafile = decltype(datafile)(std::forward<ARGS>(arguments)...);
        SetupDatafile();
      }

  };
  // This class will cache results for recent tag queries if caching is enabled.
  // It is automatically cleared when operations on the MatchBin would invalidate it (such
  // as adding tags, removing tags, adjusting regulators)
  template <typename Query, typename Selector>
  class MatchBinCache {
    using query_t = Query;
    using cache_state_t = typename Selector::cache_state_t;

    template <typename Val,
      typename Metric,
      typename Selector_,
      typename Regulator>
    friend class MatchBin;

    private:
      #ifdef EMP_THREAD_SAFE
      mutable std::shared_mutex cache_regulated_mutex;
      mutable std::shared_mutex cache_raw_mutex;
      #endif

      static constexpr bool cache_available = std::is_base_of<
        CacheStateBase,
        cache_state_t
      >::value;
      bool caching_activated{cache_available};

      // caches
      // cache of regulated scores
      robin_hood::unordered_flat_map<
        query_t,
        cache_state_t
      > cache_regulated;
      // cache of raw scores
      robin_hood::unordered_flat_map<
        query_t,
        cache_state_t
      > cache_raw;

    public:
      /// Reset the Selector cache for regulated scores.
      void ClearRegulated() {
        if constexpr (cache_available) cache_regulated.clear();
      }

      /// Reset the Selector cache for raw scores.
      void ClearRaw() {
        if constexpr (cache_available) cache_raw.clear();
      }

      /// Reset the Selector cache for all scores.
      void Clear() {
        ClearRaw();
        ClearRegulated();
      }

      /// Returns whether caching is available
      constexpr bool IsAvailable() const { return cache_available; }

      /// Returns whether caching is activated for this particular instance.
      bool IsActivated() { return caching_activated; }

      /// Tries to activate caching for this instance.
      /// Returns true on success.
      bool Activate() {
        Clear();
        if (IsAvailable()) caching_activated = true;
        return caching_activated;
      }

      /// Deactivates caching.
      void Deactivate() {
        Clear();
        caching_activated = false;
      }

      /// Finds a query in raw cache.
      /// This method is thread-safe if EMP_THREAD_SAFE is defined.
      size_t CountRaw(const query_t& query) {
        #ifdef EMP_THREAD_SAFE
        std::shared_lock lock(cache_raw_mutex);
        #endif
        return cache_raw.count(query);
      }

      /// Finds a query in regulated cache.
      /// This method is thread-safe if EMP_THREAD_SAFE is defined.
      size_t CountRegulated(const query_t& query) {
        #ifdef EMP_THREAD_SAFE
        std::shared_lock lock(cache_regulated_mutex);
        #endif
        return cache_regulated.count(query);
      }

      /// Stores a query in regulated cache.
      /// This method is thread-safe if EMP_THREAD_SAFE is defined.
      void CacheRegulated(const query_t& query, const cache_state_t& result) {
        if (CountRegulated(query) != 0) return;
        #ifdef EMP_THREAD_SAFE
        std::unique_lock lock(cache_regulated_mutex);
        #endif
        cache_regulated[query] = result;
      }

      /// Stores a query in raw cache.
      /// This method is thread-safe if EMP_THREAD_SAFE is defined.
      void CacheRaw(const query_t& query, const cache_state_t& result) {
        if (CountRaw(query) != 0) return;
        #ifdef EMP_THREAD_SAFE
        std::unique_lock lock(cache_raw_mutex);
        #endif
        cache_raw[query] = result;
      }

      /// Gets query from regulated cache.
      /// User must check for existance of query in cache before calling this method.
      cache_state_t& GetRegulated(const query_t& query) {
        return cache_regulated.at(query);
      }

      /// Gets query from regulated cache.
      /// User must check for existance of query in cache before calling this method.
      cache_state_t& GetRaw(const query_t& query) {
        return cache_raw.at(query);
      }

      /// Returns size of regulated cache.
      size_t RegulatedSize() { return cache_regulated.size(); }

      /// Returns size of raw cache.
      size_t RawSize() { return cache_raw.size(); }
  };
}

namespace emp {
  /// Internal state packet for MatchBin
  template<typename Val, typename Tag, typename Regulator>
  struct MatchBinState {

    using tag_t = Tag;
    using uid_t = size_t;

    struct entry {
      Val val;
      Regulator regulator;
      Tag tag;

      template <class Archive>
      void serialize( Archive & ar ) {
        ar(
          CEREAL_NVP(val),
          CEREAL_NVP(regulator),
          CEREAL_NVP(tag)
        );
      }

    };

    robin_hood::unordered_flat_map< uid_t, entry > data;

    #ifdef CEREAL_NVP
    template <class Archive>
    void serialize( Archive & ar ) { ar( CEREAL_NVP(data) ); }
    #endif
  };

  /// Abstract base class for MatchBin
  template<typename Val, typename Query, typename Tag, typename Regulator>
  class BaseMatchBin {

  public:
    using query_t = Query;
    using tag_t = Tag;
    using uid_t = size_t;
    using state_t = MatchBinState<Val, tag_t, Regulator>;

    virtual ~BaseMatchBin() {};
    virtual emp::vector<uid_t> Match(const query_t & query, size_t n=0) = 0;
    virtual emp::vector<uid_t> MatchRaw(const query_t & query, size_t n=0) = 0;
    virtual uid_t Put(const Val & v, const tag_t & t) = 0;
    virtual uid_t Set(const Val & v, const tag_t & t, const uid_t uid) = 0;
    virtual void Delete(const uid_t uid) = 0;
    virtual void Clear() = 0;
    virtual void ClearCache() = 0;
    virtual bool ActivateCaching() = 0;
    virtual void DeactivateCaching() = 0;
    virtual Val & GetVal(const uid_t uid) = 0;
    virtual const tag_t & GetTag(const uid_t uid) const = 0;
    virtual void SetTag(const uid_t uid, tag_t tag) = 0;
    virtual emp::vector<Val> GetVals(const emp::vector<uid_t> & uids) = 0;
    virtual emp::vector<tag_t> GetTags(const emp::vector<uid_t> & uids) = 0;
    virtual size_t Size() const = 0;
    virtual void DecayRegulator(
      const uid_t uid,
      const int steps
    ) = 0;
    virtual void DecayRegulators(const int steps=1) = 0;
    virtual void AdjRegulator(
      const uid_t uid,
      const typename Regulator::adj_t & amt
    ) = 0;
    virtual void SetRegulator(
      const uid_t uid,
      const typename Regulator::set_t & set
    ) = 0;
    virtual void SetRegulator(
      const uid_t uid,
      const Regulator & set
    ) = 0;
    virtual const Regulator & GetRegulator(const uid_t uid) = 0;
    virtual const typename Regulator::view_t & ViewRegulator(
      const uid_t uid
    ) const = 0;
    virtual const state_t & GetState() const = 0;
    virtual void ImprintRegulators(const BaseMatchBin & target) = 0;
    virtual void ImprintRegulators(const BaseMatchBin::state_t & target) = 0;
    virtual std::string name() const = 0;
    virtual emp::vector<uid_t> ViewUIDs() const = 0;
    #ifdef EMP_LOG_MATCHBIN
    virtual emp::internal::MatchBinLog<query_t, tag_t>& GetLog() = 0;
    #endif
  };


  /// A data container that allows lookup by tag similarity. It can be templated
  /// on different tag types and is configurable on construction for (1) the
  /// distance metric used to compute similarity between tags and (2) the
  /// selector that is used to select which matches to return. Regulation
  /// functionality is also provided, allowing dynamically adjustment of match
  /// strength to a particular item (i.e., making all matches stronger/weaker).
  /// A unique identifier is generated upon tag/item placement in the container.
  /// This unique identifier can be used to view or edit the stored items and
  /// their corresponding tags. Tag-based lookups return a list of matched
  /// unique identifiers.
  template <
    typename Val,
    typename Metric,
    typename Selector,
    typename Regulator
  >
  class MatchBin
  : public BaseMatchBin<
    Val,
    typename Metric::query_t,
    typename Metric::tag_t,
    Regulator
  > {

  public:
    using base_t = BaseMatchBin<
      Val,
      typename Metric::query_t,
      typename Metric::tag_t,
      Regulator
    >;
    using query_t = typename base_t::query_t;
    using tag_t = typename base_t::tag_t;
    using uid_t = typename base_t::uid_t;
    using state_t = typename base_t::state_t;

    #ifdef EMP_LOG_MATCHBIN
    emp::internal::MatchBinLog<query_t, tag_t> log;
    #endif
  protected:
    state_t state;
    uid_t uid_stepper{};

    emp::internal::MatchBinCache<query_t, Selector> cache;

    Selector selector;


  public:
    MatchBin() = default;

    MatchBin(emp::Random & rand)
    : selector(rand)
    { ; }

    // have to define this manually due to mutexes
    MatchBin(const MatchBin &other)
    : state( other.state )
    , uid_stepper( other.uid_stepper )
    , selector( other.selector ) { }

    // have to define this manually due to mutexes
    MatchBin(MatchBin &&other)
    : state( std::move(other.state) )
    , uid_stepper( std::move(other.uid_stepper) )
    , selector( std::move(other.selector) ) { }

  // have to define this manually due to mutexes
  MatchBin &operator=(const MatchBin &other) {
    selector = other.selector;
    state = other.state;
    uid_stepper = other.uid_stepper;
    cache = other.cache;
    return *this;
  }

  // have to define this manually due to mutexes
  MatchBin &operator=(MatchBin &&other) {
    selector = std::move( other.selector );
    state = std::move( other.state );
    uid_stepper = std::move( other.uid_stepper );
    cache = std::move( other.cache );
    return *this;
  }

    /// Compare a query tag to all stored tags using the distance metric
    /// function and return a vector of unique IDs chosen by the selector
    /// function.
    /// Calling with n = std::numeric_limits<size_t>::max() means
    /// delegate choice for how many values to return to the Selector.
    emp::vector<uid_t> Match(
      const query_t & query,
      size_t n=std::numeric_limits<size_t>::max()
    ) override {
      const auto makeResult = [&]() {
        // compute distance between query and all stored tags
        emp::vector< std::pair< uid_t, double > > scores;
        scores.reserve( state.data.size() );

        std::transform(
          std::begin(state.data),
          std::end(state.data),
          std::back_inserter(scores),
          [&](const auto& pair){
            const auto& [uid, pack] = pair;
            return std::pair{
              uid,
              pack.regulator( Metric::calculate(query, pack.tag) )
            };
          }
        );

        return selector( std::move(scores), n );
      };
      const auto getResult = [&]() {
        // try looking up in cache
        if (cache.IsAvailable() && cache.IsActivated()) {
          // try cache lookup first
          if (cache.CountRegulated(query)) {
            const auto res = cache.GetRegulated(query)(n); /* emp::optional */
            if (res) return res.value();
          }

          auto cacheResult = makeResult();

          cache.CacheRegulated(query, cacheResult);

          return cacheResult(n).value();

        } else {

          auto cacheResult = makeResult();

          return cacheResult(n).value();
        }
      };

      auto result = getResult();

      #ifdef EMP_LOG_MATCHBIN
      // store counts for results
      if (result.empty()) {
        log.LogMiss(query, "regulated");
      }
      for (const auto &uid : result) {
        log.LogMatch(query, GetTag(uid), "regulated");
      }
      #endif

      return result;
    }

    /// Compare a query tag to all stored tags using the distance metric
    /// function and return a vector of unique IDs chosen by the selector
    /// function. Ignore regulators.
    /// Calling with n = std::numeric_limits<size_t>::max() means
    /// delegate choice for how many values to return to the Selector.
    emp::vector<uid_t> MatchRaw(
      const query_t & query,
      size_t n=std::numeric_limits<size_t>::max()
    ) override {
      const auto makeResult = [&]() {
        // compute distance between query and all stored tags
        emp::vector< std::pair< uid_t, double > > scores;
        scores.reserve( state.data.size() );

        std::transform(
          std::begin(state.data),
          std::end(state.data),
          std::back_inserter(scores),
          [&](const auto& pair){
            const auto& [uid, pack] = pair;
            return std::pair{
              uid,
              Metric::calculate(query, pack.tag)
            };
          }
        );

        return selector( std::move(scores), n );
      };
      const auto getResult = [&]() {
        // try looking up in cache
        if (cache.IsAvailable() && cache.IsActivated()) {
          // try cache lookup first
          if (cache.CountRaw(query)) {
            const auto res = cache.GetRaw(query)(n); /* emp::optional */
            if (res) return res.value();
          }
          auto cacheResult = makeResult();

          cache.CacheRaw(query, cacheResult);

          return cacheResult(n).value();

        } else {

          auto cacheResult = makeResult();

          return cacheResult(n).value();
        }
      };
      auto result = getResult();

      #ifdef EMP_LOG_MATCHBIN
      if (result.empty()) {
        log.LogMiss(query, "raw");
      }
      // store counts for results
      for (const auto &uid : result) {
        log.LogMatch(query, GetTag(uid), "raw");
      }
      #endif

      return result;
    }
    /// Put an item and associated tag in the container. Returns the uid for
    /// that entry.
    uid_t Put(const Val & v, const tag_t & t) override {
      const uid_t orig = uid_stepper;
      while (state.data.find(++uid_stepper) != std::end(state.data)) {
        // if container is full
        // i.e., wrapped around because all uids already allocated
        if (uid_stepper == orig) throw std::runtime_error("container full");
      }
      return Set(v, t, uid_stepper);
    }

    /// Put with a manually-chosen UID.
    /// (Caller is responsible for ensuring UID is unique
    /// or calling Delete beforehand.)
    uid_t Set(const Val & v, const tag_t & t, const uid_t uid) override {
      emp_assert(state.data.find(uid) == std::end(state.data));

      cache.Clear();

      state.data[uid] = { v, {}, t };

      return uid;
    }


    /// Delete an item and its associated tag.
    void Delete(const uid_t uid) override {
      emp_assert(state.data.find(uid) != std::end(state.data));

      cache.Clear();

      state.data.erase( uid );

    }

    /// Clear all items and tags.
    void Clear() override {
      cache.Clear();
      state.data.clear();
    }
    /// Reset the Selector caches.
    void ClearCache() override {
      cache.Clear();
    }

    /// Attempt to activate result caching.
    /// @return true if caching activated
    bool ActivateCaching() override {
      return cache.Activate();
    }

    void DeactivateCaching() override {
      cache.Deactivate();
    }

    /// Access a reference single stored value by uid.
    Val & GetVal(const uid_t uid) override {
      emp_assert(state.data.find(uid) != std::end(state.data));
      return state.data.at(uid).val;
    }

    /// Access a const reference to a single stored tag by uid.
    const tag_t & GetTag(const uid_t uid) const override {
      emp_assert(state.data.find(uid) != std::end(state.data));
      return state.data.at(uid).tag;
    }

    /// Change the tag at a given uid and clear the cache.
    void SetTag(const uid_t uid, const tag_t tag) override {
      emp_assert(state.data.find(uid) != std::end(state.data));
      cache.Clear();
      state.data.at(uid).tag = tag;
    }

    /// Generate a vector of values corresponding to a vector of uids.
    emp::vector<Val> GetVals(const emp::vector<uid_t> & uids) override {
      emp::vector<Val> res;
      std::transform(
        std::begin(uids),
        std::end(uids),
        std::back_inserter(res),
        [this](uid_t uid) -> Val { return GetVal(uid); }
      );
      return res;
    }

    /// Generate a vector of tags corresponding to a vector of uids.
    emp::vector<tag_t> GetTags(const emp::vector<uid_t> & uids) override {
      emp::vector<tag_t> res;
      std::transform(
        std::begin(uids),
        std::end(uids),
        std::back_inserter(res),
        [this](uid_t uid) -> tag_t { return GetTag(uid); }
      );
      return res;
    }

    /// Get the number of items stored in the container.
    size_t Size() const override { return state.data.size(); }

    /// Adjust an item's regulator value.
    void AdjRegulator(
      const uid_t uid,
      const typename Regulator::adj_t & amt
    ) override {
      emp_assert(state.data.find(uid) != std::end(state.data));

      if (state.data.at(uid).regulator.Adj(amt)) cache.ClearRegulated();

    }

    /// Set an item's regulator value.
    void SetRegulator(
      const uid_t uid,
      const typename Regulator::set_t & set
    ) override {
      emp_assert(state.data.find(uid) != std::end(state.data));

      if (state.data.at(uid).regulator.Set(set)) cache.ClearRegulated();

    }

    /// Set an item's regulator value.
    void SetRegulator(
      const uid_t uid,
      const Regulator & set
    ) override {
      emp_assert(state.data.find(uid) != std::end(state.data));

      if (
        set != std::exchange(state.data.at(uid).regulator, set)
      ) cache.ClearRegulated();

    }

    /// View an item's regulator value.
    const typename Regulator::view_t & ViewRegulator(
      const uid_t uid
    ) const override {
      emp_assert(state.data.find(uid) != std::end(state.data));

      return state.data.at(uid).regulator.View();
    }

    /// Get a regulator.
    const Regulator & GetRegulator(const uid_t uid) override {
      emp_assert(state.data.find(uid) != std::end(state.data));

      return state.data.at(uid).regulator;
    }

    /// Apply decay to a regulator.
    void DecayRegulator(const uid_t uid, const int steps) override {
      emp_assert(state.data.find(uid) != std::end(state.data));

      if (
        state.data.at(uid).regulator.Decay(steps)
      ) cache.ClearRegulated();

    }

    /// Apply decay to all regulators.
    void DecayRegulators(const int steps=1) override {
      for (auto & [uid, pack] : state.data ) {
        if ( pack.regulator.Decay(steps) ) cache.ClearRegulated();
      }
    }

    /// Set up regulators to match target MatchBin
    /// @param target MatchBin to match
    void ImprintRegulators(
      const BaseMatchBin<Val, query_t, tag_t, Regulator> & target
    ) override { ImprintRegulators(target.GetState()); }

    /// Set up regulators to match target state
    /// @param target State to match
    void ImprintRegulators(const state_t & target) override {

      for (const auto& [uid, pack] : state.data) {

        std::unordered_map<uid_t, double> scores;
        std::transform(
          std::cbegin(target.data),
          std::cend(target.data),
          std::inserter(scores, std::begin(scores)),
          [this, uid = uid](const auto& target_pair){
            const auto& [target_uid, target_pack] = target_pair;
            return std::pair{
              target_uid,
              Metric::calculate(target_pack.tag, GetTag(uid))
            };
          }
        );
        SetRegulator(
          uid,
          target.data.at(
            std::min_element(
              std::begin(scores),
              std::end(scores),
              [](const auto& l, const auto& r){ return l.second < r.second; }
            )->first
          ).regulator
        );

      }

      cache.ClearRegulated();

    }

    /// View UIDs associated with this MatchBin
    emp::vector<uid_t> ViewUIDs() const override {
      emp::vector<uid_t> res;
      res.reserve( state.data.size() );
      std::transform(
        std::begin(state.data),
        std::end(state.data),
        std::back_inserter(res),
        [](const auto& pair){ return pair.first; }
      );
      return res;
    }

    /// Get selector, metric name
    std::string name() const override {
      static Metric metric;
      Regulator reg{};
      return emp::to_string(
        "Selector: ",
        selector.name(),
        " / ",
        "Metric: ",
        metric.name(),
        " / ",
        "Regulator: ",
        reg.name()
      );
    }

    /// Extract MatchBin state
    const state_t & GetState() const override { return state; }

    /// Load MatchBin state
    void SetState(const state_t & state_) {
      state = state_;
      cache.Clear();
    }

    #ifdef EMP_LOG_MATCHBIN
    /// Returns reference to internal logging instance.
    emp::internal::MatchBinLog<query_t, tag_t>& GetLog() override { return log; }
    #endif

    /// Returns size of regulated cache.
    size_t GetRegulatedCacheSize() { return cache.RegulatedSize(); }

    /// Returns size of raw cache.
    size_t GetRawCacheSize() { return cache.RawSize(); }
  };

}

namespace cereal {

template<
  class Archive,
  class T
>
void save(
  Archive & archive,
  robin_hood::unordered_flat_map<size_t, T> const & m
) {

  std::unordered_map<size_t, T> t;
  std::transform(
    std::begin( m ),
    std::end( m ),
    std::inserter( t , std::begin(t) ),
    [](const auto& pair){
      const auto& [k , v] = pair;
      return std::pair{k, v};
    }
  );

  archive( t );
}

template<
  class Archive,
  class T
>
void load(
  Archive & archive,
  robin_hood::unordered_flat_map<size_t, T> & m
) {

  std::unordered_map<size_t, T> t;
  archive( t );

  for (const auto& [k, v] :  t) {
    m[k] = v;
  }

}

} // namespace cereal

#endif // #ifndef EMP_MATCH_BIN_HPP
