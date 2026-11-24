///
/// Copyright (C) 2026 Ultralight, Inc. All rights reserved.
/// A license is required for commercial use. https://ultralig.ht
///

///
/// @file CAPI_GPUDriverAsync.h
///
/// A GPU driver that records the library's GPU work into frames for another thread to execute.
///
/// `#include <AppCore/CAPI/CAPI_GPUDriverAsync.h>`
///
/// The library makes every GPU driver call on the thread that calls ulRender() (the Renderer's
/// thread). This driver records those calls into frames (ULGPUFrame) and queues them, so you can
/// execute them on your own render thread with your own graphics API.
///
/// Each frame holds the GPU work of one command list, plus every texture, render buffer, and
/// geometry change the library made before it. The data a frame refers to stays valid until you
/// destroy the frame, so you don't need to copy anything.
///
/// ## Setting Up the Driver
///
/// Create the driver and register it before calling ulCreateRenderer():
///
/// ```
/// ULGPUDriverAsyncOptions options = {0};
/// options.struct_size = sizeof(ULGPUDriverAsyncOptions);
/// options.caps.supports_msaa = 1;
/// options.user_data = my_render_thread;
/// options.on_frame_queued = OnFrameQueued;   // Wake your render thread here.
///
/// ULGPUDriverAsync driver = ulCreateGPUDriverAsync(&options);
/// ulPlatformSetGPUDriverAsync(driver);
/// ```
///
/// ## Producing Frames
///
/// On the Renderer's thread, render only when the driver is ready for another frame. Skipping
/// ulRender() is safe: the library draws everything that changed the next time you call it.
///
/// ```
/// ulUpdate(renderer);
/// ulRefreshDisplay(renderer, display_id);
/// if (ulGPUDriverAsyncIsReadyForFrame(driver))
///   ulRender(renderer);
/// ```
///
/// ## Executing Frames
///
/// On your render thread, take each queued frame, apply it in order, then destroy it:
///
/// ```
/// ULGPUFrame frame;
/// while ((frame = ulGPUDriverAsyncTryDequeue(driver))) {
///   unsigned int count;
///   const ULGPUTextureOp* textures = ulGPUFrameGetTextures(frame, &count);
///   // ... apply the textures, render buffers, and geometry, draw the commands,
///   //     then serve the readbacks.
///   ulDestroyGPUFrame(frame);
/// }
/// ```
///
/// Never drop, merge, or reorder frames.
///
/// ## Threading
///
/// - **The Renderer's thread:** ulCreateGPUDriverAsync(), ulDestroyGPUDriverAsync(),
///   ulPlatformSetGPUDriverAsync(), ulGPUDriverAsyncReadbackTexture(),
///   ulGPUDriverAsyncPurgeMemory(), and every callback in ULGPUAllocator and
///   ULGPUDriverAsyncOptions.
/// - **Your render thread (one thread at a time):** ulGPUDriverAsyncTryDequeue().
/// - **Any thread:** ulDestroyGPUFrame(), the ulGPUFrame* accessors, the getters, and
///   ulGPUDriverAsyncCompleteReadback().
///
/// A frame may move between threads, but only one thread may use it at a time.
///
/// ## Shutting Down
///
/// Destroy the Renderer first, then destroy every frame you still hold (and release every
/// ULBuffer you retained from one), then destroy the driver.
///
#ifndef APPCORE_CAPI_GPUDRIVERASYNC_H
#define APPCORE_CAPI_GPUDRIVERASYNC_H

#include <AppCore/CAPI/CAPI_Defines.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// Opaque handle to an async GPU driver.
///
/// @see  ulCreateGPUDriverAsync(), ulDestroyGPUDriverAsync()
///
typedef struct C_GPUDriverAsync* ULGPUDriverAsync;

///
/// Opaque handle to a recorded frame.
///
/// @see  ulGPUDriverAsyncTryDequeue(), ulDestroyGPUFrame()
///
typedef struct C_GPUFrame* ULGPUFrame;

