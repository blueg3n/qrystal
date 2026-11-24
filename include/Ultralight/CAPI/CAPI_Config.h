///
/// Copyright (C) 2026 Ultralight, Inc. All rights reserved.
/// A license is required for commercial use. https://ultralig.ht
///

///
/// @file CAPI_Config.h
///
/// Core configuration for the renderer.
///
/// `#include <Ultralight/CAPI/CAPI_Config.h>`
///
/// These are various configuration options that can be used to customize the behavior of the
/// library. These options can only be set once before creating the Renderer.
///
#ifndef ULTRALIGHT_CAPI_CONFIG_H
#define ULTRALIGHT_CAPI_CONFIG_H

#include <Ultralight/CAPI/CAPI_Defines.h>
#include <Ultralight/CAPI/CAPI_Logger.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Config
 *****************************************************************************/

///
/// How much checking and warning the library does for mistakes made through one of its APIs.
///
/// Warnings go to the Logger at kLogLevel_Warning, tagged with the API they came from (eg,
/// `[js]`, `[dom]`, `[data]`), and to the page's console when a page is involved.
///
/// @see  ulConfigSetJavaScriptDiagnostics(), ulConfigSetDOMDiagnostics()
///
typedef enum {
  ///
  /// No per-operation warnings (a mistake the library only finds once, like an invalid setup
  /// call, still warns).
  ///
  kULDiagnosticsLevel_Off = 0,

  ///
  /// Development warnings about each mistake as it happens.
  ///
  kULDiagnosticsLevel_Warn = 1,

  ///
  /// Everything kULDiagnosticsLevel_Warn reports, plus stricter checks (eg, a lossy argument
  /// conversion in a JavaScript binding becomes a TypeError, and every DOM call on a NULL handle
  /// warns).
  ///
  kULDiagnosticsLevel_Strict = 2,

  ///
  /// Warn when developer mode is on, Off when it's off (see ulConfigSetDeveloperMode()).
  ///
  kULDiagnosticsLevel_Auto = 3,
} ULDiagnosticsLevel;

///
/// How the library renders pixel fonts.
///
/// A pixel font is a monochrome bitmap font, or an outline font the library detects as drawn
/// on a pixel grid. Color fonts are never pixel fonts.
///
/// Pages can override this per element with the `-ul-font-rendering` CSS property. The value
/// `auto` follows this setting, while `smooth` and `pixelated` take precedence over it.
///
/// @note  Using `pixelated` on a monochrome font that isn't a pixel font draws it aliased (hard
///        edges, no antialiasing) on whole pixels. Color fonts ignore the property.
///
/// @see  ulConfigSetPixelFontRendering()
///
typedef enum {
  ///
  /// Render pixel fonts like other text-- edges stay antialiased with fractional glyph placement.
  /// Bitmap glyphs scale smoothly to the font size.
  ///
  kPixelFontRendering_Smooth = 0,

  ///
  /// Keep pixel fonts crisp-- hard pixel edges with every glyph on a whole device pixel.
  ///
  /// At any size, the font's pixels are enlarged by repeating device pixels. By default the used
  /// size moves to a whole multiple of the font's design size (see
  /// ulConfigSetSnapPixelFontSize()); with that off, at a size that isn't a whole multiple, some
  /// font pixels draw one device pixel wider than others.
  ///
  /// Glyph advances, letter-spacing and word-spacing round to whole device pixels. Because of
  /// this, a line's width at fractional device scales can differ slightly from other browsers.
  ///
  /// @note  An outline pixel font below its design size renders like other text.
  ///
  kPixelFontRendering_Pixelated = 1,
} ULPixelFontRendering;

///
/// Create config with default values (see <Ultralight/platform/Config.h>).
///
/// @return  Returns a new ULConfig instance. You must call ulDestroyConfig() when finished.
///
ULExport ULConfig ulCreateConfig(void);

///
/// Destroy a ULConfig instance created by ulCreateConfig().
///
/// @param  config  The config to destroy (can be NULL).
///
ULExport void ulDestroyConfig(ULConfig config);

