///
/// Copyright (C) 2026 Ultralight, Inc. All rights reserved.
/// A license is required for commercial use. https://ultralig.ht
///
/// @file CAPI_JSValue.h
///
/// JavaScript context and value handles for interacting with page script in C.
///
/// `#include <Ultralight/CAPI/CAPI_JSValue.h>`
///
/// @note  This API is a preview and may still change after 2.0.
///
/// This header provides the core types and functions of the JavaScript C API. You'll use its
/// context and value handles to call page functions and exchange data with scripts running in a
/// View.
///
/// The rules documented in this header hold across every JavaScript C header in the library.
///
/// This example calls a function on the page and destroys every handle afterward:
///
/// ```c
/// void ShowGreeting(ULView view) {
///   ULJSContext ctx = ulViewGetJSContext(view);
///   ULJSValue global = ulJSContextGetGlobalObject(ctx);
///   ULJSValue fn = ulJSObjectGetProperty(global, "ShowMessage", NULL);
///   ULJSValue arg = ulCreateJSValueStringFromCString(ctx, "Howdy!");
///   ULJSValue exception = NULL;
///   ULJSValue result = ulJSFunctionCall(fn, NULL, &arg, 1, &exception);
///
///   if (exception)
///     ulDestroyJSValue(exception); /* the page threw */
///
///   ulDestroyJSValue(result);
///   ulDestroyJSValue(arg);
///   ulDestroyJSValue(fn);
///   ulDestroyJSValue(global);
///   ulDestroyJSContext(ctx);
/// }
/// ```
///
/// ## Contexts and Values
///
/// The JavaScript C API organizes script execution around these handle types:
///
/// - **A ULJSContext represents the JavaScript environment of one page in one frame.** Call
///   ulViewGetJSContext() to obtain the context for a View's main frame. Each page navigation
///   creates a new context, so you'll need to fetch a fresh handle after each navigation.
/// - **A ULJSValue represents a single live JavaScript value.** Holding a handle keeps the value
///   from being garbage-collected.
///
/// ## Handle Ownership and Destruction
///
/// Handle ownership determines who destroys a handle:
///
/// - **You destroy every handle a library function returns.** This includes each intermediate
///   handle in a chained property read.
/// - **The library destroys handles passed into your callbacks.** Arguments, `this_value`, and
///   `ctx` stay valid only during the callback (call ulCreateJSValueRef() or ulCreateJSContextRef()
///   to keep one longer).
/// - **You destroy handles you pass into library functions.** The library only borrows them during
///   the call.
/// - **The library destroys handles your callback returns or stores in `*exception`.**
///
/// ## Page Navigations and Handle Lifetimes
///
/// A handle never keeps its underlying page alive. When a page navigates away, its frame is
/// removed, or its View is destroyed, every handle from that page enters the gone state.
///
/// Calls on `NULL` or on a handle whose page is gone fail safely without crashing. You'll still
/// need to destroy those handles to prevent memory leaks.
///
/// To check whether a handle can still interact with its page, call ulJSValueIsAlive() or
/// ulJSContextIsAlive(). Both functions return `false` for `NULL` handles as well as handles whose
/// page is gone, so check whether a handle is `NULL` first if you need to distinguish between the
/// two states.
///
/// ## Type Conversions
///
/// Conversion functions like ulJSValueToNumber(), ulJSValueToString(), and ulJSValueToBoolean()
/// follow JavaScript's coercion rules rather than strict type checks. Converting an object can run
/// page script by calling its valueOf() or toString() method.
///
/// To enforce strict conversions without coercion, call ulJSValueGetType() first to check the
/// value's type (and ulJSValueGetObjectKind() to tell objects like a Map or a Date apart).
///
/// This function shows how to perform a strict type check before reading a number versus relying on
/// coercion:
///
/// ```c
/// void ReadLevel(ULJSValue player) {
///   /* player.level is the string "7" */
///   ULJSValue level = ulJSObjectGetProperty(player, "level", NULL);
///
///   double strict = 0;  /* stays 0 (not a number) */
///   if (ulJSValueGetType(level) == kULJSType_Number)
///     ulJSValueToNumber(level, &strict, NULL);
///
///   double loose = 0;
///   ulJSValueToNumber(level, &loose, NULL);  /* 7 (JavaScript's rules) */
///
///   ulDestroyJSValue(level);
/// }
/// ```
///
/// ## Error Handling
///
/// Functions that can encounter JavaScript exceptions accept a `ULJSValue* exception`
/// out-parameter. Pass `NULL` if you don't need exception details. When an operation throws, the
/// library populates the parameter with an owned `ULJSValue` handle that you must destroy.
///
/// Passing a `NULL` handle or calling a function on a handle whose page is gone leaves `exception`
/// untouched. Check whether the handle is alive to distinguish a missing page from a script
/// exception.
///
/// To inspect an error, pass the exception handle to ulJSValueGetErrorDetails(). It populates a
/// ULJSErrorDetails struct with the error's line and column numbers, along with owned string
/// handles for the message, source URL, and stack trace. Release the string handles with
/// ulJSErrorDetailsRelease().
///
/// For errors created by the library, inspect the `code` property on the error object to check what
/// went wrong without parsing the message text. Standard error codes are defined in
/// `<Ultralight/js/Error.h>`.
///
/// ## Threading Rules
///
/// You must call functions in this header on the Renderer's thread, unless a function's
/// documentation states otherwise.
///
/// You can perform these operations from any thread, including a finalizer thread:
///
/// - **Release handles using ulDestroyJSValue() or ulDestroyJSContext().**
/// - **Keep a reference using ulCreateJSValueRef() or ulCreateJSContextRef().** Callback arguments
///   are the exception (retain them during the callback on the Renderer's thread).
/// - **Check liveness using ulJSValueIsAlive() or ulJSContextIsAlive().** Off the Renderer's
///   thread, the result may already be out of date by the time your code acts on it.
///
/// ## Native Functions and User Data
///
/// Call ulCreateJSFunction() to create a JavaScript function that invokes a native
/// ULJSFunctionCallback.
///
/// You can pass application state in `user_data` along with a `destroy_user_data` cleanup callback.
/// The library runs `destroy_user_data` exactly once on the Renderer's thread when the function is
/// garbage-collected or if function creation fails. Inside `destroy_user_data`, you must not call
/// any library functions other than destroying handles.
///
/// @note  A value belongs to the page that created it and can't cross to another page unless both
///        pages are frames of the same top-level page sharing the exact same origin. Operations
///        that receive a value from an incompatible page fail with a TypeError whose code is
///        `ULJS_CROSS_CONTEXT` (see `<Ultralight/js/Value.h>`).
///
/// @see  ulViewGetJSContext(), ulJSValueGetErrorDetails(), `<Ultralight/js/Value.h>`,
///       `<Ultralight/js/Context.h>`
///
#ifndef ULTRALIGHT_CAPI_JSVALUE_H
#define ULTRALIGHT_CAPI_JSVALUE_H