///
/// Pass as ULGPUDriverAsyncOptions::readback_timeout_ms to wait for a readback until it completes.
///
#define UL_GPU_DRIVER_ASYNC_NO_TIMEOUT 0xFFFFFFFFu

/******************************************************************************
 * Memory
 *****************************************************************************/

///
/// Who may access a block of memory, and how (combine with `|`).
///
typedef enum {
  kULGPUMemoryUsage_None     = 0,
  kULGPUMemoryUsage_CPURead  = 1 << 0,  ///< The CPU reads it.
  kULGPUMemoryUsage_CPUWrite = 1 << 1,  ///< The CPU writes it.
  kULGPUMemoryUsage_GPURead  = 1 << 2,  ///< The GPU reads it in place (only memory from your own
                                        ///< allocator has this).
  kULGPUMemoryUsage_GPUWrite = 1 << 3,  ///< The GPU writes it in place (readback memory).
} ULGPUMemoryUsage;

///
/// A block of memory inside a buffer.
///
/// `buffer` is the whole buffer the block lives in (eg, one chunk your allocator handed out, or an
/// image's pixels). Its user data (ulBufferGetUserData()) is whatever you set when you created it,
/// eg, your native resource.
///
typedef struct {
  ULBuffer buffer;     ///< The buffer holding this block (NULL when the block is empty).
  size_t offset;       ///< The start of the block, in bytes from ulBufferGetData().
  size_t size;         ///< The size of the block, in bytes.
  unsigned int usage;  ///< A combination of ULGPUMemoryUsage values.
} ULGPUMemory;

///
/// Callback for users to allocate a block of memory.
///
/// @param  user_data  The ULGPUAllocator::user_data you provided.
///
/// @param  size       The number of bytes needed.
///
/// @param  alignment  The required alignment of the block's start address, in bytes.
///
/// @param  usage      The access the block must allow (a combination of ULGPUMemoryUsage
///                    values). Payloads ask for kULGPUMemoryUsage_CPUWrite; readbacks ask for
///                    kULGPUMemoryUsage_GPUWrite | kULGPUMemoryUsage_CPURead.
///
/// @param  out        The block to fill in. Set `out->buffer` to a ULBuffer you own a reference
///                    to (eg, from ulCreateBufferRef()), `out->usage` to the access it allows
///                    (it must include `usage`, and may add kULGPUMemoryUsage_GPURead), and
///                    `out->offset` / `out->size` to the block inside it.
///
/// @return  Returns true if you filled in `out`, or false to let the driver use its own CPU
///          memory for this block.
///
/// @note  The library takes over the reference in `out->buffer` and calls ulDestroyBuffer() on
///        it when it no longer needs the block.
///
/// @warning  This must never block.
///
typedef bool (*ULGPUAllocatorAllocateCallback)(void* user_data, size_t size, size_t alignment,
                                               unsigned int usage, ULGPUMemory* out);

///
/// Callback for users to report an alignment rule (in bytes) for memory of a certain usage.
///
/// @param  user_data  The ULGPUAllocator::user_data you provided.
///
/// @param  usage      The memory's usage (a combination of ULGPUMemoryUsage values).
///
/// @return  Returns the alignment, in bytes (1 for none).
///
typedef size_t (*ULGPUAllocatorAlignmentCallback)(void* user_data, unsigned int usage);

///
/// User-defined allocator for the memory that frames hold.
///
/// The driver copies texture and geometry data into this memory, and asks for readback memory
/// from it. You can use this to have the driver write straight into memory your GPU reads (eg,
/// a persistently mapped upload buffer), so your render thread doesn't need another copy.
///
/// Without an allocator, the driver uses its own CPU memory.
///
/// @see  ULGPUDriverAsyncOptions::allocator
///
typedef struct {
  void* user_data;  ///< Passed to every callback below (can be NULL).

  ULGPUAllocatorAllocateCallback allocate;  ///< Required. If NULL, the allocator is ignored.

  ///
  /// The row alignment your graphics API needs to copy texture rows out of memory of a certain
  /// usage. May be NULL (1).
  ///
  ULGPUAllocatorAlignmentCallback texture_row_alignment;

  ///
  /// The offset alignment your graphics API needs to copy a texture out of memory of a certain
  /// usage. May be NULL (1).
  ///
  ULGPUAllocatorAlignmentCallback texture_offset_alignment;
} ULGPUAllocator;