///
/// A writable OS file path to store persistent Session data in.
///
/// This data may include cookies, cached network resources, indexed DB, etc.
///
/// @note  Files are only written to the path when using a persistent Session.
///
ULExport void ulConfigSetCachePath(ULConfig config, ULString cache_path);

///
/// The relative path to the resources folder (loaded via the FileSystem API).
///
/// The library loads certain resources (SSL certs, ICU data, etc.) from the FileSystem API
/// during runtime (eg, `file:///resources/cacert.pem`).
///
/// You can customize the relative file path to the resources folder by modifying this setting.
///
/// (Default = "resources/")
///
ULExport void ulConfigSetResourcePathPrefix(ULConfig config, ULString resource_path_prefix);

///
/// The winding order for front-facing triangles.
///
/// @pre  Only used when GPU rendering is enabled for the View.
///
/// (Default = kFaceWinding_CounterClockwise)
///
ULExport void ulConfigSetFaceWinding(ULConfig config, ULFaceWinding winding);

///
/// The hinting algorithm to use when rendering fonts. (Default = kFontHinting_Normal)
///
/// Leave this at kFontHinting_Normal to take the hinting from the font profile (see
/// ulConfigSetFontProfile()). Any other value overrides the profile.
///
/// When analytic text is enabled (the default, see ulConfigSetEnablePhotonText()), hinting
/// applies only to upright text (no rotation or skew) smaller than the analytic text size cutoff
/// (see ulConfigSetPhotonTextMinPx()). Larger or rotated text renders from unhinted outlines.
/// When analytic text is disabled, hinting applies to upright text of every size.
///
/// @note  Pixel fonts rendered as pixel art (kPixelFontRendering_Pixelated) ignore this setting.
///
/// @see  ULFontHinting
///
ULExport void ulConfigSetFontHinting(ULConfig config, ULFontHinting font_hinting);

///
/// The font-appearance preset providing default values for the text rendering knobs (font
/// gamma, font contrast, font stem darkening, font embolden). Knobs you set explicitly override
/// the preset's values. (Default = kFontProfile_Default)
///
/// @see  ULFontProfile
///
ULExport void ulConfigSetFontProfile(ULConfig config, ULFontProfile font_profile);

///
/// Set how antialiased glyph edges are blended into the page (Default = kFontBlendMode_Profile,
/// meaning the font profile's value).
///
/// @see  ULFontBlendMode
///
ULExport void ulConfigSetFontBlendMode(ULConfig config, ULFontBlendMode font_blend_mode);

///
/// The gamma to use when compositing font glyphs, change this value to adjust contrast (Adobe and
/// Apple prefer 1.8, others may prefer 2.2). Set to 0 to use the value from the font profile.
/// (Default = 0, meaning the profile's value; the default profile uses 2.2)
///
ULExport void ulConfigSetFontGamma(ULConfig config, double font_gamma);

///
/// Additional contrast enhancement applied when compositing font glyphs. Values above 0 thicken
/// the midtones of antialiased glyph edges, increasing apparent text weight at every size. The
/// useful range is 0.0 (none) to about 1.0. Set below 0 to use the value from the font profile.
/// (Default = -1.0, meaning the profile's value; the default profile uses 0.3)
///
ULExport void ulConfigSetFontContrast(ULConfig config, double font_contrast);

///
/// Stem-darkening strength for rendered text. Boosts the coverage of antialiased glyph pixels
/// so thin stems keep their apparent weight on light backgrounds; each preset shapes how the
/// boost varies with glyph size. Text on dark backgrounds is unaffected. Set below 0 to use the
/// value from the font profile. (Default = -1.0, meaning the profile's value)
///
ULExport void ulConfigSetFontStemDarkening(ULConfig config, double font_stem_darkening);

