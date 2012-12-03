/*
 LuaCocoa
 Copyright (C) 2009-2011 PlayControl Software, LLC. 
 Eric Wing <ewing . public @ playcontrol.net>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 */

#ifndef LUA_COCOA_H
#define LUA_COCOA_H


#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/** @cond DOXYGEN_SHOULD_IGNORE_THIS */

/* Note: For Doxygen to produce clean output, you should set the 
 * PREDEFINED option to remove ALMIXER_DECLSPEC, ALMIXER_CALL, and
 * the DOXYGEN_SHOULD_IGNORE_THIS blocks.
 * PREDEFINED = DOXYGEN_SHOULD_IGNORE_THIS=1 ALMIXER_DECLSPEC= ALMIXER_CALL=
 */

#if defined(__cplusplus)
	#define LUACOCOA_EXTERN extern "C"
#else
	#define LUACOCOA_EXTERN extern
#endif

#if TARGET_OS_WIN32

	#if defined(NSBUILDINGLUACOCOA)
		#define LUACOCOA_EXPORT __declspec(dllexport)
	#else
		#define LUACOCOA_EXPORT __declspec(dllimport)
	#endif

	#define LUACOCOA_IMPORT __declspec(dllimport)

#else
	#if defined(__GNUC__) && __GNUC__ >= 4 && defined(NSBUILDINGLUACOCOA)
		#define LUACOCOA_EXPORT __attribute__ ((visibility("default")))
	#else
		#define LUACOCOA_EXPORT
	#endif
	
	#define LUACOCOA_IMPORT
#endif

/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */



#include <stdarg.h> /* va_list */
#include <stdbool.h>

#ifdef LUACOCOA_DONT_USE_BUNDLED_LUA_HEADERS
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
#else
	/* Since I compiled Lua with C-linkage for LuaCocoa, C++ users need the header guards since Lua doesn't
	 * specify them in their main headers.
	 */
	#ifdef __cplusplus
	extern "C" {
	#endif
		#include <LuaCocoa/lua.h>
		#include <LuaCocoa/lualib.h>
		#include <LuaCocoa/lauxlib.h>
	#ifdef __cplusplus
	}
	#endif
#endif /* end LUACOCOA_DONT_USE_BUNDLED_LUA_HEADERS */
/* Hide Obj-C stuff for people writing pure C (or C++) programs that just want access to the LuaCocoa C APIs */
#ifdef __OBJC__ 
#import <Foundation/Foundation.h>

// Forward declaration so I don't need to #include "lua.h" here. (I don't have to worry about the search path of Lua for the public header.)
// But I have to use the formal "struct" version of the name instead of the typedef, i.e. "struct lua_State" instead of just "lua_State"
struct lua_State;

// Not sure if I want to make part of the API
/*
@protocol LuaCocoaErrorDelegate <NSObject>
@optional
- (void) error:(NSString*)error_string luaState:(lua_State*)lua_state functionName:(NSString*)function_name fileName:(NSString*)file_name lineNumber:(NSInteger)line_number;
@end
*/

/**
 * @class LuaCocoa
 * LuaCocoa is the primary class you use to bridge your Lua code with Objective-C (and vice-versa).
 * LuaCocoa associates a lua_State with Objective-C so you may easily cross between languages.
 * Behind the scenes, LuaCocoa loads dependent frameworks and dylibs, parses XML, and uses the Obj-C runtime
 * to provide core bridging functions.
 */ 
LUACOCOA_EXPORT @interface LuaCocoa : NSObject
{
@private
	struct lua_State* luaState;
	bool ownsLuaState;
	NSMutableDictionary* frameworksLoaded;
	bool skipDLopen; // in case you are using with other language bridges
	bool disableImportFromLua; // consider for sandboxing
	NSThread* originThread; // because Lua/LuaCocoa are not completely thread-safe, I might need this.

	// Not sure if I want to make part of the API
//	lua_CFunction luaErrorFunction;
//	id<LuaCocoaErrorDelegate> errorDelegate;

}
/**
 * Returns the lua_State associated with the LuaCocoa instance.
 * Returns the lua_State associated with the LuaCocoa instance.
 * @return The lua_State
 */ 
@property(assign, readonly) struct lua_State* luaState;
@property(assign) bool skipDLopen;
@property(assign) bool disableImportFromLua;

