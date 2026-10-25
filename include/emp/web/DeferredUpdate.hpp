/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 * @file include/emp/web/DeferredUpdate.hpp
 * @brief Owned, combined browser updates; requests made during a callback schedule another turn.
 * 
 * Let web code request an update later and combine repeated requests into one execution.
 * 
 * For example, several setting changes might all require an expensive redraw:
 * 
 *   emp::web::DeferredUpdate update([&] { RedrawVisualization(); });
 *   update.Request();
 *   update.Request();
 *   update.Request();  // Only one redraw is scheduled.
 * 
 * By default, it runs on the next browser animation frame, which suits visual updates.
 */
#pragma once

#ifndef INCLUDE_EMP_WEB_DEFERRED_UPDATE_HPP_GUARD
#define INCLUDE_EMP_WEB_DEFERRED_UPDATE_HPP_GUARD

#include "JSWrap.hpp"

namespace emp::web {

  class DeferredUpdate {
  public:
    enum class Schedule { AnimationFrame, Task };
  private:
    JSCallback callback;
    Schedule schedule;
  public:
    explicit DeferredUpdate(std::function<void()> fn, Schedule mode = Schedule::AnimationFrame)
      : callback(std::move(fn)), schedule(mode) {}
    DeferredUpdate(const DeferredUpdate &) = delete;
    DeferredUpdate & operator=(const DeferredUpdate &) = delete;
    ~DeferredUpdate() { Cancel(); }
    void Request() {
      MAIN_THREAD_EM_ASM({
        const jobs = emp_i.deferredUpdates || (emp_i.deferredUpdates = new Map());
        const id = $0;
        if (jobs.has(id)) return;
        const job = {};
        jobs.set(id, job);
        const run = () => {
          if (jobs.get(id) !== job) return;
          jobs.delete(id);
          if (job.channel) { job.channel.port1.close(); job.channel.port2.close(); }
          emp.Callback(id);
        };
        if ($1) {
          job.channel = new MessageChannel(); job.channel.port1.onmessage = run;
          job.channel.port2.postMessage(null);
        } else job.frame = requestAnimationFrame(run);
      }, callback.GetID(), schedule == Schedule::Task);
    }
    void Cancel() {
      MAIN_THREAD_EM_ASM({
        const jobs = emp_i.deferredUpdates; const job = jobs?.get($0);
        if (!job) return;
        jobs.delete($0);
        if (job.channel) {
          job.channel.port1.onmessage = null;
          job.channel.port1.close(); job.channel.port2.close();
        } else cancelAnimationFrame(job.frame);
      }, callback.GetID());
    }
    bool IsPending() const {
      return MAIN_THREAD_EM_ASM_INT({ return !!emp_i.deferredUpdates?.has($0); }, callback.GetID());
    }
  };
}

#endif  // INCLUDE_EMP_WEB_DEFERRED_UPDATE_HPP_GUARD