///
/// Extra stroke weight applied to glyph shapes, as a fraction of the font size (for example,
/// 0.01 widens stems by roughly 1% of the em). Applies uniformly at every size and on both
/// light and dark backgrounds; glyph advances and layout are unaffected. Set below 0 to use the
/// value from the font profile. (Default = -1.0, meaning the profile's value)
///
/// @note  This does not apply to pixel fonts rendered as pixel art (see ULPixelFontRendering)--
///        they keep the weight they were drawn with.
///
ULExport void ulConfigSetFontEmbolden(ULConfig config, double font_embolden);

///
/// Set how pixel fonts are rendered. (Default = kPixelFontRendering_Pixelated)
///
/// @param  config  The config to modify.
///
/// @param  mode    The rendering mode for pixel fonts.
///
/// @see  ULPixelFontRendering
///
ULExport void ulConfigSetPixelFontRendering(ULConfig config, ULPixelFontRendering mode);

///
/// Set whether or not to adjust the used size of pixel fonts so each font pixel covers a whole
/// number of device pixels at the current device scale (see ULPixelFontRendering).
/// (Default = True)
///
/// The library chooses the closest size the font can draw without scaling. For bitmap fonts,
/// this means strike sizes and their whole multiples. For outline pixel fonts, it means the
/// design size and its whole multiples. Ties go to the larger size. A size below half the
/// smallest such size is left unchanged. This applies with either ULPixelFontRendering value;
/// text styled `-ul-font-rendering: smooth` keeps its size.
///
/// For an 8 px design at 1.25x:
/// ```
/// font-size: 16px  ->  20 device px  ->  24 device px (19.2px used)
/// font-size: 12px  ->  15 device px  ->  16 device px (12.8px used)
/// ```
///
/// The computed style (eg, `getComputedStyle()`, `em` units) keeps the size the page asked for.
///
/// Pass false to keep the size the page asked for. Pixel fonts then scale to that size, and at a
/// size that isn't a whole multiple of the font's design size some font pixels draw one device
/// pixel wider than others.
///
/// @param  config   The config to modify.
///
/// @param  enabled  Whether or not to snap pixel font sizes.
///
/// @note  Text set in a pixel font can grow or shrink with the device scale (and at a 1x scale
///        when the page asks for a size the font can't draw 1:1), so layout changes too. Fixed
///        line heights and boxes sized in `em` don't follow the text.
///
ULExport void ulConfigSetSnapPixelFontSize(ULConfig config, bool enabled);

///
/// Global user-defined CSS string (included before any CSS on the page).
///
/// You can use this to override default styles for various elements on the page.
///
/// @note  This is an actual string of CSS, not a file path.
///
ULExport void ulConfigSetUserStylesheet(ULConfig config, ULString css_string);

///
/// Whether or not to continuously repaint any Views, regardless if they are dirty.
///
/// This is mainly used to diagnose painting/shader issues and profile performance.
/// 
/// (Default = False)
///
ULExport void ulConfigSetForceRepaint(ULConfig config, bool enabled);

///
/// Whether or not GPU-accelerated Views render vector content (path fills and strokes)
/// analytically instead of tessellating it to triangles. Analytic rendering is
/// resolution-independent: edges stay crisp under any zoom or transform.
///
/// Path rendering on CPU-rendered Views is unaffected. Disabling this also disables analytic
/// text (see ulConfigSetEnablePhotonText).
///
/// (Default = True)
///
ULExport void ulConfigSetEnablePhoton(ULConfig config, bool enabled);

///
/// Whether or not text and color emoji also render analytically when analytic vector rendering
/// is enabled (see ulConfigSetPhotonTextMinPx for the size cutoff). Ignored when analytic vector
/// rendering is disabled.
///
/// This selects the text appearance for both renderers: CPU-rendered Views match the analytic
/// look (unhinted outlines, fractional glyph placement) so output stays near-identical across
/// renderers.
///
/// (Default = True)
///
ULExport void ulConfigSetEnablePhotonText(ULConfig config, bool enabled);