// Not sure if I want to make part of the API
//@property(assign) lua_CFunction luaErrorFunction;
//@property(assign) id<LuaCocoaErrorDelegate> errorDelegate;

/**
 * Initializes a new LuaCocoa instance, complete with a new lua_State.
 * When creating a new LuaCocoa instance with this initializer, a new lua_State will be created to go along with it.
 * You should not try to replace the lua_State or directly close the lua_State behind the LuaCocoa object wrapper's back.
 */
- (id) init;

/**
 * Will initialize a new LuaCococa instance with the lua_State you provide.
 * @param lua_state The lua_State you want to bind the LuaCocoa instance to.
 * @param should_assume_ownership If true, LuaCocoa will manage cleanup of the lua_State.
 * If set to false, you are responsible for cleaning up the lua_State.
 * It must not be destroyed until after this instance is destroyed.
 */
- (id) initWithLuaState:(struct lua_State*)lua_state assumeOwnership:(bool)should_assume_ownership;

// Might be moved to private API
- (bool) loadFrameworkWithBaseName:(NSString*)base_name hintPath:(NSString*)hint_path searchHintPathFirst:(bool)search_hint_path_first skipDLopen:(bool)skip_dl_open;

// Might be moved to private API
- (bool) isFrameworkLoaded:(NSString*)base_name;

/**
 * Forces both the Lua and Objective-C garbage collectors to collect exhaustively.
 * This will first force Lua to garbage collect exhaustively, and then subsequently force 
 * Objective-C's garbage collector to collectExhaustively.
 * Under Objective-C garbage collection, it might be a good idea to call this before closing the lua_State
 * because there is a potential race condition/out-of-order clean-up bug where LuaCocoaProxyObjects might
 * still be pending to be cleaned up after the lua_State is closed. Since they are intimately connected to their
 * lua_States, their access will result in an illegal access of freed memory.
 * This function is safe to call under traditional Obj-C memory management. Lua will still be forced 
 * to garbage collect, but the Obj-C collector will be a no-op.
 * @param should_wait_until_done Set to true if you want the function to block until garbage collection is complete.
 * @note 10.7 bug. should_wait_until_done=true will cause the application to hang on quit.
 * My suspicion is that it is connected to subclassing in Lua because the SimpleLuaView examples both hang but 
 * the CoreAnimationScriptability example does not hang.
 * Filed rdar://10660280
 */
- (void) collectExhaustivelyWaitUntilDone:(bool)should_wait_until_done;

/**
 * Forces the Objective-C garbage collector to collect exhaustively.
 * This will tell Objective-C's garbage collector to collectExhaustively. 
 * This is a class version, so there is no Lua state to invoke garbage collection on.
 * This function is safe to call under traditional Obj-C memory management.
 * @param should_wait_until_done Set to true if you want the function to block until garbage collection is complete.
 * @see collectExhaustivelyWaitUntilDone:
 * @note 10.7 bug. should_wait_until_done=true will cause the application to hang on quit.
 * My suspicion is that it is connected to subclassing in Lua because the SimpleLuaView examples both hang but 
 * the CoreAnimationScriptability example does not hang.
 * Filed rdar://10660280
 */
+ (void) collectExhaustivelyWaitUntilDone:(bool)should_wait_until_done;