/******************************************************************************
 * Resource operations
 *****************************************************************************/

///
/// What an operation does to its resource.
///
typedef enum {
  kULGPUResourceAction_Create = 0,
  kULGPUResourceAction_Update = 1,
  kULGPUResourceAction_Destroy = 2,
} ULGPUResourceAction;

///
/// A texture operation.
///
/// Create: `pixels` holds the whole texture (empty for a render target or an empty bitmap). Apply
/// it when you create the texture (an immutable or block-compressed texture can only get its data
/// then).
///
/// Update: `pixels` holds `rect` of the texture. For a block-compressed format, `rect` is
/// block-aligned and `row_pitch` is the bytes between rows of blocks.
///
typedef struct {
  unsigned int id;             ///< The texture.
  ULGPUResourceAction action;  ///< What to do.
  unsigned int width;          ///< The width, in pixels (Create).
  unsigned int height;         ///< The height, in pixels (Create).
  ULBitmapFormat format;       ///< The pixel format.
  unsigned int flags;          ///< A combination of ULGPUTextureFlags values (Create).
  ULIntRect rect;              ///< The region written (Update).
  size_t row_pitch;            ///< The bytes between rows in `pixels`.
  ULGPUMemory pixels;          ///< The pixel data (can be empty).
} ULGPUTextureOp;

///
/// A render buffer operation.
///
typedef struct {
  unsigned int id;             ///< The render buffer.
  ULGPUResourceAction action;  ///< What to do (Create or Destroy).
  ULRenderBuffer desc;         ///< The render buffer description (Create).
} ULGPURenderBufferOp;

///
/// A geometry operation.
///
/// Create: allocate `vertex_capacity` and `index_capacity` bytes, then write the data as Update
/// does.
///
/// Create and Update: write `vertices` at byte `vertex_offset` and `indices` at byte
/// `index_offset`. Bytes not written keep their previous contents. The frame's commands only use
/// bytes written by this frame or an earlier one.
///
typedef struct {
  unsigned int id;              ///< The geometry.
  ULGPUResourceAction action;   ///< What to do.
  ULVertexBufferFormat format;  ///< The vertex format (Create).
  unsigned int vertex_capacity; ///< The vertex buffer size, in bytes (Create).
  unsigned int index_capacity;  ///< The index buffer size, in bytes (Create).
  unsigned int vertex_offset;   ///< Where to write `vertices`, in bytes.
  unsigned int index_offset;    ///< Where to write `indices`, in bytes.
  ULGPUMemory vertices;         ///< The vertex data to write (can be empty).
  ULGPUMemory indices;          ///< The index data to write (can be empty).
} ULGPUGeometryOp;

///
/// A request to copy pixels out of a render target.
///
/// Copy `rect` of the texture into `destination`, with row 0 being the top row of `rect`, then
/// call ulGPUDriverAsyncCompleteReadback() with `request_id` once the bytes are there.
///
/// If `destination` has kULGPUMemoryUsage_GPUWrite, your GPU can copy into it directly;
/// otherwise copy through your own staging memory.
///
typedef struct {
  unsigned int request_id;  ///< Pass this to ulGPUDriverAsyncCompleteReadback() (never reused).
  unsigned int texture_id;  ///< The render target's texture.
  ULIntRect rect;           ///< The region to copy, in texture pixels.
  ULBitmapFormat format;    ///< The pixel format to write (kBitmapFormat_BGRA8_UNORM_SRGB).
  size_t row_pitch;         ///< The bytes between rows in `destination`.
  ULGPUMemory destination;  ///< Where to write the pixels.
} ULGPUReadbackOp;

/******************************************************************************
 * Driver
 *****************************************************************************/