///
/// The minimum on-screen glyph size, in pixels, at which text renders analytically when analytic
/// text is enabled; smaller glyphs use the rasterized glyph atlas. Set to 0 to render all text
/// analytically. (Default = 16)
///
ULExport void ulConfigSetPhotonTextMinPx(ULConfig config, unsigned int min_px);

///
/// Additional delay (in seconds) on top of the engine's internal page-settled detection.
///
/// Increase this if you want a little more assurance that the page has fully settled (animations
/// finished, late-arriving network activity complete) before ulViewIsSettled() flips and the
/// page-settled callback fires, at the cost of additional wait time.
///
/// (Default = 0.1)
///
/// @see  ulViewSetPageSettledCallback()
///
ULExport void ulConfigSetPageSettleDelay(ULConfig config, double delay);

///
/// The number of consecutive idle samples the library requires before it declares a page settled.
///
/// Together with the settle tick this sets the shortest time a page can take to settle (cycles *
/// tick). Lower it to settle sooner, at the cost of tolerance for late layout or paint.
///
/// (Default = 5)
///
/// @see  ulConfigSetPageSettleTick()
///
ULExport void ulConfigSetPageSettleCycles(ULConfig config, unsigned int cycles);

///
/// How often (in seconds) the library samples a loading page for idleness. (Default = 0.006)
///
/// @see  ulConfigSetPageSettleCycles()
///
ULExport void ulConfigSetPageSettleTick(ULConfig config, double tick);

///
/// The interval (in seconds) at which the library automatically recycles internal caches and
/// reclaims memory.
///
/// The library performs this lightweight recycle for you from inside ulUpdate() / ulRender(). It
/// is cheap enough to be invisible at typical frame rates, and letting the library drive it on
/// this timer is the recommended approach.
///
/// Set to 0 to disable the automatic recycler entirely, for applications that prefer to call
/// ulRecycle() on their own schedule.
///
/// (Default = 0.5)
///
ULExport void ulConfigSetRecycleDelay(ULConfig config, double delay);

///
/// Whether the library automatically collects garbage and returns freed memory to the OS without
/// embedder involvement.
///
/// When a page builds up a large JavaScript heap and then goes quiet, that memory would otherwise
/// stay resident until the next burst of activity or an explicit ulRecycle() / ulPurgeMemory().
/// With this enabled, the library collects on its own and pushes the freed memory back to the OS,
/// so the process footprint drifts back down after heavy content.
///
/// @note  Reclamation only happens while ulUpdate() is being pumped. An application that stops
///        calling ulUpdate() while idle gets no automatic reclamation; call ulRecycle() /
///        ulPurgeMemory() explicitly in that case.
///
/// (Default = True)
///
ULExport void ulConfigSetIdleGCEnabled(ULConfig config, bool enabled);

///
/// How long (in seconds) a page must be idle, with no user input, before automatic idle
/// collection begins.
///
/// Lower values reclaim sooner after the user stops interacting; higher values wait longer before
/// assuming the page is idle. Only used when idle GC is enabled.
///
/// (Default = 2.0)
///
/// @see  ulConfigSetIdleGCEnabled()
///
ULExport void ulConfigSetIdleGCIdleTime(ULConfig config, double idle_time);

///
/// The size of the library's memory cache in bytes.
///
/// @note  You should increase this if you anticipate handling pages with large resources, Safari
///        typically uses 128+ MiB for its cache.
///
/// (Default = 256 * 1024 * 1024)
///
ULExport void ulConfigSetMemoryCacheSize(ULConfig config, unsigned int size);

///
/// The number of pages to keep in the cache. (Default: 0, none)
///
/// @note
/// \parblock
///
/// Safari typically caches about 5 pages and maintains an on-disk cache to support typical
/// web-browsing activities.
///
/// If you increase this, you should probably increase the memory cache size as well.
///
/// \endparblock
///
/// (Default = 0)
///
ULExport void ulConfigSetPageCacheSize(ULConfig config, unsigned int size);