#include <Ultralight/CAPI/CAPI_Defines.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Headers that reference these handles without depending on this file (eg, the ULDOM
// JavaScript-interop functions) repeat the typedefs under the same guard, keeping strict-C89
// consumers free of duplicate-typedef errors.
#ifndef ULJS_HANDLE_TYPES_DEFINED
#define ULJS_HANDLE_TYPES_DEFINED

///
/// Opaque handle to a JavaScript context (the script environment of the page in one frame).
///
/// @see  ulViewGetJSContext(), ulCreateJSContextRef(), ulDestroyJSContext()
///
typedef struct C_JSContext* ULJSContext;

///
/// Opaque handle to a JavaScript value.
///
/// While a handle exists, its value can't be garbage collected.
///
/// @see  ulCreateJSValueRef(), ulDestroyJSValue()
///
typedef struct C_JSValue* ULJSValue;

#endif  // ULJS_HANDLE_TYPES_DEFINED

///
/// The type of a JavaScript value.
///
/// The values stay the same across releases. Later versions may add types, so handle a value
/// you don't know (eg, with a `default` case).
///
typedef enum {
  kULJSType_Invalid = 0,  ///< A NULL handle or one whose page is gone.
  kULJSType_Undefined,
  kULJSType_Null,
  kULJSType_Boolean,
  kULJSType_Number,
  kULJSType_String,
  kULJSType_Symbol,
  kULJSType_Object,
  kULJSType_BigInt = 8,
  kULJSType_Count,  ///< The number of types (not a type).
} ULJSType;

///
/// Attribute flags for a new property (see ulJSObjectSetProperty(), ulJSObjectDefineProperty(),
/// and ulJSClassAddMethod()).
///
typedef enum {
  kULJSPropertyAttributes_None = 0,             ///< No flags (a normal property).
  kULJSPropertyAttributes_ReadOnly = 1 << 1,    ///< Script can't change its value.
  kULJSPropertyAttributes_DontEnum = 1 << 2,    ///< Hidden from for...in and Object.keys.
  kULJSPropertyAttributes_DontDelete = 1 << 3,  ///< It can't be deleted.
} ULJSPropertyAttributes;

///
/// The type of a JavaScript error, matching the standard error constructors.
///
typedef enum {
  kULJSErrorType_Error = 0,  ///< A plain Error.
  kULJSErrorType_TypeError,
  kULJSErrorType_RangeError,
  kULJSErrorType_SyntaxError,
  kULJSErrorType_ReferenceError,
} ULJSErrorType;

///
/// The kind of a JavaScript object (see ulJSValueGetObjectKind()).
///
/// The values stay the same across releases. Later versions may split new kinds out of
/// kULJSObjectKind_Other, so handle a kind you don't know (eg, with a `default` case).
///
/// Each kind agrees with the matching check where one exists: kULJSObjectKind_Array with
/// ulJSValueIsArray(), kULJSObjectKind_Promise with ulJSValueIsPromise(), and
/// kULJSObjectKind_Function with ulJSValueIsCallable() (minus proxies).
///
typedef enum {
  kULJSObjectKind_Invalid = 0,       ///< A NULL handle or one whose page is gone.
  kULJSObjectKind_None = 1,          ///< Not an object (a primitive like a number or string).
  kULJSObjectKind_Plain = 2,         ///< An ordinary object (eg, `{}`, `Object.create(proto)`, or
                                     ///< an instance of a class defined by page script).
  kULJSObjectKind_Array = 3,
  kULJSObjectKind_Function = 4,      ///< A callable object that isn't a Proxy (this includes a
                                     ///< native class constructor, which isn't a function by
                                     ///< ulJSValueIsFunction()).
  kULJSObjectKind_Proxy = 5,         ///< A Proxy, callable or not (its target isn't inspected).
  kULJSObjectKind_Date = 6,
  kULJSObjectKind_RegExp = 7,
  kULJSObjectKind_Map = 8,
  kULJSObjectKind_Set = 9,
  kULJSObjectKind_WeakMap = 10,
  kULJSObjectKind_WeakSet = 11,
  kULJSObjectKind_Promise = 12,
  kULJSObjectKind_Error = 13,        ///< An Error or one of its subclasses (eg, a TypeError).
  kULJSObjectKind_ArrayBuffer = 14,
  kULJSObjectKind_TypedArray = 15,   ///< Any typed array (eg, a Uint8Array or a Float32Array).
  kULJSObjectKind_DataView = 16,
  kULJSObjectKind_Other = 17,        ///< Any other object (eg, `window`, a DOM object, `Math`,
                                     ///< `arguments`, `new String("x")`, or an instance of a
                                     ///< native class).
  kULJSObjectKind_Count,             ///< The number of kinds (not a kind).
} ULJSObjectKind;

///
/// Flags for ulJSObjectGetOwnKeys(), combined with `|`.
///
/// Bits not defined here must be zero.
///
typedef enum {
  kULJSOwnKeys_IncludeNonEnumerable = 1 << 0,  ///< Also list non-enumerable keys (like
                                               ///< Object.getOwnPropertyNames()).
} ULJSOwnKeysFlags;

///
/// Opaque handle to a list of strings (see ulJSObjectGetOwnKeys()).
///
/// The list is a snapshot: it holds copies of the strings, so it doesn't change when the object
/// it came from does, and it stays valid after that object's page is gone.
///
/// @see  ulJSStringListGetLength(), ulJSStringListGetString(), ulDestroyJSStringList()
///
typedef struct C_JSStringList* ULJSStringList;

///
/// Callback invoked when JavaScript calls a function created with ulCreateJSFunction().
///
/// The callback can call back into JavaScript (eg, evaluate scripts or call functions).
///
/// @param  user_data   The user data passed to ulCreateJSFunction().
///
/// @param  ctx         The calling context (owned by the library, valid only during the callback;
///                     call ulCreateJSContextRef() to keep it).
///
/// @param  this_value  The call's `this` value (owned by the library, valid only during the
///                     callback).
///
/// @param  args        The arguments (owned by the library, valid only during the callback; call
///                     ulCreateJSValueRef() to keep one).
///
/// @param  argc        The number of entries in `args`.
///
/// @param  exception   To throw an exception into JavaScript, set `*exception` to a handle you own.
///                     The library takes ownership and ignores the return value.
///
/// @return  Return the function's result as a handle you own (the library takes ownership), or
///          NULL for `undefined`.
///
/// \parblock
/// @note  Invoked on the Renderer's thread while script is running.
/// \endparblock
///
/// \parblock
/// @note  A returned or thrown value whose page is gone reaches script as `undefined`. One from
///        another page (unless both pages are frames of the same top-level page with exactly the
///        same origin) makes the call throw a TypeError whose `code` is `"ULJS_CROSS_CONTEXT"`.
/// \endparblock
///
typedef ULJSValue (*ULJSFunctionCallback)(void* user_data, ULJSContext ctx, ULJSValue this_value,
                                          const ULJSValue* args, size_t argc,
                                          ULJSValue* exception);