/** 
 * Convenience method to pcall functions in Lua.
 * This convenience method adopts the techniques described in Programming in Lua's call_va function to make
 * calling arbitrary Lua functions a little easier.
 * @warning Parameter signatures are subject to change in future releases to be more consistent with Obj-C. Sorry.
 * FIXME: Unify with Obj-C method signatures. Perhaps call actual LuaCocoa backend for parsing?
 * Adapted from Programming in Lua call_va
 * Example:
   @code
   [luaCocoa callFunction:@"OnMouseUp" withSignature:"@@idd>bids@@@@", 
		the_layer, layer_name, button_number, x_pos, y_pos,
  		&bool_val, &int_val, &double_val, &str_val, &ns_str_val, &layer_val, &array_val, &dict_val];
   @endcode
 * In this case, we pass in two Obj-C objects followed by an int and two doubles.
 * The > marker denotes the return types.
 * In this case, we expect the Lua function to return a boolean, int, double, const char* and 4 Obj-C objects.
 * The return objects go through the propertylist conversion, so in particular, Lua tables and arrays get passed back as 
 * NSDictionary and NSArray.
 * Notes: Since I pop the stack at the end of the function (so you don't have to worry about balancing), 
 * I can't return a char* because it could get deallocated before I am done. 
 * So, the easiest answer seems to be to convert to an NSString. So 's' tokens on the return side get copied
 * into NSString, so you must provide that type of variable.
 * Also, passing nil to Lua functions and  Lua functions returning nil are not supported well. (nil might work through the property list.)
 * Bugs: error handling isn't quite right. luaL_error isn't right because we haven't pcall'ed yet or have returned from pcall.
 * I don't think error: has all the correct information. I think I need to write a special error handler the case of
 * being outside pcall.
 * TODO: Figure out how to support optional return parameters.
 * @note In addition to Programming in Lua which uses 'b' (boolean) 'd' (double) 'i' (integer) 's' (string) '>'
 * I also add '@' for id, '#' for Class, and 'v' for void*.
 * @param lua_function_name The name of the Lua function to call.
 * @param parameter_signature A format string describing the parameter types. (See above notes.)
 * @param ... The list of parameters to go with the format string.
 * @return A NSString containing an error message if pcall failed, nil otherwise.
 */
- (NSString*) pcallLuaFunction:(const char*)lua_function_name withSignature:(const char*)parameter_signature, ...;
/** 
 * Convenience method to pcall functions in Lua and allows an error function to be specified.
 * This convenience method adopts the techniques described in Programming in Lua's call_va function to make
 * calling arbitrary Lua functions a little easier.
 * @warning Parameter signatures are subject to change in future releases to be more consistent with Obj-C. Sorry.
 * FIXME: Unify with Obj-C method signatures. Perhaps call actual LuaCocoa backend for parsing?
 * Adapted from Programming in Lua call_va
 * Example:
   @code
   [luaCocoa callFunction:@"OnMouseUp" withSignature:"@@idd>bids@@@@", 
	   the_layer, layer_name, button_number, x_pos, y_pos,
       &bool_val, &int_val, &double_val, &str_val, &ns_str_val, &layer_val, &array_val, &dict_val];
   @endcode
 * In this case, we pass in two Obj-C objects followed by an int and two doubles.
 * The > marker denotes the return types.
 * In this case, we expect the Lua function to return a boolean, int, double, const char* and 4 Obj-C objects.
 * The return objects go through the propertylist conversion, so in particular, Lua tables and arrays get passed back as 
 * NSDictionary and NSArray.
 * Notes: Since I pop the stack at the end of the function (so you don't have to worry about balancing), 
 * I can't return a char* because it could get deallocated before I am done. 
 * So, the easiest answer seems to be to convert to an NSString. So 's' tokens on the return side get copied
 * into NSString, so you must provide that type of variable.
 * Also, passing nil to Lua functions and  Lua functions returning nil are not supported well. (nil might work through the property list.)
 * Bugs: error handling isn't quite right. luaL_error isn't right because we haven't pcall'ed yet or have returned from pcall.
 * I don't think error: has all the correct information. I think I need to write a special error handler the case of
 * being outside pcall.
 * TODO: Figure out how to support optional return parameters.
 * @note In addition to Programming in Lua which uses 'b' (boolean) 'd' (double) 'i' (integer) 's' (string) '>'
 * I also add '@' for id, '#' for Class, and 'v' for void*.
 * @param lua_function_name The name of the Lua function to call.
 * @param error_function The error function to handle any Lua errors.
 * @param parameter_signature A format string describing the parameter types. (See above notes.)
 * @param ... The list of parameters to go with the format string.
 * @return A NSString containing an error message if pcall failed, nil otherwise.
 */
- (NSString*) pcallLuaFunction:(const char*)lua_function_name errorFunction:(lua_CFunction)error_function withSignature:(const char*)parameter_signature, ...;
// Not sure if I want to make an API or not
//- (void) error:(const char *)fmt, ...;

@end


//LUACOCOA_EXTERN LUACOCOA_EXPORT NSString* const kLuaCocoaErrorDomain /* = @"kLuaCocoaErrorDomain" */;