///
/// Memory vs. performance tuning for the renderer's HTML / JavaScript engine heaps. See the
/// ULMemoryProfile enumerators for the effect of each profile.
///
/// Read once when the Renderer is created. (Default = kMemoryProfile_Balanced)
///
/// @see  ULMemoryProfile
///
ULExport void ulConfigSetMemoryProfile(ULConfig config, ULMemoryProfile memory_profile);

///
/// The system's physical RAM size in bytes.
///
/// JavaScriptCore tries to detect the system's physical RAM size to set reasonable allocation
/// limits. Set this to anything other than 0 to override the detected value. Size is in bytes.
///
/// This can be used to force JavaScriptCore to be more conservative with its allocation strategy
/// (at the cost of some performance).
///
ULExport void ulConfigSetOverrideRAMSize(ULConfig config, unsigned int size);

///
/// The minimum size of large VM heaps in JavaScriptCore.
///
/// Set this to a lower value to make these heaps start with a smaller initial value.
///
/// Set to 0 to derive this value from the renderer's memory profile.
///
/// (Default = 0)
///
ULExport void ulConfigSetMinLargeHeapSize(ULConfig config, unsigned int size);

///
/// The minimum size of small VM heaps in JavaScriptCore.
///
/// Set this to a lower value to make these heaps start with a smaller initial value.
///
/// (Default = 1 * 1024 * 1024)
/// 
ULExport void ulConfigSetMinSmallHeapSize(ULConfig config, unsigned int size);

///
/// A soft target for the total JavaScriptCore heap size, in bytes.
///
/// This is not a hard limit: the heap may grow beyond this target. When it does, the garbage
/// collector treats the heap as under memory pressure and reclaims more aggressively to bring it
/// back toward the target.
///
/// Set to 0 to derive this value from the memory profile.
///
/// (Default = 0)
///
/// @see  ulConfigSetMemoryProfile()
///
ULExport void ulConfigSetMaxHeapSize(ULConfig config, unsigned int size);

///
/// The number of threads to use in the Renderer (for parallel painting on the CPU, etc.).
///
/// You can set this to a certain number to limit the number of threads to spawn. Values above
/// the number of hardware threads are clamped to it.
///
/// Set to `0` (the default) to use 8 threads (or fewer on machines with fewer hardware threads).
///
ULExport void ulConfigSetNumRendererThreads(ULConfig config, unsigned int num_renderer_threads);

///
/// The max amount of time (in seconds) to allow repeating timers to run during each call to
/// Renderer::Update.
///
/// The library will attempt to throttle timers if this time budget is exceeded.
/// 
/// (Default = 1.0 / 200.0)
///
ULExport void ulConfigSetMaxUpdateTime(ULConfig config, double max_update_time);

///
/// The row alignment (in bytes) of the BitmapSurface when using the CPU renderer.
///
/// The underlying bitmap associated with each BitmapSurface will have row_bytes padded to reach
/// this alignment. This must be a power of two (or 0, see below).
///
/// Aligning the bitmap helps improve performance when using the CPU renderer. Determining the
/// proper value to use depends on the CPU architecture and max SIMD instruction set used.
///
/// We generally target the 128-bit SSE2 instruction set across most PC platforms so '16' is a safe
/// value to use.
///
/// You can set this to '0' to perform no padding (row_bytes will always be width * 4) at a slight
/// cost to performance.
///
/// (Default = 16)
///
ULExport void ulConfigSetBitmapRowAlignment(ULConfig config, unsigned int bitmap_row_alignment);

///
/// The quality of effects (blurs, CSS filters, SVG filters, etc.) to use when rendering.
///
/// (Default = kEffectQuality_Medium)
///
/// @see  ULEffectQuality
///
ULExport void ulConfigSetEffectQuality(ULConfig config, ULEffectQuality quality);

///
/// Whether to ignore SSL certificate errors when using the built-in network backend.
///
/// When set to true, SSL certificate verification will be disabled, allowing connections
/// to servers with self-signed or invalid certificates.
///
/// @warning  This reduces security and should never be enabled in production.
///
/// (Default = false)
///
ULExport void ulConfigSetIgnoreSSLErrors(ULConfig config, bool enabled);