/******************************************************************************
 * JSContext
 *****************************************************************************/

///
/// Create a new handle to the same context.
///
/// @param  ctx  The context handle to reference.
///
/// @return  Returns a new ULJSContext handle. You must call ulDestroyJSContext() when finished.
///
/// @note  Safe to call from any thread.
///
ULExport ULJSContext ulCreateJSContextRef(ULJSContext ctx);

///
/// Destroy a context handle (NULL-safe).
///
/// This doesn't affect the context itself.
///
/// @param  ctx  The context handle to destroy.
///
/// @note  Safe to call from any thread, including a managed runtime's finalizer thread.
///
ULExport void ulDestroyJSContext(ULJSContext ctx);

///
/// Whether or not the context is still alive.
///
/// A context goes away when its page navigates away, its frame is removed, or its View or Renderer
/// is destroyed.
///
/// @param  ctx  The context handle.
///
/// @return  Returns true if the context is valid (false for a NULL handle or one whose page is
///          gone).
///
/// @note  Safe to call from any thread, but off the Renderer's thread the result may already be
///        out of date when you act on it.
///
ULExport bool ulJSContextIsAlive(ULJSContext ctx);

///
/// Get the context's global object.
///
/// @param  ctx  The context handle.
///
/// @return  Returns a handle to the global object (NULL if the context is gone). You must call
///          ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulJSContextGetGlobalObject(ULJSContext ctx);

///
/// Run a script in the context.
///
/// @param  ctx         The context handle.
///
/// @param  script      The script.
///
/// @param  source_url  A URL for the script as a null-terminated UTF-8 string, used in error
///                     reports (may be NULL).
///
/// @param  exception   A pointer to a ULJSValue in which to store an exception, if any. Pass
///                     NULL if you don't care about exceptions. If set, you own the stored
///                     handle.
///
/// @return  Returns the script's completion value (you must call ulDestroyJSValue() when
///          finished), or NULL if the script throws or the context is gone.
///
ULExport ULJSValue ulJSContextEvaluate(ULJSContext ctx, ULString script, const char* source_url,
                                       ULJSValue* exception);

///
/// Get the JavaScriptCore JSContextRef, for use with the JavaScriptCore C API.
///
/// @param  ctx  The context handle.
///
/// @return  Returns the JSContextRef (NULL if the context is gone).
///
/// @warning  Use it only on the Renderer's thread, and don't store it.
///
ULExport JSContextRef ulJSContextGetJSContextRef(ULJSContext ctx);

/******************************************************************************
 * JSValue creation
 *****************************************************************************/

///
/// Create a JavaScript `undefined` value.
///
/// @param  ctx  The context to create the value in.
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone). You must call
///          ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSValueUndefined(ULJSContext ctx);

///
/// Create a JavaScript `null` value.
///
/// @param  ctx  The context to create the value in.
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone). You must call
///          ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSValueNull(ULJSContext ctx);

///
/// Create a JavaScript boolean value.
///
/// @param  ctx    The context to create the value in.
///
/// @param  value  The boolean to store.
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone). You must call
///          ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSValueBoolean(ULJSContext ctx, bool value);

///
/// Create a JavaScript number value.
///
/// @param  ctx    The context to create the value in.
///
/// @param  value  The number to store.
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone). You must call
///          ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSValueNumber(ULJSContext ctx, double value);

///
/// Create a JavaScript string value from a ULString.
///
/// @param  ctx    The context to create the value in.
///
/// @param  value  The string to store.
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone or `value` is NULL).
///          You must call ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSValueString(ULJSContext ctx, ULString value);

///
/// Create a JavaScript string value from a null-terminated UTF-8 string.
///
/// @param  ctx    The context to create the value in.
///
/// @param  value  The null-terminated UTF-8 string to store.
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone or `value` is NULL).
///          You must call ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSValueStringFromCString(ULJSContext ctx, const char* value);

///
/// Create a JavaScript string value from UTF-8 bytes with an explicit length.
///
/// The bytes don't need a null terminator and may contain null characters. Invalid UTF-8
/// becomes U+FFFD.
///
/// @param  ctx     The context to create the value in.
///
/// @param  bytes   The UTF-8 bytes (may be NULL when `length` is 0).
///
/// @param  length  The number of bytes.
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone). You must call
///          ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSValueStringUTF8(ULJSContext ctx, const char* bytes, size_t length);

///
/// Create a JavaScript value by parsing JSON text.
///
/// @param  ctx        The context to create the value in.
///
/// @param  json       The JSON text.
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns a new ULJSValue handle, or NULL if the JSON is invalid (a SyntaxError is
///          stored in `exception`) or the context is gone. You must call ulDestroyJSValue() when
///          finished.
///
ULExport ULJSValue ulCreateJSValueFromJSON(ULJSContext ctx, ULString json, ULJSValue* exception);

///
/// Create a JavaScript error object of the given type.
///
/// Use this to throw from a callback (through its `exception` out-parameter) or to reject a
/// promise.
///
/// @param  ctx      The context to create the error in.
///
/// @param  type     The error type (TypeError, RangeError, and so on).
///
/// @param  message  The error's message as a null-terminated UTF-8 string.
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone or `message` is NULL).
///          You must call ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSError(ULJSContext ctx, ULJSErrorType type, const char* message);

///
/// Create a JavaScript TypeError for a value that didn't convert to the type a binding
/// expected.
///
/// The message has the same form as the library's own conversion errors:
///
/// ```
/// myApp.fs.readFile: argument 2: expected string, got number
/// myApp.setConfig: argument 1 (volume): expected number, got 'loud'
/// myApp.volume: expected number, got null
/// ```
///
/// The error's `code` property is `"ULJS_BAD_ARG"`, so page script can check the kind of error
/// without parsing the message.
///
/// @param  ctx           The context to create the error in.
///
/// @param  binding_path  The binding's full dot-separated path, root included (eg,
///                       "myApp.fs.readFile").
///
/// @param  arg_number    The 1-based position of the argument that failed, or 0 when the
///                       failure isn't tied to an argument (eg, a property assignment).
///
/// @param  value_path    Where the failing value is inside the argument (eg, "volume" or
///                       "items[2].name"), or NULL when the whole argument failed.
///
/// @param  expected      A short description of the expected type (eg, "string", "number[]",
///                       or "'Small' | 'Large'").
///
/// @param  got           The value that failed to convert (NULL reads as `undefined`). It
///                       remains yours. A string is quoted in the message (shortened if it's
///                       long); any other value is described by its type.
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone or a required parameter
///          is NULL). You must call ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSMarshalError(ULJSContext ctx, const char* binding_path,
                                          size_t arg_number, const char* value_path,
                                          const char* expected, ULJSValue got);