/**
 * Returns true if an instance of an Obj-C class.
 * Returns true if the object at the specified index is an instance of an Obj-C class.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return True if an Obj-C instance.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_IsInstance(struct lua_State* lua_state, int stack_index);
/**
 * Returns an Obj-C instance or triggers a Lua error.
 * Returns an Obj-C instance at the specified index or triggers a Lua error.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return The Obj-C instance.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT id LuaCocoa_CheckInstance(struct lua_State* lua_state, int stack_index);
/**
 * Returns an Obj-C instance.
 * Returns an Obj-C instance at the specified index or nil.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return The Obj-C instance or nil.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT id LuaCocoa_ToInstance(struct lua_State* lua_state, int stack_index);
/**
 * Pushes an Obj-C instance.
 * Pushes an Obj-C instance into Lua.
 * @param lua_state The lua_State to operate on.
 * @param the_object The object to push.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_PushInstance(struct lua_State* lua_state, id the_object);

/**
 * Returns true if an Obj-C class.
 * Returns true if the object at the specified index is an Obj-C class.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return True if an Obj-C class.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_IsClass(struct lua_State* lua_state, int stack_index);
/**
 * Returns an Obj-C class or triggers a Lua error.
 * Returns an Obj-C class the specified index or triggers a Lua error.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return The class.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT Class LuaCocoa_CheckClass(struct lua_State* lua_state, int stack_index);
/**
 * Returns an Obj-C class.
 * Returns an Obj-C class at the specified index or nil.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return The Obj-C class or nil.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT Class LuaCocoa_ToClass(struct lua_State* lua_state, int stack_index);
/**
 * Pushes an Obj-C class.
 * Pushes an Obj-C class into Lua.
 * @param lua_state The lua_State to operate on.
 * @param the_class The class to push.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_PushClass(struct lua_State* lua_state, Class the_class);

/**
 * Returns true for integers, floats, booleans, and NSNumbers.
 * Returns true if the object at the specified index is an integer, float, boolean, or NSNumber.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return True for integers, floats, booleans, and NSNumbers.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_IsNSNumber(struct lua_State* lua_state, int stack_index);
/**
 * Returns a NSNumber or triggers a Lua error.
 * Returns a NSNumber or triggers a Lua error. If the object is already a NSNumber, this will return the pointer.
 * If the object is an integer, float, or boolean, an NSNumber will be constructed and returned.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return A NSNumber.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT NSNumber* LuaCocoa_CheckNSNumber(struct lua_State* lua_state, int stack_index);
/**
 * Returns a NSNumber.
 * Returns a NSNumber at the specified index or nil.
 * If the object is already a NSNumber, this will return the pointer.
 * If the object is an integer, float, or boolean, an NSNumber will be constructed and returned.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return A NSNumber.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT NSNumber* LuaCocoa_ToNSNumber(struct lua_State* lua_state, int stack_index);
/**
 * Pushes a Lua number converted from an NSNumber onto the stack (instead of pushing the NSObject through the bridge).
 * Note that if the NSNumber is a boolean, it will push a Lua boolean instead.
 * On failure, nil is pushed.
 * @param lua_state The lua_State to operate on.
 * @param the_number The NSNumber to copy/push.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_PushUnboxedNSNumber(struct lua_State* lua_state, NSNumber* the_number);

/**
 * Returns true if the object at the specified index is nil or NSNull.
 * Returns true if the object at the specified index is nil or NSNull.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return True for nil and NSNull.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_IsNSNull(struct lua_State* lua_state, int stack_index);
/**
 * Returns NSNull or triggers a Lua error.
 * Returns NSNull or triggers a Lua error.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return NSNull.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT NSNull* LuaCocoa_CheckNSNull(struct lua_State* lua_state, int stack_index);
/**
 * Returns NSNull.
 * Returns NSNull at the specified index or nil.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return NSNull.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT NSNull* LuaCocoa_ToNSNull(struct lua_State* lua_state, int stack_index);

/**
 * Returns true for strings, numbers (which is always convertible to a string), and NSString.
 * Returns true if the object at the specified index is a strings, number (which is always convertible to a string), or NSString.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return True for strings, numbers, and NSString.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_IsNSString(struct lua_State* lua_state, int stack_index);
/**
 * Returns a NSString or triggers a Lua error.
 * Returns a NSString or triggers a Lua error. If the object is already a NSString, this will return the pointer.
 * If the object is a string or number, an NSString will be constructed and returned.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return A NSString.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT NSString* LuaCocoa_CheckNSString(struct lua_State* lua_state, int stack_index);
/**
 * Returns a NSString.
 * Returns a NSString at the specified index or nil.
 * If the object is already a NSString, this will return the pointer.
 * If the object is a string or number, an NSString will be constructed and returned.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return A NSString.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT NSString* LuaCocoa_ToNSString(struct lua_State* lua_state, int stack_index);

/**
 * Pushes a Lua string converted from an NSString onto the stack (instead of pushing the NSObject through the bridge).
 * On failure, nil is pushed.
 * @param lua_state The lua_State to operate on.
 * @param the_string The NSString to copy/push.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_PushUnboxedNSString(struct lua_State* lua_state, NSString* the_string);
/**
 * If a NSString, returns C-string, or if C-string, returns C-string.
 */ 
