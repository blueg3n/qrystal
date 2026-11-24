///
/// Copyright (C) 2026 Ultralight, Inc. All rights reserved.
/// A license is required for commercial use. https://ultralig.ht
///
#pragma once
#include "Defines.h"
#include <Ultralight/Defines.h>
#include <stdint.h>

#if UL_HAS(TESTING)

/// \cond ignore

// Every GPUDriverCounters field, one row each, in declaration order. X(name, kind) declares a
// uint64_t; A(name, size, kind) declares an array of `size` uint64_t. The kind says how a sampler
// reads two snapshots:
//   Count  cumulative since the driver was created; difference two snapshots.
//   Rate   cumulative like Count; a creation or allocation count also worth reading per second.
//   Live   a current value (a gauge); read it as is, never difference it.
//   Max    a running maximum, or a value recorded with one; read it as is, never difference it.
//
// consumer_copy_bytes counts the bytes an async driver's consumer copied on the CPU into staging
// (payloads that were not already in GPU-visible upload memory). It stays 0 on a backend that does
// not track it.
//
// The rows below are kept by AppCore's asynchronous GPU path and stay 0 elsewhere.
// drawables_reowed counts window frames dropped because the window's size changed after the frame
// was recorded (the window paints again at the new size); frames_handed_off counts window frames
// presented by the main thread in a Core Animation transaction (live resize, Show). main_waits,
// main_wait_us, and main_wait_timeouts count the main thread's waits on the GPU thread, the time
// spent in them, and the waits that gave up; readback_timeouts counts readbacks that gave up
// waiting (they are not readback_failures).
#define UL_GPU_DRIVER_COUNTERS(X, A)          \
  X(frames, Count)                            \
  X(frame_permit_wait_us, Count)              \
  X(command_buffers_committed, Count)         \
  X(command_buffers_completed, Count)         \
  X(render_passes, Count)                     \
  X(render_passes_window, Count)              \
  X(render_passes_clear, Count)               \
  X(blit_encoders, Count)                     \
  X(draws, Count)                             \
  X(draws_dropped, Count)                     \
  X(clears_requested, Count)                  \
  X(clears_folded, Count)                     \
  X(clears_pending, Live)                     \
  X(flushes, Count)                           \
  X(textures_created, Rate)                   \
  X(textures_destroyed, Count)                \
  X(render_targets_created, Rate)             \
  X(render_targets_destroyed, Count)          \
  X(texture_uploads, Count)                   \
  X(texture_upload_bytes, Count)              \
  X(texture_uploads_partial, Count)           \
  X(staging_buffers_reused, Count)            \
  X(staging_buffers_allocated, Rate)          \
  X(staging_buffers_discarded, Count)         \
  X(staging_pool_free_bytes, Live)            \
  X(geometry_created, Rate)                   \
  X(geometry_destroyed, Count)                \
  X(geometry_updates, Count)                  \
  X(geometry_upload_bytes, Count)             \
  X(geometry_buffers_allocated, Rate)         \
  X(drawables_acquired, Count)                \
  X(drawables_missed, Count)                  \
  X(drawables_reowed, Count)                  \
  X(frames_presented, Count)                  \
  X(frames_handed_off, Count)                 \
  X(presented_frame_copies, Count)            \
  X(presented_frame_textures_created, Rate)   \
  X(pipeline_states_created, Rate)            \
  X(readbacks, Count)                         \
  X(readback_failures, Count)                 \
  X(readback_timeouts, Count)                 \
  X(driver_errors, Count)                     \
  X(gpu_busy_us, Count)                       \
  X(gpu_timed_frames, Count)                  \
  X(async_frames, Count)                      \
  X(main_waits, Count)                        \
  X(main_wait_us, Count)                      \
  X(main_wait_timeouts, Count)                \
  X(constant_buffer_blocks_allocated, Rate)   \
  X(upload_chunks_allocated, Rate)            \
  X(consumer_copy_bytes, Count)               \
  X(descriptor_ring_waits, Count)             \
  X(descriptor_ring_wait_us, Count)           \
  X(fence_waits, Count)                       \
  X(fence_wait_us, Count)                     \
  X(allocator_heaps_live, Live)               \
  X(allocator_heaps_created, Rate)            \
  X(textures_reused, Rate)                    \
  X(texture_pool_bytes, Live)                 \
  X(ops_executed, Count)                      \
  A(op_time_hist, 20, Count)                  \
  X(op_time_max_us, Max)                      \
  X(op_time_max_stage, Max)

namespace ultralight {

struct AExport GPUDriverCounters {
#define UL_GPU_COUNTER_FIELD(name, kind) uint64_t name = 0;
#define UL_GPU_COUNTER_ARRAY(name, size, kind) uint64_t name[size] = {};
  UL_GPU_DRIVER_COUNTERS(UL_GPU_COUNTER_FIELD, UL_GPU_COUNTER_ARRAY)
#undef UL_GPU_COUNTER_ARRAY
#undef UL_GPU_COUNTER_FIELD
};

}  // namespace ultralight

/// \endcond

#endif  // UL_HAS(TESTING)