///
/// Create a new handle to the same JavaScript value.
///
/// Use this to keep a value you received in a callback after the callback returns.
///
/// @param  value  The value handle to reference.
///
/// @return  Returns a new ULJSValue handle. You must call ulDestroyJSValue() when finished.
///
/// @note  Safe to call from any thread, except for a handle passed to your callback: call it
///        during that callback (on the Renderer's thread).
///
ULExport ULJSValue ulCreateJSValueRef(ULJSValue value);

///
/// Destroy a value handle (NULL-safe).
///
/// @param  value  The value handle to destroy.
///
/// @note  Safe to call from any thread, including a managed runtime's finalizer thread.
///
ULExport void ulDestroyJSValue(ULJSValue value);

/******************************************************************************
 * JSValue queries
 *****************************************************************************/

///
/// Get the type of a JavaScript value.
///
/// @param  value  The value handle.
///
/// @return  Returns the value's type (kULJSType_Invalid for a NULL handle or one whose page is
///          gone).
///
ULExport ULJSType ulJSValueGetType(ULJSValue value);

///
/// Whether or not the value is an Array.
///
/// @param  value  The value handle.
///
/// @return  Returns true for an Array (false for a NULL handle or one whose page is gone).
///
ULExport bool ulJSValueIsArray(ULJSValue value);

///
/// Whether or not the value can be called (see ulJSFunctionCall()).
///
/// @param  value  The value handle.
///
/// @return  Returns true if the value can be called (false for a NULL handle or one whose page
///          is gone).
///
ULExport bool ulJSValueIsCallable(ULJSValue value);

///
/// Whether or not the value is a function.
///
/// @param  value  The value handle.
///
/// @return  Returns true for a function (false for a NULL handle or one whose page is gone).
///
/// @note  A few callable objects aren't functions (eg, the constructor of a native class). Use
///        ulJSValueIsCallable() to check whether you can call a value.
///
ULExport bool ulJSValueIsFunction(ULJSValue value);

///
/// Whether or not the value is a Promise.
///
/// This has no side effects: the promise isn't marked as handled.
///
/// @param  value  The value handle.
///
/// @return  Returns true for a Promise (false for a NULL handle or one whose page is gone).
///
ULExport bool ulJSValueIsPromise(ULJSValue value);

///
/// Whether or not the value can be called with `new` (see ulJSFunctionConstruct()).
///
/// @param  value  The value handle.
///
/// @return  Returns true for a constructor (false for a NULL handle or one whose page is gone).
///
/// @note  Some callable values aren't constructors (eg, an arrow function or a class method).
///
ULExport bool ulJSValueIsConstructor(ULJSValue value);

///
/// Get the kind of a JavaScript object (eg, a Map, a Date, or a plain object).
///
/// This never runs script. A Proxy reports kULJSObjectKind_Proxy whatever its target is, and its
/// handler isn't called.
///
/// @param  value  The value handle.
///
/// @return  Returns the object's kind (kULJSObjectKind_None for a value that isn't an object, or
///          kULJSObjectKind_Invalid for a NULL handle or one whose page is gone).
///
ULExport ULJSObjectKind ulJSValueGetObjectKind(ULJSValue value);

///
/// Whether or not two values are strictly equal (`===`).
///
/// Strings compare by content, objects are equal only when they're the same object, NaN isn't
/// equal to itself, and +0 equals -0. This never runs script, and the values may come from
/// different pages.
///
/// @param  a  The first value handle.
///
/// @param  b  The second value handle.
///
/// @return  Returns true if `a === b` (false if either handle is NULL or its page is gone).
///
ULExport bool ulJSValueIsStrictEqual(ULJSValue a, ULJSValue b);

///
/// Whether or not the value's page is still alive.
///
/// @param  value  The value handle.
///
/// @return  Returns true if the handle is valid (false for a NULL handle or one whose page is
///          gone).
///
/// @note  Safe to call from any thread, but off the Renderer's thread the result may already
///        be out of date when you act on it.
///
ULExport bool ulJSValueIsAlive(ULJSValue value);

///
/// Log a JavaScript exception that your code received but never read.
///
/// When JavaScript diagnostics are on (see ulJSAPISetDiagnostics()), this logs the exception's
/// type, message, and stack to the Logger and, if the page is still alive, to the page's
/// console. Otherwise it does nothing. A language binding built on this API can call it when a
/// dropped result held an exception (the C++ js::Error does this in its destructor).
///
/// @param  exception  The exception (NULL for no report). The handle isn't consumed.
///
/// @note  Safe to call from any thread. Off the Renderer's thread the log line has no details
///        of the exception.
///
ULExport void ulJSValueReportUnobservedException(ULJSValue exception);

///
/// Get the context a value belongs to.
///
/// The context may already be gone (see ulJSContextIsAlive()).
///
/// @param  value  The value handle.
///
/// @return  Returns a new ULJSContext handle (NULL only for a NULL `value`). You must call
///          ulDestroyJSContext() when finished.
///
/// @note  Safe to call from any thread.
///
ULExport ULJSContext ulJSValueGetContext(ULJSValue value);

/******************************************************************************
 * JSValue conversion
 *****************************************************************************/

///
/// Convert to a boolean using JavaScript's rules (this never runs script).
///
/// @param  value   The value handle.
///
/// @param  result  Receives the boolean.
///
/// @return  Returns true on success (false for a NULL handle or one whose page is gone).
///
ULExport bool ulJSValueToBoolean(ULJSValue value, bool* result);