/**
 * Returns a C string.
 * Returns a C string at the specified index or nil.
 * If the object is a NSString, the implementation tends to call UTF8String/
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return C string.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT __strong const char* LuaCocoa_ToString(struct lua_State* lua_state, int stack_index);

/**
 * Returns true for Lua arrays and NSArrays.
 * Returns true if the object at the specified index is a Lua array or NSArray.
 * If you are planning to convert a Lua array to an NSArray, it may be faster to just
 * try converting via ToPropertyList and checking the return value instead of running this function
 * since both functions need to do essentially the same thing.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return True for Lua arrays and NSArrays.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_IsNSArray(struct lua_State* lua_state, int stack_index);
/**
 * Converts a NSArray to a Lua array (table) and pushes it on the stack.
 * Converts a NSArray to a Lua array (table) and pushes it on the stack.
 * Elements contained in the NSArray will also (recursively) be converted to equivalent Lua types.
 * @param lua_state The lua_State to operate on.
 * @param the_array The NSArray to push.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_PushUnboxedNSArray(struct lua_State* lua_state, NSArray* the_array);


/**
 * Returns true for Lua tables and NSDictionaries.
 * Returns true if the object at the specified index is a Lua tables or NSDictionary.
 * If you are planning to convert a Lua tables to an NSDictionary, it may be faster to just
 * try converting via ToPropertyList and checking the return value instead of running this function
 * since both functions need to do essentially the same thing.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return True for Lua tables and NSDictionaries.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_IsNSDictionary(struct lua_State* lua_state, int stack_index);
/**
 * Converts a NSDictionary to a Lua table and pushes it on the stack.
 * Elements contained in the NSDictionary will also (recursively) be converted to equivalent Lua types.
 */
