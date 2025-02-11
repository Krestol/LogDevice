/**
 * Copyright (c) 2019-present, Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "logdevice/server/ServerHealthMonitor.h"

#include "logdevice/common/Timestamp.h"
#include "logdevice/common/Worker.h"
#include "logdevice/common/chrono_util.h"

namespace facebook { namespace logdevice {

ServerHealthMonitor::ServerHealthMonitor(
    folly::Executor& executor,
    std::chrono::milliseconds sleep_period,
    int num_workers,
    StatsHolder* stats,
    std::chrono::milliseconds max_queue_stalls_avg,
    std::chrono::milliseconds max_queue_stall_duration,
    double max_overloaded_worker_percentage,
    std::chrono::milliseconds max_stalls_avg,
    double max_stalled_worker_percentage)
    : executor_(executor),
      sleep_period_(sleep_period),
      stats_(stats),
      max_queue_stalls_avg_(max_queue_stalls_avg),
      max_queue_stall_duration_(max_queue_stall_duration),
      max_overloaded_worker_percentage_(max_overloaded_worker_percentage),
      max_stalls_avg_(max_stalls_avg),
      max_stalled_worker_percentage_(max_stalled_worker_percentage),
      state_timer_(
          /*min=*/sleep_period, // cannot be unhealthy shorter than one loop
          /*initial=*/sleep_period,
          /*max=*/kMaxTimerValue,
          /*multiplier=*/kMultiplier,
          /*decrease_rate=*/kDecreaseRate,
          /*fuzz_factor=*/kFuzzFactor) {
  internal_info_.num_workers_ = num_workers;
  internal_info_.worker_stalls_.reserve(num_workers);
  internal_info_.worker_stalls_.assign(
      num_workers, {kNumBuckets, kNumPeriods * sleep_period_});
  internal_info_.worker_queue_stalls_.reserve(num_workers);
  internal_info_.worker_queue_stalls_.assign(
      num_workers, {kNumBuckets, kNumPeriods * sleep_period_});
}

void ServerHealthMonitor::startUp() {
  auto now = SteadyTimestamp::now();
  updateVariables(now);
  monitorLoop();
}

void ServerHealthMonitor::monitorLoop() {
  last_entry_time_ = SteadyTimestamp::now();
  folly::futures::sleep(sleep_period_)
      .via(&executor_)
      .then([this](folly::Try<folly::Unit>) mutable {
        STAT_INCR(stats_, health_monitor_num_loops);

        if (shutdown_.load(std::memory_order::memory_order_relaxed)) {
          shutdown_promise_.setValue();
          return;
        }
        int64_t loop_entry_delay = msec_since(last_entry_time_);
        internal_info_.health_monitor_delay_ =
            (loop_entry_delay - sleep_period_.count() > kMaxLoopStall.count())
            ? true
            : false;
        processReports();
        monitorLoop();
      });
}
void ServerHealthMonitor::updateVariables(TimePoint now) {
  std::for_each(internal_info_.worker_stalls_.begin(),
                internal_info_.worker_stalls_.end(),
                [now](TimeSeries& t) { t.update(now); });
  std::for_each(internal_info_.worker_queue_stalls_.begin(),
                internal_info_.worker_queue_stalls_.end(),
                [now](TimeSeries& t) { t.update(now); });
  state_timer_.positiveFeedback(now); // calc how much time has passed
}
bool ServerHealthMonitor::isOverloaded(TimePoint now,
                                       std::chrono::milliseconds half_period) {
  // A node is overloaded when more than max_overloaded_worker_percentage_% of
  // workers have overloaded request queues.
  return (std::count_if(
              internal_info_.worker_queue_stalls_.begin(),
              internal_info_.worker_queue_stalls_.end(),
              [this, now, half_period](TimeSeries& t) {
                // Detection of problematic queuing periods is done on the past
                // 2 HM loops (2*sleep_period_), taking into account time
                // intervals that are equivalent to sleep_period_ start and end
                // in neighboring loops.
                for (int p = 2; p <= 2 * kPeriodRange; ++p) {
                  // Sum of queue stalls during this period.
                  auto period_sum =
                      t.sum(now - p * half_period, now - (p - 2) * half_period);
                  // Number of queue stalls during this period.
                  auto period_count = t.count(
                      now - p * half_period, now - (p - 2) * half_period);
                  if ((period_sum >= max_queue_stall_duration_) &&
                      (period_sum / period_count >= max_queue_stalls_avg_)) {
                    return true;
                  }
                }
                return false;
              }) >= max_overloaded_worker_percentage_ *
              internal_info_.worker_queue_stalls_.capacity());
}
ServerHealthMonitor::StallInfo
ServerHealthMonitor::isStalled(TimePoint now,
                               std::chrono::milliseconds half_period) {
  // A node is stalled when more than max_stalled_worker_percentage_% of
  // workers have stalled requests.
  ServerHealthMonitor::StallInfo info{0, false};
  info.stalled_ =
      (std::count_if(
           internal_info_.worker_stalls_.begin(),
           internal_info_.worker_stalls_.end(),
           [this, now, &info, half_period](TimeSeries& t) {
             // Similar to isOverloaded(), detection of problematic queuing
             // periods is done on the past 2 HM loops, and takes into account
             // additional sleep_period_ intervals that extend into neighboring
             // loops.
             for (int p = 2; p <= 2 * kPeriodRange; ++p) {
               // Sum of request stalls during this period.
               auto period_sum =
                   t.sum(now - p * half_period, now - (p - 2) * half_period);
               // Number of request stalls during this period.
               auto period_count =
                   t.count(now - p * half_period, now - (p - 2) * half_period);
               if (period_count > 0 &&
                   (period_sum / period_count >= max_stalls_avg_)) {
                 // Critically stalled requests are those whose duration is
                 // equal to or greater than the sleep_period_. These represent
                 // a serious concern and have priority over shorter stalls.
                 info.critically_stalled_ +=
                     period_sum / period_count >= sleep_period_ ? 1 : 0;
                 return true;
               }
             }
             return false;
           }) >= max_stalled_worker_percentage_ *
           internal_info_.worker_stalls_.capacity());
  return info;
}
void ServerHealthMonitor::calculateNegativeSignal(TimePoint now) {
  auto half_period = sleep_period_ / 2;
  stall_info_ = isStalled(now, half_period);
  overloaded_ = isOverloaded(now, half_period);
  // temporary
  STAT_ADD(
      stats_, health_monitor_stall_indicator, stall_info_.stalled_ ? 1 : 0);
  STAT_ADD(stats_, health_monitor_overload_indicator, overloaded_ ? 1 : 0);
  if (internal_info_.health_monitor_delay_ || internal_info_.watchdog_delay_ ||
      internal_info_.total_stalled_workers > 0 || stall_info_.stalled_) {
    state_timer_.negativeFeedback();
    state_timer_.positiveFeedback(now); // for timekeeping purposes
  }
  if (stall_info_.critically_stalled_ > 0) {
    state_timer_.negativeFeedback();
    state_timer_.positiveFeedback(now); // for timekeeping purposes
  }
}