///
/// Convert to a number using JavaScript's rules.
///
/// @param  value      The value handle.
///
/// @param  result     Receives the number.
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns true on success. Returns false if the conversion throws (eg, a valueOf()
///          method throws or the value is a Symbol), or for a NULL handle or one whose page is
///          gone.
///
ULExport bool ulJSValueToNumber(ULJSValue value, double* result, ULJSValue* exception);

///
/// Convert to a string using JavaScript's rules.
///
/// @param  value      The value handle.
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns a string you must destroy with ulDestroyString(), or NULL if the
///          conversion throws or the handle is NULL or its page is gone.
///
ULExport ULString ulJSValueToString(ULJSValue value, ULJSValue* exception);

///
/// Convert to a string using JavaScript's rules and write it as UTF-8 into your buffer.
///
/// For a string value, a good pattern is a small stack buffer first, then a heap buffer of
/// `*out_length` bytes if the text didn't fit. For any other value, each call converts it again
/// (running any toString() method again), so convert once with ulJSValueToString() instead.
///
/// @param  value       The value handle.
///
/// @param  buffer      The destination buffer (may be NULL when `capacity` is 0, to query the
///                     length).
///
/// @param  capacity    The buffer size in bytes.
///
/// @param  out_length  Receives the full UTF-8 length in bytes even when it exceeds
///                     `capacity` (no null terminator is written or counted).
///
/// @param  exception   A pointer to a ULJSValue in which to store an exception, if any. Pass
///                     NULL if you don't care about exceptions. If set, you own the stored
///                     handle.
///
/// @return  Returns true if the whole string was written. Returns false if the buffer was too
///          small (check `*out_length`), the conversion throws, or the handle is NULL or its page
///          is gone.
///
/// @note  Null characters in the string are kept. Unpaired UTF-16 surrogates become U+FFFD.
///
ULExport bool ulJSValueGetUTF8(ULJSValue value, char* buffer, size_t capacity,
                               size_t* out_length, ULJSValue* exception);

///
/// Convert to JSON text like `JSON.stringify()` does.
///
/// @param  value      The value handle.
///
/// @param  indent     The number of spaces to indent each level by (0 for compact output, at
///                    most 10).
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns a string you must destroy with ulDestroyString(). Returns NULL if a
///          toJSON() method or a getter throws, if the value has no JSON form (`undefined`, a
///          function, or a Symbol; `exception` is left unchanged), or for a NULL handle or one
///          whose page is gone.
///
ULExport ULString ulJSValueToJSON(ULJSValue value, unsigned indent, ULJSValue* exception);

/******************************************************************************
 * JSObject
 *****************************************************************************/

///
/// Create a new empty JavaScript object.
///
/// @param  ctx  The context to create the object in.
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone). You must call
///          ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSObject(ULJSContext ctx);

///
/// Create a new JavaScript Array from a list of values.
///
/// @param  ctx       The context to create the array in.
///
/// @param  elements  The element handles (they remain yours; a NULL entry becomes
///                   `undefined`). May be NULL when `count` is 0.
///
/// @param  count     The number of entries in `elements`.
///
/// @return  Returns a new ULJSValue handle (NULL if the context or an element's page is gone).
///          You must call ulDestroyJSValue() when finished.
///
/// @note  An element from another page is refused unless both pages are frames of the same
///        top-level page with exactly the same origin. The call then returns NULL (and logs a
///        diagnostic).
///
ULExport ULJSValue ulCreateJSArray(ULJSContext ctx, const ULJSValue* elements, size_t count);