/**
 * Converts a NSDictionary to a Lua table and pushes it on the stack.
 * Converts a NSDictionary to a Lua table and pushes it on the stack.
 * Elements contained in the NSDictionary will also (recursively) be converted to equivalent Lua types.
 * @param lua_state The lua_State to operate on.
 * @param the_dictionary The NSDictionary to push.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_PushUnboxedNSDictionary(struct lua_State* lua_state, NSDictionary* the_dictionary);

/**
 * Returns true for NSNumber/number, NSNull/nil, NSString/string, NSDictionary, NSArray, or a Lua table.
 * Returns true for NSNumber/number, NSNull/nil, NSString/string, NSDictionary, NSArray, or a Lua table.
 * If you are planning on converting items, espeicial tables and dictionaries, you might want to just call 
 * LuaCocoa_ToPropertyList directly and skip the overhead of doing this check since both methods essentially
 * do the same thing.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return True for NSNumber/number, NSNull/nil, NSString/string, NSDictionary, NSArray, or a Lua table.
 * @note This method may not handle circular references.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_IsPropertyList(struct lua_State* lua_state, int stack_index);
/**
 * Converts Lua types to Cocoa types. 
 * Returns or converts the element at the stack index to one of the appropriate Cocoa containers:
 * NSNumber => number, NSNull => nil, NSString => string, NSDictionary => table, NSArray => table.
 * In NSDictionaries and NSArrays, NSNull might be substituted as placeholders for Lua types that can't be 
 * boxed/converted to Cocoa equivalents.
 * This method will recursively convert items in arrays and dictionaries.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return Returns the Cocoa equivalent container.
 * @note This method may not handle circular references.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT id LuaCocoa_ToPropertyList(struct lua_State* lua_state, int stack_index);

/**
 * Converts Cocoa types to Lua types and pushes on the stack.
 * Converts the following Cocoa containers to their Lua equivalent types:
 * NSNumber => number, NSNull => nil, NSString => string, NSDictionary => table, NSArray => table.
 * Elements contained in NSDictionaries and NSArrays will also (recursively) be converted to their Lua equivalent types.
 * This method will recursively convert items in arrays and dictionaries.
 * @param lua_state The lua_State to operate on.
 * @param the_object The Cocoa object to convert to Lua and push.
 * @note This method may not handle circular references.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_PushUnboxedPropertyList(struct lua_State* lua_state, id the_object);


// Warning: will convert strings and nsstrings and NSValue's with correct encoding to selectors
// Becareful to check for strings/nsstrings separately if you need to distinguish
/**
 * Returns true for selectors or selectors boxed in NSValues.
 * Returns true for selectors or selectors boxed in NSValues.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return True for selectors or selectors boxed in NSValues.
 * @warning Currently this does not consider strings or NSStrings to be selectors, but the sibling functions do. (Maybe this should change.)
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_IsSelector(struct lua_State* lua_state, int stack_index);
/**
 * Returns a selector or triggers a Lua error.
 * Returns a selector or triggers a Lua error. Will handle selectors, boxed selectors in NSValues, strings, and NSStrings.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return A selector.
 * @warning String and NSString are automatically converted to selectors unlike the IsSelector function. Be careful to check for strings/nsstrings separately if you need to distinguish
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT SEL LuaCocoa_CheckSelector(struct lua_State* lua_state, int stack_index);
/**
 * Returns a selector.
 * Returns a selector at the specified index or nil.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return A selector.
 * @warning String and NSString are automatically converted to selectors unlike the IsSelector function. Be careful to check for strings/nsstrings separately if you need to distinguish
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT SEL LuaCocoa_ToSelector(struct lua_State* lua_state, int stack_index);
/**
 * Pushes a selector into Lua.
 * On failure, nil is pushed.
 * @param lua_state The lua_State to operate on.
 * @param the_selector The selector push.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_PushSelector(struct lua_State* lua_state, SEL the_selector);

/**
 * Helper function to get debug info on a Lua function.
 * This is a helper function used to get info about a Lua function. This can be useful for dealing with Lua errors
 * and showing where an error occurred.
 * Assumes function exists. Not expecting to be running in Lua at the moment.
 * Returns autoreleased string containing filename or nil.
 * Returns by reference the line number and last line number of the function.
 * @param lua_state The lua_State to operate on.
 * @param function_name The name of the Lua function.
 * @param line_defined Returns by reference the line the function is defined on.
 * @param last_line_defined Returns by reference the line in the function that an error may occured on.
 * @return autoreleased string containing filename or nil.
 * @warning This function may change or go away.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT NSString* LuaCocoa_GetInfoOnFunction(lua_State* lua_state, const char* function_name, int* line_defined, int* last_line_defined);

/**
 * Helper function to parse a Lua error string and extract the filename and line number.
 * Helper function to parse a Lua error string and extract the filename and line number.
 * @param the_string The string containing the Lua error message.
 * @param line_number Returns by reference the line number the error occurred on.
 * @return The name of the file the error was encountered in.
 * @warning This function may change or go away.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT NSString* LuaCocoa_ParseForErrorFilenameAndLineNumber(NSString* the_string, int* line_number);

// Warning: Parameter signatures are subject to change in future releases to be more consistent with Obj-C. Sorry.
/** 
 * Convenience method to pcall functions in Lua and allows an error function to be specified.
 * This convenience method adopts the techniques described in Programming in Lua's call_va function to make
 * calling arbitrary Lua functions a little easier.
 * @warning Parameter signatures are subject to change in future releases to be more consistent with Obj-C. Sorry.
 * FIXME: Unify with Obj-C method signatures. Perhaps call actual LuaCocoa backend for parsing?
 * Adapted from Programming in Lua call_va
 * Example:
 @code
 LuaCocoa_PcallLuaFunction(lua_state, error_func, @"OnMouseUp" withSignature:"@@idd>bids@@@@", 
 the_layer, layer_name, button_number, x_pos, y_pos,
 &bool_val, &int_val, &double_val, &str_val, &ns_str_val, &layer_val, &array_val, &dict_val);
 @endcode
 * In this case, we pass in two Obj-C objects followed by an int and two doubles.
 * The > marker denotes the return types.
 * In this case, we expect the Lua function to return a boolean, int, double, const char* and 4 Obj-C objects.
 * The return objects go through the propertylist conversion, so in particular, Lua tables and arrays get passed back as 
 * NSDictionary and NSArray.
 * Notes: Since I pop the stack at the end of the function (so you don't have to worry about balancing), 
 * I can't return a char* because it could get deallocated before I am done. 
 * So, the easiest answer seems to be to convert to an NSString. So 's' tokens on the return side get copied
 * into NSString, so you must provide that type of variable.
 * Also, passing nil to Lua functions and  Lua functions returning nil are not supported well. (nil might work through the property list.)
 * Bugs: error handling isn't quite right. luaL_error isn't right because we haven't pcall'ed yet or have returned from pcall.
 * I don't think error: has all the correct information. I think I need to write a special error handler the case of
 * being outside pcall.
 * TODO: Figure out how to support optional return parameters.
 * @note In addition to Programming in Lua which uses 'b' (boolean) 'd' (double) 'i' (integer) 's' (string) '>'
 * I also add '@' for id, '#' for Class, and 'v' for void*.
 * @param lua_state The lua_State to operate on.
 * @param lua_error_function The error function to handle any Lua errors.
 * @param lua_function_name The name of the Lua function to call.
 * @param parameter_signature A format string describing the parameter types. (See above notes.)
 * @param ... The list of parameters to go with the format string.
 * @return A NSString containing an error message if pcall failed, nil otherwise.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT NSString* LuaCocoa_PcallLuaFunction(lua_State* lua_state, lua_CFunction lua_error_function, const char* lua_function_name, const char* parameter_signature, ...);
/** 
 * Convenience method to pcall functions in Lua and allows an error function to be specified.
 * This convenience method adopts the techniques described in Programming in Lua's call_va function to make
 * calling arbitrary Lua functions a little easier.
 * @warning Parameter signatures are subject to change in future releases to be more consistent with Obj-C. Sorry.
 * FIXME: Unify with Obj-C method signatures. Perhaps call actual LuaCocoa backend for parsing?
 * Adapted from Programming in Lua call_va
 * Example:
 @code
 LuaCocoa_PcallLuaFunctionv(lua_state, error_func, @"OnMouseUp" withSignature:"@@idd>bids@@@@", va_list);
 @endcode
 * In this case, we pass in two Obj-C objects followed by an int and two doubles.
 * The > marker denotes the return types.
 * In this case, we expect the Lua function to return a boolean, int, double, const char* and 4 Obj-C objects.
 * The return objects go through the propertylist conversion, so in particular, Lua tables and arrays get passed back as 
 * NSDictionary and NSArray.
 * Notes: Since I pop the stack at the end of the function (so you don't have to worry about balancing), 
 * I can't return a char* because it could get deallocated before I am done. 
 * So, the easiest answer seems to be to convert to an NSString. So 's' tokens on the return side get copied
 * into NSString, so you must provide that type of variable.
 * Also, passing nil to Lua functions and  Lua functions returning nil are not supported well. (nil might work through the property list.)
 * Bugs: error handling isn't quite right. luaL_error isn't right because we haven't pcall'ed yet or have returned from pcall.
 * I don't think error: has all the correct information. I think I need to write a special error handler the case of
 * being outside pcall.
 * TODO: Figure out how to support optional return parameters.
 * @note In addition to Programming in Lua which uses 'b' (boolean) 'd' (double) 'i' (integer) 's' (string) '>'
 * I also add '@' for id, '#' for Class, and 'v' for void*.
 * @param lua_state The lua_State to operate on.
 * @param lua_error_function The error function to handle any Lua errors.
 * @param lua_function_name The name of the Lua function to call.
 * @param parameter_signature A format string describing the parameter types. (See above notes.)
 * @param vl The list of parameters to go with the format string.
 * @return A NSString containing an error message if pcall failed, nil otherwise.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT NSString* LuaCocoa_PcallLuaFunctionv(lua_State* lua_state, lua_CFunction lua_error_function, const char* lua_function_name, const char* parameter_signature, va_list vl);


#endif /* __OBJC__ */