///
/// Callback for when the driver queues a frame (called on the Renderer's thread).
///
/// You should signal your render thread here.
///
/// @param  user_data  The ULGPUDriverAsyncOptions::user_data you provided.
///
/// @param  frame_id   The ID of the frame that was queued.
///
typedef void (*ULGPUDriverAsyncFrameQueuedCallback)(void* user_data, unsigned long long frame_id);

///
/// Callback for when ulGPUDriverAsyncReadbackTexture() is about to wait for your render thread
/// (called on the Renderer's thread).
///
/// If you move frames into your own queue, or execute them on the Renderer's thread, hand off and
/// flush any pending frames here so the readback can complete.
///
/// @param  user_data  The ULGPUDriverAsyncOptions::user_data you provided.
///
typedef void (*ULGPUDriverAsyncReadbackWaitCallback)(void* user_data);

///
/// Options for ulCreateGPUDriverAsync().
///
/// Zero-initialize the struct and set struct_size to sizeof(ULGPUDriverAsyncOptions); a zeroed
/// field means the default. Fields may be appended in later versions, so the library uses the
/// size to know which of them you set.
///
typedef struct {
  unsigned int struct_size;  ///< Set to sizeof(ULGPUDriverAsyncOptions).

  ///
  /// The capabilities your render thread supports (see ULGPUDeviceCaps). The library reads them
  /// once, so they're fixed for the driver's lifetime.
  ///
  ULGPUDeviceCaps caps;

  ///
  /// Whether or not your render thread serves the readbacks in each frame (Default = False).
  /// While false, ulGPUDriverAsyncReadbackTexture() returns false.
  ///
  bool supports_readback;

  ///
  /// The number of frames that may be queued or held by you at once before
  /// ulGPUDriverAsyncIsReadyForFrame() returns false (Default = 2).
  ///
  unsigned int max_frames_in_flight;

  ///
  /// The memory, in bytes, that frames may hold before ulGPUDriverAsyncIsReadyForFrame() returns
  /// false (Default = 0, no limit).
  ///
  size_t memory_budget;

  ///
  /// How long ulGPUDriverAsyncReadbackTexture() waits for your render thread, in milliseconds
  /// (Default = 1000). Pass UL_GPU_DRIVER_ASYNC_NO_TIMEOUT to wait until it completes.
  ///
  unsigned int readback_timeout_ms;

  ///
  /// Your memory allocator (can be NULL to use the driver's own CPU memory). The struct is
  /// copied, but its user data must stay valid until the driver is destroyed.
  ///
  const ULGPUAllocator* allocator;

  void* user_data;  ///< Passed to the callbacks below (can be NULL).

  ULGPUDriverAsyncFrameQueuedCallback on_frame_queued;    ///< Can be NULL.
  ULGPUDriverAsyncReadbackWaitCallback on_readback_wait;  ///< Can be NULL.
} ULGPUDriverAsyncOptions;

///
/// Create an async GPU driver.
///
/// @param  options  The driver options. You can pass NULL to use default options. If its
///                  struct_size is 0, the library logs a warning and uses default options.
///
/// @return  Returns a new ULGPUDriverAsync instance. You must call ulDestroyGPUDriverAsync() when
///          finished.
///
ACExport ULGPUDriverAsync ulCreateGPUDriverAsync(const ULGPUDriverAsyncOptions* options);

///
/// Destroy a driver created by ulCreateGPUDriverAsync().
///
/// If the driver is registered with the Platform, this unregisters it. Frames still queued are
/// destroyed.
///
/// @param  driver  The driver to destroy.
///
/// @warning  Destroy the Renderer, and every frame you still hold, before calling this.
///
ACExport void ulDestroyGPUDriverAsync(ULGPUDriverAsync driver);

///
/// Set the driver as the Platform's GPU driver (the library renders Views on the GPU through it).
///
/// You should call this before ulCreateRenderer(). The driver must stay alive until the Renderer
/// is destroyed.
///
/// @param  driver  The driver to use.
///
/// @note  This replaces any driver set with ulPlatformSetGPUDriver().
///
ACExport void ulPlatformSetGPUDriverAsync(ULGPUDriverAsync driver);