///
/// Create a new JavaScript object with a set of properties.
///
/// The properties are defined like an object literal's, so a name like `__proto__` becomes an
/// ordinary property instead of setting the prototype.
///
/// @param  ctx        The context to create the object in.
///
/// @param  names      The property names as null-terminated UTF-8 strings (a NULL entry makes
///                    the call fail and return NULL).
///
/// @param  values     The property values (they remain yours; a NULL entry becomes
///                    `undefined`).
///
/// @param  count      The number of entries in `names` and `values`.
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns a new ULJSValue handle (NULL on an exception or if the context or a value's
///          page is gone). You must call ulDestroyJSValue() when finished.
///
/// @note  A value from another page is refused unless both pages are frames of the same
///        top-level page with exactly the same origin. The call then stores a TypeError whose
///        `code` is `"ULJS_CROSS_CONTEXT"` in `exception` and returns NULL.
///
ULExport ULJSValue ulCreateJSObjectWithProperties(ULJSContext ctx, const char* const* names,
                                                  const ULJSValue* values, size_t count,
                                                  ULJSValue* exception);

///
/// Get a property of an object (own or inherited).
///
/// @param  object     The object handle.
///
/// @param  name       The property name as a null-terminated UTF-8 string.
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns the property value (`undefined` if there's no such property), or NULL if a
///          getter throws or the handle is NULL or its page is gone. You must call
///          ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulJSObjectGetProperty(ULJSValue object, const char* name,
                                         ULJSValue* exception);

///
/// Whether or not an object has a property (own or inherited).
///
/// Unlike ulJSObjectGetProperty(), this returns true for a property set to `undefined` and
/// false for a missing one. It doesn't run JavaScript getters, but on an instance of a native
/// class (see CAPI_JSClass.h) it calls the property's getter callback.
///
/// @param  object     The object handle.
///
/// @param  name       The property name as a null-terminated UTF-8 string.
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns whether the property exists. Returns false if the check throws (eg, in a
///          Proxy's `has` handler) or the handle is NULL or its page is gone.
///
ULExport bool ulJSObjectHasProperty(ULJSValue object, const char* name, ULJSValue* exception);

///
/// Get several properties of an object in one call.
///
/// @param  object      The object handle.
///
/// @param  names       The property names as null-terminated UTF-8 strings (a NULL entry makes
///                     the call fail).
///
/// @param  count       The number of entries in `names` and `out_values`.
///
/// @param  out_values  An array of `count` slots that receives the values (`undefined` for a
///                     missing property). On success you must destroy each one with
///                     ulDestroyJSValue(). On failure nothing is stored.
///
/// @param  exception   A pointer to a ULJSValue in which to store an exception, if any. Pass
///                     NULL if you don't care about exceptions. If set, you own the stored
///                     handle.
///
/// @return  Returns true on success. Returns false if a getter throws or the handle is NULL or
///          its page is gone.
///
ULExport bool ulJSObjectGetProperties(ULJSValue object, const char* const* names, size_t count,
                                      ULJSValue* out_values, ULJSValue* exception);

///
/// Set a property of an object.
///
/// @param  object      The object handle.
///
/// @param  name        The property name as a null-terminated UTF-8 string.
///
/// @param  value       The value to assign (it remains yours; NULL assigns `undefined`).
///
/// @param  attributes  ULJSPropertyAttributes flags combined with `|`. They only apply if no
///                     property with this name exists on the object or its prototype chain;
///                     otherwise the call is a plain assignment and they're ignored. Pass
///                     kULJSPropertyAttributes_None for a normal assignment, and use
///                     ulJSObjectDefineProperty() to set the attributes of a name in use.
///
/// @param  exception   A pointer to a ULJSValue in which to store an exception, if any. Pass
///                     NULL if you don't care about exceptions. If set, you own the stored
///                     handle.
///
/// @return  Returns true on success. Returns false if a setter throws or the handle is NULL or
///          its page is gone.
///
/// \parblock
/// @note  Like an assignment in non-strict JavaScript, a write the object refuses (a frozen
///        object, or a read-only property) is silently ignored and still returns true.
/// \endparblock
///
/// \parblock
/// @note  With non-zero `attributes`, the call first checks whether the name exists, which runs
///        the `has` handler of a Proxy on the object or its prototype chain.
/// \endparblock
///
/// \parblock
/// @note  A value from another page is refused unless both pages are frames of the same
///        top-level page with exactly the same origin. The call then stores a TypeError whose
///        `code` is `"ULJS_CROSS_CONTEXT"` in `exception` and returns false.
/// \endparblock
///
ULExport bool ulJSObjectSetProperty(ULJSValue object, const char* name, ULJSValue value,
                                    unsigned attributes, ULJSValue* exception);

///
/// Get a property by numeric index (see ulJSObjectGetProperty()).
///
/// @param  object     The object handle.
///
/// @param  index      The property index.
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns the property value (`undefined` if there's no such property), or NULL if a
///          getter throws or the handle is NULL or its page is gone. You must call
///          ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulJSObjectGetPropertyAtIndex(ULJSValue object, size_t index,
                                                ULJSValue* exception);

///
/// Set a property by numeric index (see ulJSObjectSetProperty()).
///
/// @param  object     The object handle.
///
/// @param  index      The property index.
///
/// @param  value      The value to assign (it remains yours; NULL assigns `undefined`).
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns true on success. Returns false if a setter throws or the handle is NULL or
///          its page is gone.
///
/// @note  A value from another page is refused unless both pages are frames of the same
///        top-level page with exactly the same origin. The call then stores a TypeError whose
///        `code` is `"ULJS_CROSS_CONTEXT"` in `exception` and returns false.
///
ULExport bool ulJSObjectSetPropertyAtIndex(ULJSValue object, size_t index, ULJSValue value,
                                           ULJSValue* exception);

///
/// Delete a property of an object (like the JavaScript `delete` operator).
///
/// This returns what `delete object[name]` returns in non-strict JavaScript: true when the
/// property was deleted or didn't exist, and false when the object refuses (eg, a
/// non-configurable property, a frozen object, or a binding on a namespace object attached
/// without kULJSAPIAttachFlags_Mutable).
///
/// @param  object     The object handle.
///
/// @param  name       The property name as a null-terminated UTF-8 string.
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns the result of the `delete` operator. Also returns false if the call throws
///          or the handle is NULL or its page is gone.
///
/// @note  A refusal returns false without storing an exception, unlike ulJSObjectSetProperty(),
///        which returns true for a write the object ignores. Only a Proxy's `deleteProperty`
///        handler, a security check (eg, on a window from another origin), or a `null` or
///        `undefined` object throws.
///
ULExport bool ulJSObjectDeleteProperty(ULJSValue object, const char* name,
                                       ULJSValue* exception);

///
/// Delete a property by numeric index (see ulJSObjectDeleteProperty()).
///
/// Deleting an array element leaves a hole: the array's `length` doesn't change.
///
/// @param  object     The object handle.
///
/// @param  index      The property index.
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns the result of the `delete` operator. Also returns false if the call throws
///          or the handle is NULL or its page is gone.
///
ULExport bool ulJSObjectDeletePropertyAtIndex(ULJSValue object, size_t index,
                                              ULJSValue* exception);

///
/// Whether or not an object has an own property (not inherited).
///
/// This works like `Object.hasOwn()`: it returns true for an own property set to `undefined`. It
/// doesn't run JavaScript getters, but on an instance of a native class (see CAPI_JSClass.h) it
/// calls the property's getter callback, which can throw.
///
/// @param  object     The object handle.
///
/// @param  name       The property name as a null-terminated UTF-8 string (a name that isn't
///                    valid UTF-8 throws a TypeError).
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns whether the object has the own property. Returns false if the check throws
///          (eg, in a Proxy's `getOwnPropertyDescriptor` handler) or the handle is NULL or its
///          page is gone.
///
/// @note  On a window from another origin, the check throws a SecurityError for any name except
///        the few that work across origins and the window's child frames.
///
ULExport bool ulJSObjectHasOwnProperty(ULJSValue object, const char* name,
                                       ULJSValue* exception);

///
/// Get the names of an object's own properties without reading their values.
///
/// Getters never run. A Proxy's `ownKeys` handler runs once. The names come in the order
/// `Object.getOwnPropertyNames()` uses (integer keys in ascending order, then the other keys in
/// the order they were added), or in the order a Proxy's handler returns them. Inherited
/// properties and Symbol keys aren't listed.
///
/// @param  object     The object handle.
///
/// @param  flags      ULJSOwnKeysFlags combined with `|` (0 lists only enumerable keys, like
///                    `Object.keys()`). Other bits must be zero, or the call fails.
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns a new list (empty if there are no keys). You must call
///          ulDestroyJSStringList() when finished. Returns NULL if the call throws, `flags` has
///          an unknown bit (no exception is stored), or the handle is NULL or its page is gone.
///
/// @note  On a window from another origin, the list holds the window's child-frame indices,
///        plus the few properties that work across origins when you pass
///        kULJSOwnKeys_IncludeNonEnumerable.
///
ULExport ULJSStringList ulJSObjectGetOwnKeys(ULJSValue object, unsigned flags,
                                             ULJSValue* exception);

///
/// Destroy a string list.
///
/// @param  list  The list (can be NULL).
///
/// @note  Safe to call from any thread.
///
ULExport void ulDestroyJSStringList(ULJSStringList list);

///
/// Get the number of strings in a list.
///
/// @param  list  The list.
///
/// @return  Returns the number of strings.
///
/// @note  Safe to call from any thread.
///
ULExport size_t ulJSStringListGetLength(ULJSStringList list);

///
/// Get a string from a list.
///
/// @param  list   The list.
///
/// @param  index  The index of the string.
///
/// @return  Returns the string, or NULL if `index` is out of range. You must call
///          ulDestroyString() when finished.
///
/// @note  Safe to call from any thread.
///
ULExport ULString ulJSStringListGetString(ULJSStringList list, size_t index);

///
/// Define a data property of an object (like `Reflect.defineProperty()`).
///
/// Unlike ulJSObjectSetProperty(), this always gives the object an own property with exactly
/// the attributes you pass, even when the name is already in use, and setters never run.
/// Defining over an accessor property turns it into a data property.
///
/// @param  object      The object handle.
///
/// @param  name        The property name as a null-terminated UTF-8 string (a name that isn't
///                     valid UTF-8 throws a TypeError).
///
/// @param  value       The value (it remains yours; NULL defines `undefined`).
///
/// @param  attributes  ULJSPropertyAttributes flags combined with `|`. Each flag you leave out
///                     grants that ability, so kULJSPropertyAttributes_None defines a writable,
///                     enumerable, deletable property (the opposite of `Object.defineProperty()`,
///                     where a field you leave out defaults to false).
///
/// @param  exception   A pointer to a ULJSValue in which to store an exception, if any. Pass
///                     NULL if you don't care about exceptions. If set, you own the stored
///                     handle.
///
/// @return  Returns true if the property was defined. Returns false if the object refuses (eg,
///          redefining a non-configurable property, adding to a frozen object, or a name bound
///          by a native class instance), if the call throws, or if the handle is NULL or its page
///          is gone.
///
/// \parblock
/// @note  A refusal returns false without storing an exception. These throw: an `object` that
///        isn't an object (a TypeError, as with `Reflect.defineProperty()`), a Proxy handler that
///        throws or breaks a Proxy rule, a revoked Proxy, and a window from another origin (a
///        SecurityError).
/// \endparblock
///
/// \parblock
/// @note  A value from another page is refused unless both pages are frames of the same
///        top-level page with exactly the same origin. The call then stores a TypeError whose
///        `code` is `"ULJS_CROSS_CONTEXT"` in `exception` and returns false.
/// \endparblock
///
ULExport bool ulJSObjectDefineProperty(ULJSValue object, const char* name, ULJSValue value,
                                       unsigned attributes, ULJSValue* exception);

///
/// Define an accessor property of an object (like `Reflect.defineProperty()`).
///
/// Reads call `getter` and assignments call `setter`. Like ulJSObjectDefineProperty(), this
/// applies the attributes you pass even when the name is already in use, and leaving out a flag
/// grants that ability. kULJSPropertyAttributes_ReadOnly is ignored (leave out `setter` instead).
///
/// @param  object      The object handle.
///
/// @param  name        The property name as a null-terminated UTF-8 string (a name that isn't
///                     valid UTF-8 throws a TypeError).
///
/// @param  getter      The getter function (it remains yours), or NULL for none.
///
/// @param  setter      The setter function (it remains yours), or NULL for none.
///
/// @param  attributes  ULJSPropertyAttributes flags combined with `|`.
///
/// @param  exception   A pointer to a ULJSValue in which to store an exception, if any. Pass
///                     NULL if you don't care about exceptions. If set, you own the stored
///                     handle.
///
/// @return  Returns true if the property was defined. Returns false if the object refuses (eg,
///          redefining a non-configurable property, adding to a frozen object, or a name bound
///          by a native class instance), if the call throws, or if the handle is NULL or its page
///          is gone.
///
/// \parblock
/// @note  A NULL `getter` or `setter` leaves that half out of the definition. On a new property
///        the missing half is `undefined`, but redefining an accessor keeps its existing getter
///        or setter, so you can't remove a setter this way. Passing NULL for both throws a
///        TypeError, and so does a handle that can't be called (`undefined` included).
/// \endparblock
///
/// \parblock
/// @note  A refusal returns false without storing an exception. These throw: an `object` that
///        isn't an object (a TypeError, as with `Reflect.defineProperty()`), a Proxy handler that
///        throws or breaks a Proxy rule, a revoked Proxy, and a window from another origin (a
///        SecurityError).
/// \endparblock
///
/// \parblock
/// @note  A getter or setter from another page is refused unless both pages are frames of the
///        same top-level page with exactly the same origin. The call then stores a TypeError
///        whose `code` is `"ULJS_CROSS_CONTEXT"` in `exception` and returns false.
/// \endparblock
///
ULExport bool ulJSObjectDefineAccessor(ULJSValue object, const char* name, ULJSValue getter,
                                       ULJSValue setter, unsigned attributes,
                                       ULJSValue* exception);

///
/// Freeze an object (like `Object.freeze()`).
///
/// After this, nothing can add, delete, or change the object's properties: page script
/// assignments and ulJSObjectSetProperty() are silently ignored, and ulJSObjectDeleteProperty()
/// and ulJSObjectDefineProperty() return false.
///
/// @param  object     The object handle.
///
/// @param  exception  A pointer to a ULJSValue in which to store an exception, if any. Pass
///                    NULL if you don't care about exceptions. If set, you own the stored
///                    handle.
///
/// @return  Returns true if the object is frozen (also for one that already was, and for a
///          value that isn't an object, which is left as it is, as `Object.freeze()` does).
///          Returns false if the call throws or the handle is NULL or its page is gone.
///
/// @note  Unlike ulJSObjectDeleteProperty() and ulJSObjectDefineProperty(), which return false
///        for a refusal, this throws a TypeError when the object can't be frozen (eg, a Proxy that
///        refuses, `window`, a typed array with elements, or an instance of a native class).
///
ULExport bool ulJSObjectFreeze(ULJSValue object, ULJSValue* exception);

///
/// Get the names and values of an object's own enumerable properties in one call.
///
/// Inherited properties aren't included. Call it twice: first with `capacity` 0 to get the
/// count, then with buffers of that size. If the object gains properties between the calls (a
/// getter can run script), the second call reports a larger count, so repeat with more room.
///
/// @param  object      The object handle.
///
/// @param  out_names   An array of `capacity` slots that receives the property names. You must
///                     destroy each one with ulDestroyString().
///
/// @param  out_values  An array of `capacity` slots that receives the property values. You
///                     must destroy each one with ulDestroyJSValue().
///
/// @param  capacity    The number of slots in `out_names` and `out_values` (0 to query the
///                     count).
///
/// @param  out_count   Receives the total number of own enumerable properties even when it
///                     exceeds `capacity` (required).
///
/// @param  exception   A pointer to a ULJSValue in which to store an exception, if any. Pass
///                     NULL if you don't care about exceptions. If set, you own the stored
///                     handle.
///
/// @return  Returns true if the first min(capacity, *out_count) entries were stored. Returns
///          false (and stores nothing) if a getter throws, the handle is NULL or its page is
///          gone, or `out_count` is NULL.
///
/// @warning  A getter that changes the object during the call can cause entries to be skipped
///           or repeated.
///
ULExport bool ulJSObjectGetOwnEntries(ULJSValue object, ULString* out_names,
                                      ULJSValue* out_values, size_t capacity, size_t* out_count,
                                      ULJSValue* exception);

/******************************************************************************
 * JSFunction
 *****************************************************************************/

///
/// Call a JavaScript function.
///
/// @param  function    The function to call.
///
/// @param  this_value  The `this` value for the call (NULL uses the global object).
///
/// @param  args        The arguments (they remain yours; a NULL entry passes `undefined`).
///
/// @param  argc        The number of entries in `args`.
///
/// @param  exception   A pointer to a ULJSValue in which to store an exception, if any. Pass
///                     NULL if you don't care about exceptions. If set, you own the stored
///                     handle.
///
/// @return  Returns the function's return value (you must call ulDestroyJSValue() when
///          finished), or NULL if the call throws, `function` can't be called, or a handle is
///          NULL or its page is gone.
///
/// @note  A `this_value` or argument from another page is refused unless both pages are frames
///        of the same top-level page with exactly the same origin. The call then stores a
///        TypeError whose `code` is `"ULJS_CROSS_CONTEXT"` in `exception` and returns NULL.
///
ULExport ULJSValue ulJSFunctionCall(ULJSValue function, ULJSValue this_value,
                                    const ULJSValue* args, size_t argc, ULJSValue* exception);

///
/// Call a constructor with `new`.
///
/// This works like `new constructor(...args)` in JavaScript, so you can create built-in objects
/// (eg, a Map or a Date) and instances of classes defined by page script.
///
/// @param  constructor  The constructor to call.
///
/// @param  args         The arguments (they remain yours; a NULL entry passes `undefined`). May
///                      be NULL when `argc` is 0.
///
/// @param  argc         The number of entries in `args`.
///
/// @param  exception    A pointer to a ULJSValue in which to store an exception, if any. Pass
///                      NULL if you don't care about exceptions. If set, you own the stored
///                      handle.
///
/// @return  Returns the new object (you must call ulDestroyJSValue() when finished), or NULL if
///          the constructor throws, `constructor` can't be called with `new` (a TypeError is
///          stored in `exception`), or a handle is NULL or its page is gone.
///
/// @note  An argument from another page is refused unless both pages are frames of the same
///        top-level page with exactly the same origin. The call then stores a TypeError whose
///        `code` is `"ULJS_CROSS_CONTEXT"` in `exception` and returns NULL.
///
/// @see  ulJSValueIsConstructor()
///
ULExport ULJSValue ulJSFunctionConstruct(ULJSValue constructor, const ULJSValue* args,
                                         size_t argc, ULJSValue* exception);

///
/// Create a JavaScript function that calls a native callback.
///
/// The function is a real function object (it inherits from Function.prototype, so `bind`,
/// `call`, and `apply` work on it).
///
/// @param  ctx                The context to create the function in.
///
/// @param  name               The function's `name` as a null-terminated UTF-8 string (NULL for
///                            "anonymous").
///
/// @param  callback           The callback to invoke when the function is called.
///
/// @param  user_data          Opaque user data passed to every call of `callback`.
///
/// @param  destroy_user_data  Callback invoked exactly once to destroy `user_data` on the
///                            Renderer's thread after the function is garbage collected, or right
///                            away if the function can't be created (may be NULL).
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone or `callback` is NULL).
///          You must call ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSFunction(ULJSContext ctx, const char* name,
                                      ULJSFunctionCallback callback, void* user_data,
                                      ULUserDataDestroyCallback destroy_user_data);