///
/// Whether to validate TLS server certificates against the operating system's certificate
/// store instead of the certificate bundle shipped with the application (resources/cacert.pem).
///
/// When true, certificates trusted by the host operating system (including enterprise or
/// user-installed roots) are honored and the bundled list is not used.
///
/// Currently honored on Windows. On platforms where the OS trust store is not yet supported,
/// this option is ignored and the bundled list is used.
///
/// (Default = false)
///
ULExport void ulConfigSetUseSystemCertificateStore(ULConfig config, bool enabled);

///
/// Whether to paint full layers when dirty instead of just the dirty subregion.
///
/// This can be useful for debugging rendering issues or when dirty region tracking
/// is causing artifacts.
///
/// @note  Enabling this will impact performance as entire layers are redrawn.
///
/// (Default = false)
///
ULExport void ulConfigSetPaintFullLayers(ULConfig config, bool enabled);

///
/// The interval (in milliseconds) at which the library samples and emits resource counters
/// (memory usage, VRAM, cache sizes, etc.) via the Profiler's EmitCounter() callback.
///
/// Set to 0 to disable resource counter sampling entirely; scopes and events are unaffected.
///
/// (Default = 500)
///
ULExport void ulConfigSetResourceSampleInterval(ULConfig config, unsigned int interval_ms);

///
/// Set whether or not the library runs in developer mode (Default = false).
///
/// Developer mode turns the kULDiagnosticsLevel_Auto levels into kULDiagnosticsLevel_Warn, and
/// lets the diagnostics environment variables (`UL_JS_DIAGNOSTICS`, `UL_DOM_DIAGNOSTICS`) override
/// them. You should turn it on in your development builds and leave it off in the builds you ship
/// (a release library ignores those variables when developer mode is off, so a shipped app's
/// behavior can't be changed from its environment). Each variable takes `off`, `warn`, or
/// `strict`.
///
/// @param  config          The config.
///
/// @param  developer_mode  Whether or not to run in developer mode.
///
ULExport void ulConfigSetDeveloperMode(ULConfig config, bool developer_mode);

///
/// Set the diagnostics level for JavaScript APIs that leave their own level at the default
/// (Default = kULDiagnosticsLevel_Auto).
///
/// @param  config  The config.
///
/// @param  level   The diagnostics level.
///
ULExport void ulConfigSetJavaScriptDiagnostics(ULConfig config, ULDiagnosticsLevel level);

///
/// Set the diagnostics level for the DOM API and data bindings
/// (Default = kULDiagnosticsLevel_Auto).
///
/// @param  config  The config.
///
/// @param  level   The diagnostics level.
///
ULExport void ulConfigSetDOMDiagnostics(ULConfig config, ULDiagnosticsLevel level);

///
/// Set the least severe level the library sends to the Logger; less severe messages are dropped
/// (Default = kLogLevel_Info).
///
/// Diagnostics warnings are sent at kLogLevel_Warning, so a minimum of kLogLevel_Error or
/// kLogLevel_Fatal hides them.
///
/// @param  config  The config.
///
/// @param  level   The minimum log level.
///
ULExport void ulConfigSetMinLogLevel(ULConfig config, ULLogLevel level);

#if UL_HAS(MEDIA)
///
/// The memory-vs-performance profile for the HTML5 media subsystem.
///
/// Picks a tradeoff between peak memory per media element and playback smoothness / seek
/// responsiveness. The value is read once when each media element starts loading and stays fixed
/// for that element's lifetime; later changes apply only to subsequently-loaded elements.
///
/// (Default = kMediaProfile_Balanced)
///
/// @pre  Requires the Pro edition or higher.
///
/// @see  ULMediaProfile
///
ULExport void ulConfigSetMediaProfile(ULConfig config, ULMediaProfile media_profile);
#endif // UL_HAS(MEDIA)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_CONFIG_H