///
/// Take the oldest queued frame, if any.
///
/// @param  driver  The driver.
///
/// @return  Returns the frame, or NULL if none is queued. You must call ulDestroyGPUFrame() when
///          finished.
///
/// @note  Call this from one thread at a time (your render thread). It never blocks.
///
ACExport ULGPUFrame ulGPUDriverAsyncTryDequeue(ULGPUDriverAsync driver);

///
/// Report that your render thread has finished a readback (see ULGPUReadbackOp).
///
/// Call this once the bytes are in the readback's destination (after your GPU work finished).
/// Unknown or late request IDs are ignored.
///
/// @param  driver      The driver.
///
/// @param  request_id  The readback's request_id.
///
/// @param  success     Whether or not the pixels were written. Pass false on any failure.
///
/// @note  Safe to call from any thread.
///
ACExport void ulGPUDriverAsyncCompleteReadback(ULGPUDriverAsync driver, unsigned int request_id,
                                               bool success);

///
/// Whether or not you should call ulRender() now.
///
/// @param  driver  The driver.
///
/// @return  Returns true whenever no frame is in flight, and false while max_frames_in_flight
///          frames are in flight or frames hold more than memory_budget bytes.
///
/// @note  Safe to call from any thread.
///
ACExport bool ulGPUDriverAsyncIsReadyForFrame(ULGPUDriverAsync driver);

///
/// Get the number of frames queued or held by you (not yet destroyed).
///
/// @param  driver  The driver.
///
/// @return  Returns the number of frames in flight.
///
/// @note  Safe to call from any thread.
///
ACExport unsigned int ulGPUDriverAsyncGetFramesInFlight(ULGPUDriverAsync driver);

///
/// Get the number of frames waiting in the queue.
///
/// @param  driver  The driver.
///
/// @return  Returns the number of queued frames.
///
/// @note  Safe to call from any thread.
///
ACExport size_t ulGPUDriverAsyncGetNumQueuedFrames(ULGPUDriverAsync driver);

///
/// Get the ID of the most recently queued frame.
///
/// @param  driver  The driver.
///
/// @return  Returns the frame ID (Will be 0 if none).
///
/// @note  Safe to call from any thread.
///
ACExport unsigned long long ulGPUDriverAsyncGetLastQueuedFrameId(ULGPUDriverAsync driver);

///
/// Get the payload memory in use, in bytes.
///
/// This is the allocator blocks that frames still hold, plus the driver's own CPU memory that
/// isn't idle yet. Memory your allocator keeps while your GPU finishes with it isn't counted.
///
/// @param  driver  The driver.
///
/// @return  Returns the memory in use, in bytes.
///
/// @note  Safe to call from any thread.
///
ACExport size_t ulGPUDriverAsyncGetMemoryInUse(ULGPUDriverAsync driver);

///
/// Whether or not a readback is waiting for your render thread.
///
/// A render thread that limits how many frames it executes per tick should keep executing frames
/// while this is true.
///
/// @param  driver  The driver.
///
/// @return  Returns true if a readback is waiting.
///
/// @note  Safe to call from any thread.
///
ACExport bool ulGPUDriverAsyncIsReadbackPending(ULGPUDriverAsync driver);

///
/// Copy pixels out of a render target, after every frame already queued has executed.
///
/// This queues a frame with one readback (see ULGPUReadbackOp), then waits for your render
/// thread to complete it, up to the readback timeout.
///
/// @param  driver       The driver.
///
/// @param  texture_id   A render target's texture (eg, the texture_id of ulViewGetRenderTarget()).
///
/// @param  rect         The region to copy, in texture pixels. It must lie inside the texture.
///
/// @param  destination  A writable bitmap in kBitmapFormat_BGRA8_UNORM_SRGB, at least as large
///                      as `rect`. Row 0 receives the top row of `rect`. The pixels are the
///                      stored values (premultiplied alpha).
///
/// @return  Returns whether or not the pixels were copied. Returns false when supports_readback
///          is false, the arguments are invalid, your render thread reported a failure, or the
///          wait timed out. The bitmap's contents are unspecified after a failure.
///
/// @note  Call this on the Renderer's thread, outside ulRender().
///
ACExport bool ulGPUDriverAsyncReadbackTexture(ULGPUDriverAsync driver, unsigned int texture_id,
                                              ULIntRect rect, ULBitmap destination);

