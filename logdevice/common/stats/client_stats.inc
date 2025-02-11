/**
 * Copyright (c) 2017-present, Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
/* can be included multiple times */

#ifndef STAT_DEFINE
#error STAT_DEFINE() macro not defined
#define STAT_DEFINE(...)
#endif

// Used to count the number of active clients in the cluster. This is bumped
// when the client object is created.
STAT_DEFINE(client_started, MAX)

// Appends that came back with different common statuses.  These are
// client-level statuses (which we pass to the client-provided callback), not
// internal error codes that we pass over the wire in APPENDED messages.
STAT_DEFINE(append_success, SUM)
STAT_DEFINE(append_failed_TIMEDOUT, SUM)
STAT_DEFINE(append_failed_NOTFOUND, SUM)
STAT_DEFINE(append_failed_NOTINSERVERCONFIG, SUM)
STAT_DEFINE(append_failed_NOSEQUENCER, SUM)
STAT_DEFINE(append_failed_CONNFAILED, SUM)
STAT_DEFINE(append_failed_PEER_CLOSED, SUM)
STAT_DEFINE(append_failed_TOOBIG, SUM)
STAT_DEFINE(append_failed_NOBUFS, SUM)
STAT_DEFINE(append_failed_SYSLIMIT, SUM)
STAT_DEFINE(append_failed_SEQNOBUFS, SUM)
STAT_DEFINE(append_failed_SEQSYSLIMIT, SUM)
STAT_DEFINE(append_failed_NOSPC, SUM)
STAT_DEFINE(append_failed_OVERLOADED, SUM)
STAT_DEFINE(append_failed_DISABLED, SUM)
STAT_DEFINE(append_failed_ACCESS, SUM)
STAT_DEFINE(append_failed_INTERNAL, SUM)
STAT_DEFINE(append_failed_INVALID_PARAM, SUM)
STAT_DEFINE(append_failed_BADPAYLOAD, SUM)
STAT_DEFINE(append_failed_ISOLATED, SUM)
STAT_DEFINE(append_failed_CANCELLED, SUM)
STAT_DEFINE(append_failed_PEER_UNAVAILABLE, SUM)
STAT_DEFINE(append_failed_other, SUM)

// Client stats to track effectiveness of REDIRECT_NOT_ALIVE
//
// Number of appends that succeeded after being redirected to
// a dead node with the REDIRECT_NOT_ALIVE flag and forcing reactivation
// of the previous sequencer
STAT_DEFINE(append_redirected_not_alive_success, SUM)
// Number of appends that failed after receiving REDIRECT_NOT_ALIVE flag
STAT_DEFINE(append_redirected_not_alive_failed, SUM)

// Write path stats

// Every time an append probe is denied by the server, this counter is
// incremented by the size of the payload, indicating how much bandwidth we
// likely avoided wasting (if we had sent the append and it had instafailed).
STAT_DEFINE(append_probes_bytes_saved, SUM)
// How many bytes were not sent because APPEND_PROBE message could not be sent
// (stat mainly for tests)
STAT_DEFINE(append_probes_bytes_unsent_probe_send_error, SUM)

// GetClusterStateRequest stats
STAT_DEFINE(get_cluster_state_started, SUM)
STAT_DEFINE(get_cluster_state_errors, SUM)
STAT_DEFINE(get_cluster_state_wave_timeout, SUM)
STAT_DEFINE(get_cluster_state_timeout, SUM)
STAT_DEFINE(get_cluster_state_failed, SUM)

// ClusterState stats
STAT_DEFINE(cluster_state_updates, SUM)

// LogsConfigApiRequest stats
STAT_DEFINE(logsconfig_api_request_started, SUM)
STAT_DEFINE(logsconfig_api_request_errors, SUM)
STAT_DEFINE(logsconfig_api_request_timeout, SUM)
STAT_DEFINE(logsconfig_api_request_failed, SUM)

// LogsConfig stats
STAT_DEFINE(logsconfig_start_timeout, SUM)

// Traffic shadowing stats
STAT_DEFINE(shadow_client_init_failed, SUM)
STAT_DEFINE(shadow_client_init_retry_failed, SUM)
STAT_DEFINE(shadow_client_init_retry_success, SUM)
STAT_DEFINE(shadow_payload_alloc_failed, SUM)
STAT_DEFINE(shadow_append_attempt, SUM)
STAT_DEFINE(shadow_append_success, SUM)
STAT_DEFINE(shadow_append_failed, SUM)
STAT_DEFINE(shadow_client_not_loaded, SUM)
STAT_DEFINE(shadow_client_load_retry, SUM)

