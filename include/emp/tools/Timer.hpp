/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/tools/Timer.hpp
 * @brief RAII timer for targeted profiling of named code sections.
 * @note Status: BETA
 */

#pragma once

#ifndef INCLUDE_EMP_TOOLS_TIMER_HPP_GUARD
#define INCLUDE_EMP_TOOLS_TIMER_HPP_GUARD

#include <chrono>
#include <iostream>
#include <ostream>
#include <string_view>

#include "../base/assert.hpp"
#include "../data/StatsTracker.hpp"
#include "TemplateString.hpp"

namespace emp {

  /// RAII timer for targeted profiling of named code sections.
  template <TemplateString NAME>
  class Timer {
  public:
    using clock_t    = std::chrono::steady_clock;
    using duration_t = std::chrono::duration<double, std::milli>;
    using stats_t    = StatsTracker<double>;

  private:
    struct TimerSummary {
      stats_t stats;

      ~TimerSummary() { PrintStats(); }

      void PrintStats(std::ostream & os = std::cout) const {
        if (!stats.HasData()) { return; }

        os << "Timer \"" << NAME.AsStringView() << "\": "
           << stats.GetCount() << " sample" << (stats.GetCount() == 1 ? "" : "s")
           << "; min " << stats.GetMin() << " ms"
           << "; max " << stats.GetMax() << " ms"
           << "; avg " << stats.GetAverage() << " ms"
           << "; total " << stats.GetTotal() << " ms"
           << std::endl;
      }
    };

    inline static TimerSummary summary{};

    clock_t::time_point start_time{};
    clock_t::time_point pause_time{};
    duration_t paused_time{0.0};
    bool running = false;
    bool paused  = false;

    double GetElapsedMS() const {
      const auto end_time = paused ? pause_time : clock_t::now();
      return duration_t(end_time - start_time).count() - paused_time.count();
    }

  public:
    Timer(bool start_on_construction=true) {
      if (start_on_construction) Start();
    }

    Timer(const Timer &) = delete;
    Timer(Timer &&) = delete;
    Timer & operator=(const Timer &) = delete;
    Timer & operator=(Timer &&) = delete;

    ~Timer() { Stop(); }

    [[nodiscard]] static constexpr std::string_view GetName() { return NAME.AsStringView(); }

    [[nodiscard]] static const stats_t & GetStats() { return summary.stats; }

    static void ResetStats() { summary.stats.Reset(); }

    static void PrintStats(std::ostream & os = std::cout) { summary.PrintStats(os); }

    [[nodiscard]] bool IsRunning() const { return running; }

    [[nodiscard]] bool IsPaused() const { return paused; }

    [[nodiscard]] double GetCurrentMS() const { return running ? GetElapsedMS() : 0.0; }

    /// Start a new measurement.
    void Start() {
      emp_assert(!running, "Timer already running.");
      start_time = clock_t::now();
      paused_time = duration_t{0.0};
      running = true;
      paused = false;
    }

    /// Stop the current measurement and record it in the summary.
    void Stop() {
      if (!running) { return; }

      summary.stats.Add(GetElapsedMS());
      running = false;
      paused = false;
      paused_time = duration_t{0.0};
    }

    /// Temporarily exclude time from the current measurement.
    void Pause() {
      emp_assert(running, "Timer cannot pause before Start().");
      emp_assert(!paused, "Timer already paused.");
      pause_time = clock_t::now();
      paused = true;
    }

    /// Resume a paused measurement.
    void Unpause() {
      emp_assert(running, "Timer cannot unpause before Start().");
      emp_assert(paused, "Timer is not paused.");
      paused_time += clock_t::now() - pause_time;
      paused = false;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_TOOLS_TIMER_HPP_GUARD