void ServerHealthMonitor::processReports() {
  auto now = SteadyTimestamp::now();
  updateVariables(now);
  auto start_time = now - kPeriodRange * sleep_period_;
  auto end_time = now + std::chrono::nanoseconds(1);
  calculateNegativeSignal(now);
  node_state_ = (sleep_period_ < state_timer_.getCurrentValue())
      ? NodeState::UNHEALTHY
      : overloaded_ ? NodeState::OVERLOADED : NodeState::HEALTHY;
  if (node_state_ == NodeState::HEALTHY) {
    STAT_INCR(stats_, health_monitor_state_indicator);
  }
}

folly::SemiFuture<folly::Unit> ServerHealthMonitor::shutdown() {
  shutdown_.exchange(true, std::memory_order::memory_order_relaxed);
  return shutdown_promise_.getSemiFuture();
}

void ServerHealthMonitor::reportWatchdogHealth(bool delayed) {
  if (shutdown_.load(std::memory_order::memory_order_relaxed)) {
    return;
  }
  executor_.add([this, delayed]() mutable {
    if (delayed && !internal_info_.watchdog_delay_) {
      internal_info_.watchdog_delay_ = delayed;
    }
    if (!delayed && internal_info_.watchdog_delay_) {
      internal_info_.watchdog_delay_ = delayed;
    }
  });
}

void ServerHealthMonitor::reportStalledWorkers(int num_stalled) {
  if (shutdown_.load(std::memory_order::memory_order_relaxed)) {
    return;
  }
  executor_.add([this, num_stalled]() mutable {
    internal_info_.total_stalled_workers = num_stalled;
  });
}

void ServerHealthMonitor::reportWorkerQueueStall(
    int idx,
    std::chrono::milliseconds duration) {
  if (shutdown_.load(std::memory_order::memory_order_relaxed)) {
    return;
  }
  auto tp = SteadyTimestamp::now();
  executor_.add([this, idx, tp, duration]() mutable {
    if (idx >= 0 && idx < internal_info_.worker_queue_stalls_.capacity()) {
      internal_info_.worker_queue_stalls_[idx].addValue(tp, duration);
    }
  });
}

void ServerHealthMonitor::reportWorkerStall(
    int idx,
    std::chrono::milliseconds duration) {
  if (shutdown_.load(std::memory_order::memory_order_relaxed)) {
    return;
  }
  auto tp = SteadyTimestamp::now();
  executor_.add([this, idx, tp, duration]() mutable {
    if (idx >= 0 && idx < internal_info_.worker_stalls_.capacity()) {
      internal_info_.worker_stalls_[idx].addValue(tp, duration);
    }
  });
}

}} // namespace facebook::logdevice