// API hits stats
//findtime
STAT_DEFINE(findtime_OK, SUM)
STAT_DEFINE(findtime_TIMEDOUT, SUM)
STAT_DEFINE(findtime_INVALID_PARAM, SUM)
STAT_DEFINE(findtime_ACCESS, SUM)
STAT_DEFINE(findtime_PARTIAL, SUM)
STAT_DEFINE(findtime_FAILED, SUM)
STAT_DEFINE(findtime_SHUTDOWN, SUM)
STAT_DEFINE(findtime_OTHER, SUM)
//findkey
STAT_DEFINE(findkey_OK, SUM)
STAT_DEFINE(findkey_TIMEDOUT, SUM)
STAT_DEFINE(findkey_INVALID_PARAM, SUM)
STAT_DEFINE(findkey_ACCESS, SUM)
STAT_DEFINE(findkey_PARTIAL, SUM)
STAT_DEFINE(findkey_FAILED, SUM)
STAT_DEFINE(findkey_SHUTDOWN, SUM)
STAT_DEFINE(findkey_OTHER, SUM)
//get_tail_attributes
STAT_DEFINE(get_tail_attributes_OK, SUM)
STAT_DEFINE(get_tail_attributes_TIMEDOUT, SUM)
STAT_DEFINE(get_tail_attributes_CONNFAILED, SUM)
STAT_DEFINE(get_tail_attributes_NOSEQUENCER, SUM)
STAT_DEFINE(get_tail_attributes_FAILED, SUM)
STAT_DEFINE(get_tail_attributes_NOBUFS, SUM)
STAT_DEFINE(get_tail_attributes_SHUTDOWN, SUM)
STAT_DEFINE(get_tail_attributes_INTERNAL, SUM)
STAT_DEFINE(get_tail_attributes_AGAIN, SUM)
STAT_DEFINE(get_tail_attributes_OTHER, SUM)
//get_head_attributes
STAT_DEFINE(get_head_attributes_OK, SUM)
STAT_DEFINE(get_head_attributes_TIMEDOUT, SUM)
STAT_DEFINE(get_head_attributes_ACCESS, SUM)
STAT_DEFINE(get_head_attributes_INVALID_PARAM, SUM)
STAT_DEFINE(get_head_attributes_SHUTDOWN, SUM)
STAT_DEFINE(get_head_attributes_FAILED, SUM)
STAT_DEFINE(get_head_attributes_OTHER, SUM)
//get_tail_lsn
STAT_DEFINE(get_tail_lsn_OK, SUM)
STAT_DEFINE(get_tail_lsn_TIMEDOUT, SUM)
STAT_DEFINE(get_tail_lsn_CONNFAILED, SUM)
STAT_DEFINE(get_tail_lsn_NOSEQUENCER, SUM)
STAT_DEFINE(get_tail_lsn_FAILED, SUM)
STAT_DEFINE(get_tail_lsn_NOBUFS, SUM)
STAT_DEFINE(get_tail_lsn_SHUTDOWN, SUM)
STAT_DEFINE(get_tail_lsn_INTERNAL, SUM)
STAT_DEFINE(get_tail_lsn_OTHER, SUM)
//is_log_empty
STAT_DEFINE(is_log_empty_OK, SUM)
STAT_DEFINE(is_log_empty_TIMEDOUT, SUM)
STAT_DEFINE(is_log_empty_INVALID_PARAM, SUM)
STAT_DEFINE(is_log_empty_PARTIAL, SUM)
STAT_DEFINE(is_log_empty_ACCESS, SUM)
STAT_DEFINE(is_log_empty_FAILED, SUM)
STAT_DEFINE(is_log_empty_NOBUFS, SUM)
STAT_DEFINE(is_log_empty_SHUTDOWN, SUM)
STAT_DEFINE(is_log_empty_INTERNAL, SUM)
STAT_DEFINE(is_log_empty_NOTEMPTY, SUM)
STAT_DEFINE(is_log_empty_NOTFOUND, SUM)
STAT_DEFINE(is_log_empty_CONNFAILED, SUM)
STAT_DEFINE(is_log_empty_OTHER, SUM)
STAT_DEFINE(is_log_empty_flappy, SUM)
//data_size
STAT_DEFINE(data_size_OK, SUM)
STAT_DEFINE(data_size_TIMEDOUT, SUM)
STAT_DEFINE(data_size_INVALID_PARAM, SUM)
STAT_DEFINE(data_size_PARTIAL, SUM)
STAT_DEFINE(data_size_ACCESS, SUM)
STAT_DEFINE(data_size_FAILED, SUM)
STAT_DEFINE(data_size_NOBUFS, SUM)
STAT_DEFINE(data_size_SHUTDOWN, SUM)
STAT_DEFINE(data_size_INTERNAL, SUM)
STAT_DEFINE(data_size_OTHER, SUM)
//trim
STAT_DEFINE(trim_OK, SUM)
STAT_DEFINE(trim_TIMEDOUT, SUM)
STAT_DEFINE(trim_INVALID_PARAM, SUM)
STAT_DEFINE(trim_FAILED, SUM)
STAT_DEFINE(trim_PARTIAL, SUM)
STAT_DEFINE(trim_ACCESS, SUM)
STAT_DEFINE(trim_NOTFOUND, SUM)
STAT_DEFINE(trim_OTHER, SUM)

// Client Events
STAT_DEFINE(critical_events, SUM)
STAT_DEFINE(error_events, SUM)
STAT_DEFINE(warning_events, SUM)
STAT_DEFINE(notice_events, SUM)
STAT_DEFINE(info_events, SUM)


// Metadata API
STAT_DEFINE(metadata_api_failures, SUM)
STAT_DEFINE(metadata_api_total, SUM)

// Appends
STAT_DEFINE(append_requests, SUM)
STAT_DEFINE(unexpected_append_failures, SUM)

#undef STAT_DEFINE