/******************************************************************************
 * Dates
 *****************************************************************************/

///
/// Create a JavaScript Date.
///
/// The time is clipped the way `new Date(ms)` clips it: NaN, an infinity, or a magnitude above
/// 8.64e15 creates an Invalid Date, a fraction is dropped (rounded toward zero), and -0 becomes
/// +0.
///
/// @param  ctx             The context to create the Date in.
///
/// @param  ms_since_epoch  The time in milliseconds since 1970-01-01 00:00:00 UTC (the value
///                         `getTime()` returns).
///
/// @return  Returns a new ULJSValue handle (NULL if the context is gone). You must call
///          ulDestroyJSValue() when finished.
///
ULExport ULJSValue ulCreateJSDate(ULJSContext ctx, double ms_since_epoch);

///
/// Get the time value of a Date, in milliseconds since the epoch.
///
/// This reads the time stored in the Date and never runs script, so a `getTime()` or `valueOf()`
/// method the page replaced isn't called.
///
/// @param  date                The value handle.
///
/// @param  out_ms_since_epoch  Receives the time in milliseconds since 1970-01-01 00:00:00 UTC
///                             (NaN for an Invalid Date).
///
/// @return  Returns true for a Date, Invalid Dates included. Returns false if the value isn't a
///          Date, or for a NULL handle or one whose page is gone.
///
ULExport bool ulJSDateGetTime(ULJSValue date, double* out_ms_since_epoch);