/**
 * Prepends a new path the Lua package search path.
 * Modifies the package.path to append the specified string. Should be in proper lua format:
 * e.g. /Users/ewing/Source/HG/LuaCocoa/Xcode/build/Debug/LuaCocoa.framework/Resources/?.lua
 * This means you must explicitly include the ?.lua (or whatever file extensions you use.)
 * @param lua_state The lua_State to operate on.
 * @param search_path The path to prepend.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_PrependToLuaSearchPath(struct lua_State* lua_state, const char* search_path);

/**
 * Appends a new path the Lua package search path.
 * Modifies the package.path to append the specified string. Should be in proper lua format:
 * e.g. /Users/ewing/Source/HG/LuaCocoa/Xcode/build/Debug/LuaCocoa.framework/Resources/?.lua
 * This means you must explicitly include the ?.lua (or whatever file extensions you use.)
 * @param lua_state The lua_State to operate on.
 * @param search_path The path to append.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_AppendToLuaSearchPath(struct lua_State* lua_state, const char* search_path);

/**
 * Prepends a new path the Lua/C package search path.
 * Modifies the package.cpath to append the specified string. Should be in proper lua format:
 * e.g. /Users/ewing/Library/Application Support/LuaCocoa/PlugIns/?.so
 * This means you must explicitly include the ?.so (or whatever file extensions you use.)
 * @param lua_state The lua_State to operate on.
 * @param search_path The path to prepend.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_PrependToCSearchPath(struct lua_State* lua_state, const char* search_path);

/**
 * Appends a new path the Lua/C package search path.
 * Modifies the package.cpath to append the specified string. Should be in proper lua format:
 * e.g. /Users/ewing/Library/Application Support/LuaCocoa/PlugIns/?.so
 * This means you must explicitly include the ?.so (or whatever file extensions you use.)
 * @param lua_state The lua_State to operate on.
 * @param search_path The path to append.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void LuaCocoa_AppendToCSearchPath(struct lua_State* lua_state, const char* search_path);


// These struct APIs may be subject to change. I'm unhappy with the keyname vs. structname issues.
// These functions are mostly untested. Please report bugs/problems/API defects.
/**
 * Returns true if the object is a struct.
 * Returns true if the object at the specified index is a struct (as known to the bridge).
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return True if a struct, false if failure.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_IsStruct(struct lua_State* lua_state, int stack_index);
/**
 * Returns true if the object is a struct of the named type you specify.
 * Returns true if the object at the specified index is a struct of the named type you specify (as known to the bridge).
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @param key_name The name of the struct.
 * @return True if a match, false if failure.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_IsStructWithName(struct lua_State* lua_state, int stack_index, const char* key_name);
/**
 * Returns the name of the struct.
 * Returns the name of the struct at the specified index.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @return The name of the struct.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT const char* LuaCocoa_GetStructName(struct lua_State* lua_state, int stack_index);
/**
 * Returns a pointer to a struct or triggers a Lua error.
 * Returns a pointer to a struct by the given name at the specified index or triggers a Lua error.
 * @param lua_state The lua_State to operate on.
 * @param stack_index The index of the stack of the object you want to know about.
 * @param key_name The name of the struct.
 * @return A pointer to the struct.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT void* LuaCocoa_CheckStruct(struct lua_State* lua_state, int stack_index, const char* key_name);
/**
 * Pushes a copy of the struct passed in onto the stack.
 * Pushes a copy of the struct passed in onto the stack. 
 * If the metatable for the struct type cannot be found, the function will fail and nothing will be pushed.
 * @param lua_state The lua_State to operate on.
 * @param the_struct A pointer to the struct to push
 * @param key_name The name of the struct.
 * @return False if there is a failure.
 */
LUACOCOA_EXTERN LUACOCOA_EXPORT bool LuaCocoa_PushStruct(struct lua_State* lua_state, void* the_struct, const char* key_name);

#endif /* LUA_COCOA_H */