///
/// Free the driver's own CPU memory that frames are no longer using.
///
/// @param  driver  The driver.
///
/// @note  Call this on the Renderer's thread.
///
ACExport void ulGPUDriverAsyncPurgeMemory(ULGPUDriverAsync driver);

/******************************************************************************
 * Frame
 *****************************************************************************/

///
/// Destroy a frame taken with ulGPUDriverAsyncTryDequeue().
///
/// This releases the frame's memory, except ULBuffers you retained with ulCreateBufferRef().
///
/// @param  frame  The frame to destroy.
///
/// @note  Safe to call from any thread.
///
ACExport void ulDestroyGPUFrame(ULGPUFrame frame);

///
/// Get the frame's ID (1, 2, 3, ... for each driver).
///
/// @param  frame  The frame.
///
/// @return  Returns the frame ID.
///
ACExport unsigned long long ulGPUFrameGetId(ULGPUFrame frame);

///
/// Get the frame's texture changes. Apply them first.
///
/// @param  frame  The frame.
///
/// @param  count  Receives the number of entries.
///
/// @return  Returns the entries (NULL if there are none), owned by the frame and valid until
///          ulDestroyGPUFrame(). The ULBuffers in them are owned by the frame too: don't destroy
///          them (call ulCreateBufferRef() to keep one longer).
///
ACExport const ULGPUTextureOp* ulGPUFrameGetTextures(ULGPUFrame frame, unsigned int* count);

///
/// Get the frame's render buffer changes. Apply them after the texture changes.
///
/// @param  frame  The frame.
///
/// @param  count  Receives the number of entries.
///
/// @return  Returns the entries (NULL if there are none), owned by the frame and valid until
///          ulDestroyGPUFrame().
///
ACExport const ULGPURenderBufferOp* ulGPUFrameGetRenderBuffers(ULGPUFrame frame,
                                                                   unsigned int* count);

///
/// Get the frame's geometry changes. Apply them after the render buffer changes.
///
/// @param  frame  The frame.
///
/// @param  count  Receives the number of entries.
///
/// @return  Returns the entries (NULL if there are none), owned by the frame and valid until
///          ulDestroyGPUFrame(). The ULBuffers in them are owned by the frame too: don't destroy
///          them (call ulCreateBufferRef() to keep one longer).
///
ACExport const ULGPUGeometryOp* ulGPUFrameGetGeometry(ULGPUFrame frame, unsigned int* count);

///
/// Get the frame's commands. Execute them in order, after the geometry changes.
///
/// A resource the frame destroys is removed after its commands run, and the commands never use
/// it.
///
/// @param  frame  The frame.
///
/// @param  count  Receives the number of commands (can be 0).
///
/// @return  Returns the commands (NULL if there are none), owned by the frame and valid until
///          ulDestroyGPUFrame().
///
ACExport const ULCommand* ulGPUFrameGetCommands(ULGPUFrame frame, unsigned int* count);

///
/// Get the frame's readbacks. Serve them last, after the commands.
///
/// @param  frame  The frame.
///
/// @param  count  Receives the number of entries.
///
/// @return  Returns the entries (NULL if there are none), owned by the frame and valid until
///          ulDestroyGPUFrame(). The ULBuffers in them are owned by the frame too: don't destroy
///          them.
///
ACExport const ULGPUReadbackOp* ulGPUFrameGetReadbacks(ULGPUFrame frame, unsigned int* count);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // APPCORE_CAPI_GPUDRIVERASYNC_H