/******************************************************************************
 * Error details
 *****************************************************************************/

///
/// Details of a JavaScript error, filled by ulJSValueGetErrorDetails().
///
/// Each string field is either NULL or a string you own. Call ulJSErrorDetailsRelease() to
/// destroy them all at once when you're finished.
///
/// @note  New fields are only ever added at the end.
///
typedef struct {
  ULJSErrorType type;  ///< The error's type (kULJSErrorType_Error for any other kind of
                       ///< error, and for a thrown value that isn't an error object).
  ULString message;    ///< The error's message (NULL if it has none).
  ULString source_url; ///< The URL of the script that created the error (NULL if unknown).
  unsigned int line;   ///< The 1-based line the error was created at (0 if unknown).
  unsigned int column; ///< The 1-based column the error was created at (0 if unknown).
  ULString stack;      ///< The error's stack trace (NULL if it has none).
} ULJSErrorDetails;

///
/// Get the type, message, source location, and stack trace of an error in one call.
///
/// Use this on an exception from any function with an `exception` out-parameter. A thrown value
/// that isn't an error object (eg, a thrown string) reports kULJSErrorType_Error, its string
/// form as the message, and no source location or stack.
///
/// @param  error    The error value.
///
/// @param  details  Receives the details. Every field is reset first, so it's safe to pass to
///                  ulJSErrorDetailsRelease() whatever this returns.
///
/// @return  Returns true on success. Returns false if `details` is NULL, or for a NULL handle
///          or one whose page is gone.
///
/// @note  Getting the message of a value that isn't an error object converts it to a string,
///        which can run script (its toString() method).
///
ULExport bool ulJSValueGetErrorDetails(ULJSValue error, ULJSErrorDetails* details);

///
/// Destroy the strings a ulJSValueGetErrorDetails() call stored in `details` and reset them to
/// NULL.
///
/// @param  details  The details to release (NULL does nothing).
///
/// @note  Safe to call from any thread.
///
ULExport void ulJSErrorDetailsRelease(ULJSErrorDetails* details);

/******************************************************************************
 * Typed array lengths
 *****************************************************************************/

///
/// Get the number of elements in a typed array. The ArrayBuffer can still be detached afterwards.
///
/// Unlike reading the array's `length` property, this never runs script.
///
/// @param  typed_array  The typed array.
///
/// @return  Returns the element count (0 for a detached typed array, a value that isn't a typed
///          array, or a NULL handle or one whose page is gone).
///
/// @see  ulJSTypedArrayGetInfo() in `<Ultralight/CAPI/CAPI_JSBuffer.h>`, which also gives you
///       the bytes (the ArrayBuffer cannot be detached afterwards).
///
ULExport size_t ulJSTypedArrayGetLength(ULJSValue typed_array);

///
/// Get the length of an ArrayBuffer in bytes. The ArrayBuffer can still be detached afterwards.
///
/// Unlike reading the ArrayBuffer's `byteLength` property, this never runs script.
///
/// @param  array_buffer  The ArrayBuffer.
///
/// @return  Returns the byte length (0 for a detached ArrayBuffer, a value that isn't an
///          ArrayBuffer, or a NULL handle or one whose page is gone).
///
ULExport size_t ulJSArrayBufferGetByteLength(ULJSValue array_buffer);

#ifdef __cplusplus
}
#endif

#endif  // ULTRALIGHT_CAPI_JSVALUE_H
