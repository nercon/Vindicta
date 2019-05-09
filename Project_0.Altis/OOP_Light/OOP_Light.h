/*
 * OOP-Light
 * A preprocessor-based limited OOP implementation for SQF
 * Author: Sparker
 * 02.06.2018
*/

/*
 * Technical info:
 *
 * Name formatting:
 * Special static class members:	o_MyClass_sm_mySpecialMember
 * Static class members: 			o_MyClass_st_myStaticMember
 * General members:					o_MyClass_N_123_myMember
 *
 * Special class members:
 * special members are static members of the class meant to be accessed only by the internals of the OOP-Light
 * nextID			- NUMBER counter to provide a new ID each time an object of this class is created
 * memList			- ARRAY with all members of this class
 * staticMemList	- ARRAY with all static members of this class
 * methodList		- ARRAY with all methods and static methods of this class
 *
 */

// ----------------------------------------------------
// |          C O N T R O L  F L A G S                |
// ----------------------------------------------------

// Defines the ofstream file name for OOP_INFO_, OOP_ERROR_, OOP_WARNING_ macros
// Must be defined before including OOP_Light.h into your class definition .sqf file
//#define OFSTREAM_FILE "OOP.rpt"

// Enables output to external file with ofstream in all OOP classes
// It's a global flag, must be defined here

#define OFSTREAM_ENABLE

// Enables checks for member accesses at runtime
// As well as other assertions
// It's a global flag, must be defined here

#define OOP_ASSERT
// #define OOP_ASSERT_ACCESS

#ifdef _SQF_VM
#define TEXT_
#undef ASP_ENABLE
#undef OFSTREAM_ENABLE
#undef OFSTREAM_FILE
#define VM_LOG(t) diag_log t
#define VM_LOG_FMT(t, args) diag_log format ([t] + args)
#define OOP_ASSERT
#define OOP_ASSERT_ACCESS
#undef OOP_DEBUG
#undef OOP_INFO
#define OOP_WARNING
#define OOP_ERROR

#define TIME_NOW 0
#define CLIENT_OWNER objNull

#else
#define TEXT_ text

#define VM_LOG(t)
#define VM_LOG_FMT(t, args)

#define TIME_NOW time
#define CLIENT_OWNER clientOwner

#endif

// Defining OOP_SCRIPTNAME it will add 	_fnc_scriptName = "..."; to each method created with OOP_Light
// You can either define it here or usage of OOP_INFO_, ..., macros will cause its automatic definition
// ! ! ! It's currently totally disabled because recompiling breaks file names in callstacks ! ! !
// OOP SCRIPTNAME

//#define OOP_SCRIPTNAME
/*
#ifdef OOP_INFO
#define OOP_SCRIPTNAME
#endif
#ifdef OOP_WARNING
#define OOP_SCRIPTNAME
#endif
#ifdef OOP_ERROR
#define OOP_SCRIPTNAME
#endif
*/

// Enables support for Arma Script Profiler globally
// Set it in this file
//#define ASP_ENABLE

// Enables macros for Arma Script Profiler counters, enables global counter variables per every class
// Define it at the top of the file per every class where you need to count objects
//#define PROFILER_COUNTERS_ENABLE

// Notifies code that Arma Debug Engine is enabled. Currently it is used to dump callstack.
#define ADE

// ----------------------------------------------------------------------
// |                P R O F I L E R   C O U N T E R S                   |
// ----------------------------------------------------------------------
#ifndef ASP_ENABLE
#undef PROFILER_COUNTERS_ENABLE
#endif

#define COUNTER_NAME_STR(nameStr) ("g_profCnt_" + nameStr)

#ifdef PROFILER_COUNTERS_ENABLE

#define PROFILER_COUNTER_INIT(nameStr) missionNamespace setVariable[COUNTER_NAME_STR(nameStr), 0]; nameStr profilerSetCounter 0;

#define PROFILER_COUNTER_INC(nameStr) isNil { \
private _oop_cnt = missionNamespace getVariable COUNTER_NAME_STR(nameStr); \
missionNamespace setVariable [COUNTER_NAME_STR(nameStr), _oop_cnt+1]; \
nameStr profilerSetCounter _oop_cnt; };

#define PROFILER_COUNTER_DEC(nameStr) isNil { \
private _oop_cnt = missionNamespace getVariable COUNTER_NAME_STR(nameStr); \
missionNamespace setVariable [COUNTER_NAME_STR(nameStr), _oop_cnt-1]; \
nameStr profilerSetCounter _oop_cnt; };

#else
#define PROFILER_COUNTER_INIT(nameStr)
#define PROFILER_COUNTER_INC(nameStr)
#define PROFILER_COUNTER_DEC(nameStr)
#endif

// Minimum amount of time for a function to take before its profile entry will be written out
#ifndef OOP_PROFILE_MIN_T
	#define OOP_PROFILE_MIN_T 0.1
#endif

/*
#ifdef OOP_ASSERT
	diag_log "[OOP] Warning: member assertion is enabled. Disable it for better performance.";
#endif
*/

// Namespaces are NYI :/
#ifndef NAMESPACE
	#define NAMESPACE missionNameSpace
#endif

// ----------------------------------------------------------------------
// |                 I N T E R N A L   S T R I N G S                    |
// ----------------------------------------------------------------------

#define OOP_PREFIX "o_"
#define OBJECT_SEPARATOR "_N_"
#define SPECIAL_SEPARATOR "_spm_"
#define STATIC_SEPARATOR "_stm_"
#define METHOD_SEPARATOR "_fnc_"
#define INNER_PREFIX "inner_"

// ----------------------------------------------------------------------
// |          I N T E R N A L   N A M E   F O R M A T T I N G           |
// ----------------------------------------------------------------------

//Name of a specific instance of object
#define OBJECT_NAME_STR(classNameStr, objIDInt) (OOP_PREFIX + (classNameStr) + OBJECT_SEPARATOR + (format ["%1", objIDInt]))

//String name of a static member
#define CLASS_STATIC_MEM_NAME_STR(classNameStr, memNameStr) ((OOP_PREFIX) + (classNameStr) + STATIC_SEPARATOR + (memNameStr))

//String name of a method
#define CLASS_METHOD_NAME_STR(classNameStr, methodNameStr) ((classNameStr) + METHOD_SEPARATOR + (methodNameStr))

//String name of a special member
#define CLASS_SPECIAL_MEM_NAME_STR(classNameStr, memNameStr) (OOP_PREFIX + (classNameStr) + SPECIAL_SEPARATOR + (memNameStr))

//String name of a non-static member
#define OBJECT_MEM_NAME_STR(objNameStr, memNameStr) ((objNameStr) + "_" + memNameStr)

//Gets parent class of an object
#define OBJECT_PARENT_CLASS_STR(objNameStr) (FORCE_GET_MEM(objNameStr, OOP_PARENT_STR))

//String name of an inner method
#define INNER_METHOD_NAME_STR(methodNameStr) (INNER_PREFIX + methodNameStr)

// ==== Private special members
#define NEXT_ID_STR "nextID"
#define MEM_LIST_STR "memList"
#define STATIC_MEM_LIST_STR "staticMemList"
#define METHOD_LIST_STR "methodList"
#define PARENTS_STR "parents"
#define OOP_PARENT_STR "oop_parent"
#define OOP_PUBLIC_STR "oop_public"

// Other important strings
#define OOP_ERROR_DEBRIEFING_SECTION_VAR_NAME_STR "oop_missionEndText"
// CfgDebriefing class entry in description.ext which is shown when a critical OOP error happens
#define OOP_ERROR_DEBRIEFING_CLASS_NAME	end_OOP_class_error
#define OOP_ERROR_DEBRIEFING_CLASS_NAME_STR "end_OOP_class_error"

// ----------------------------------------------------------------------
// |          I N T E R N A L   A C C E S S   M E M B E R S             |
// ----------------------------------------------------------------------

#define FORCE_SET_MEM(objNameStr, memNameStr, value) NAMESPACE setVariable [OBJECT_MEM_NAME_STR(objNameStr, memNameStr), value]
#define FORCE_SET_MEM_REF(objNameStr, memNameStr, value) \
	isNil { \
		private _oldVal = NAMESPACE getVariable [OBJECT_MEM_NAME_STR(objNameStr, memNameStr), objNull]; \
		if (_oldVal isEqualType "") then { CALLM0(_oldVal, "unref") }; \
		if ((value) isEqualType "") then { CALLM0((value), "ref") }; \
		NAMESPACE setVariable [OBJECT_MEM_NAME_STR(objNameStr, memNameStr), value] \
	}

#define FORCE_SET_STATIC_MEM(classNameStr, memNameStr, value) missionNamespace setVariable [CLASS_STATIC_MEM_NAME_STR(classNameStr, memNameStr), value]
#define FORCE_SET_METHOD(classNameStr, methodNameStr, code) missionNamespace setVariable [CLASS_METHOD_NAME_STR(classNameStr, methodNameStr), code]
#define FORCE_GET_MEM(objNameStr, memNameStr) ( NAMESPACE getVariable OBJECT_MEM_NAME_STR(objNameStr, memNameStr) )
#define FORCE_GET_STATIC_MEM(classNameStr, memNameStr) ( NAMESPACE getVariable CLASS_STATIC_MEM_NAME_STR(classNameStr, memNameStr) )
#define FORCE_GET_METHOD(classNameStr, methodNameStr) ( NAMESPACE getVariable CLASS_METHOD_NAME_STR(classNameStr, methodNameStr) )

#ifndef _SQF_VM
#define FORCE_PUBLIC_MEM(objNameStr, memNameStr) publicVariable OBJECT_MEM_NAME_STR(objNameStr, memNameStr)
#define FORCE_PUBLIC_STATIC_MEM(classNameStr, memNameStr) publicVariable CLASS_STATIC_MEM_NAME_STR(classNameStr, memNameStr)
#else
#define FORCE_PUBLIC_MEM(objNameStr, memNameStr)
#define FORCE_PUBLIC_STATIC_MEM(classNameStr, memNameStr)
#endif

//Special members don't use run time checks
#define SET_SPECIAL_MEM(classNameStr, memNameStr, value) missionNamespace setVariable [CLASS_SPECIAL_MEM_NAME_STR(classNameStr, memNameStr), value]
#define GET_SPECIAL_MEM(classNameStr, memNameStr) ( missionNamespace getVariable CLASS_SPECIAL_MEM_NAME_STR(classNameStr, memNameStr) )

// -----------------------------------------------------
// |           A C C E S S   M E M B E R S             |
// -----------------------------------------------------

#ifdef OOP_ASSERT_ACCESS
#define ASSERT_SET_MEMBER_ACCESS(objNameStr, memNameStr) 			[objNameStr, memNameStr, __FILE__, __LINE__] call OOP_assert_set_member_access
#define ASSERT_SET_STATIC_MEMBER_ACCESS(classNameStr, memNameStr) 	[classNameStr, memNameStr, __FILE__, __LINE__] call OOP_assert_set_static_member_access
#define ASSERT_GET_MEMBER_ACCESS(objNameStr, memNameStr) 			[objNameStr, memNameStr, __FILE__, __LINE__] call OOP_assert_get_member_access
#define ASSERT_GET_STATIC_MEMBER_ACCESS(classNameStr, memNameStr)	[classNameStr, memNameStr, __FILE__, __LINE__] call OOP_assert_get_static_member_access
#else
#define ASSERT_SET_MEMBER_ACCESS(objNameStr, memNameStr) 			
#define ASSERT_SET_STATIC_MEMBER_ACCESS(classNameStr, memNameStr) 	
#define ASSERT_GET_MEMBER_ACCESS(objNameStr, memNameStr) 			
#define ASSERT_GET_STATIC_MEMBER_ACCESS(classNameStr, memNameStr)	
#endif

#ifdef OOP_ASSERT
	#define SET_MEM(objNameStr, memNameStr, value) \
		if([objNameStr, memNameStr, __FILE__, __LINE__] call OOP_assert_member_is_not_ref) then { \
			ASSERT_SET_MEMBER_ACCESS(objNameStr, memNameStr); \
			FORCE_SET_MEM(objNameStr, memNameStr, value) \
		}
	#define SET_MEM_REF(objNameStr, memNameStr, value) \
		if([objNameStr, memNameStr, __FILE__, __LINE__] call OOP_assert_member_is_ref) then { \
			ASSERT_SET_MEMBER_ACCESS(objNameStr, memNameStr); \
			FORCE_SET_MEM_REF(objNameStr, memNameStr, value) \
		}
	#define SET_STATIC_MEM(classNameStr, memNameStr, value) \
		if([classNameStr, memNameStr, __FILE__, __LINE__] call OOP_assert_staticMember) then { \
			ASSERT_SET_STATIC_MEMBER_ACCESS(classNameStr, memNameStr); \
			FORCE_SET_STATIC_MEM(classNameStr, memNameStr, value) \
		}
	#define GET_MEM(objNameStr, memNameStr) \
		( if([objNameStr, memNameStr, __FILE__, __LINE__] call OOP_assert_member) then { \
			ASSERT_GET_MEMBER_ACCESS(objNameStr, memNameStr); \
			FORCE_GET_MEM(objNameStr, memNameStr) \
		}else{nil} )
	#define GET_STATIC_MEM(classNameStr, memNameStr) \
		( if([classNameStr, memNameStr, __FILE__, __LINE__] call OOP_assert_staticMember) then { \
			ASSERT_GET_STATIC_MEMBER_ACCESS(classNameStr, memNameStr); \
			FORCE_GET_STATIC_MEM(classNameStr, memNameStr) \
		}else{nil} )
	#define GET_METHOD(classNameStr, methodNameStr) \
		( if([classNameStr, methodNameStr, __FILE__, __LINE__] call OOP_assert_method) then { \
			FORCE_GET_METHOD(classNameStr, methodNameStr) \
		}else{nil} )
	#define PUBLIC_MEM(objNameStr, memNameStr) \
		if([objNameStr, memNameStr, __FILE__, __LINE__] call OOP_assert_member) then { \
			FORCE_PUBLIC_MEM(objNameStr, memNameStr) \
		}
	#define PUBLIC_STATIC_MEM(classNameStr, memNameStr) \
		if([classNameStr, memNameStr, __FILE__, __LINE__] call OOP_assert_staticMember) then { \
			FORCE_PUBLIC_STATIC_MEM(classNameStr, memNameStr) \
		}
#else
	#define SET_MEM(objNameStr, memNameStr, value) FORCE_SET_MEM(objNameStr, memNameStr, value)
	#define SET_MEM_REF(objNameStr, memNameStr, value) FORCE_SET_MEM_REF(objNameStr, memNameStr, value)
	#define SET_STATIC_MEM(classNameStr, memNameStr, value) FORCE_SET_STATIC_MEM(classNameStr, memNameStr, value)
	#define GET_MEM(objNameStr, memNameStr) FORCE_GET_MEM(objNameStr, memNameStr)
	#define GET_STATIC_MEM(classNameStr, memNameStr) FORCE_GET_STATIC_MEM(classNameStr, memNameStr)
	#define GET_METHOD(classNameStr, methodNameStr) FORCE_GET_METHOD(classNameStr, methodNameStr)
	#define PUBLIC_MEM(objNameStr, memNameStr) FORCE_PUBLIC_MEM(objNameStr, memNameStr)
	#define PUBLIC_STATIC_MEM(classNameStr, memNameStr) FORCE_PUBLIC_STATIC_MEM(classNameStr, memNameStr)
#endif

#define SET_VAR(a, b, c) SET_MEM(a, b, c)
#define SET_VAR_REF(a, b, c) SET_MEM_REF(a, b, c)
#define SET_STATIC_VAR(a, b, c) SET_STATIC_MEM(a, b, c)
#define GET_VAR(a, b) GET_MEM(a, b)
#define GET_STATIC_VAR(a, b) GET_STATIC_MEM(a, b)
#define PUBLIC_VAR(a, b) PUBLIC_MEM(a, b)
#define PUBLIC_STATIC_VAR(a, b) PUBLIC_STATIC_MEM(a, b)
#define SET_VAR_PUBLIC(a, b, c) SET_VAR(a, b, c); PUBLIC_VAR(a, b)

// Shortened variants of macros
#define SETV(a, b, c) SET_VAR(a, b, c)
#define SETV_REF(a, b, c) SET_VAR_REF(a, b, c)
#define SETSV(a, b, c) SET_STATIC_VAR(a, b, c)
#define GETV(a, b) GET_VAR(a, b)
#define GETSV(a, b) GET_STATIC_VAR(a, b)
#define PVAR(a, b) PUBLIC_VAR(a, b)

// Getting/setting variables of _thisObject
#define T_SETV(varNameStr, varValue) SET_VAR(_thisObject, varNameStr, varValue)
#define T_SETV_REF(varNameStr, varValue) SET_VAR_REF(_thisObject, varNameStr, varValue)
#define T_GETV(varNameStr) GET_VAR(_thisObject, varNameStr)

// Unpacking a _thisObject variable into a private _variable
// So if we have private _var = GET_VAR(_thisObject, "var"), this macros can help
#define __STRINGIFY(s) #s
#define T_PRVAR(varName) private _##varName = GET_VAR(_thisObject, __STRINGIFY(varName))

// Returns object class name
#define GET_OBJECT_CLASS(_objNameStr) OBJECT_PARENT_CLASS_STR(_objNameStr)

// Returns true if reference passed is pointing at a valid object 
#define IS_OOP_OBJECT(objNameStr) (! (isNil {GET_OBJECT_CLASS(_objNameStr)}))

// Returns variable names of this class
#define GET_CLASS_MEMBERS(classNameStr) GET_SPECIAL_MEM(classNameStr, MEM_LIST_STR)

// -----------------------------------------------------
// |             M E T H O D   C A L L S               |
// -----------------------------------------------------

#define GETM(objNameStr, methodNameStr) GET_METHOD(OBJECT_PARENT_CLASS_STR(objNameStr), methodNameStr)

//Same performance for small functions
//#define CALL_METHOD(objNameStr, methodNameStr, extraParams) ([objNameStr] + extraParams) call (call compile (CLASS_STATIC_MEM_NAME_STR(OBJECT_PARENT_CLASS_STR(objNameStr), methodNameStr)))
#define CALL_METHOD(objNameStr, methodNameStr, extraParams) (([objNameStr] + extraParams) call GET_METHOD(OBJECT_PARENT_CLASS_STR(objNameStr), methodNameStr))
#define CALL_METHOD_0(objNameStr, methodNameStr) (([objNameStr]) call GET_METHOD(OBJECT_PARENT_CLASS_STR(objNameStr), methodNameStr))
#define CALL_METHOD_1(objNameStr, methodNameStr, a) (([objNameStr, a]) call GET_METHOD(OBJECT_PARENT_CLASS_STR(objNameStr), methodNameStr))
#define CALL_METHOD_2(objNameStr, methodNameStr, a, b) (([objNameStr, a, b]) call GET_METHOD(OBJECT_PARENT_CLASS_STR(objNameStr), methodNameStr))
#define CALL_METHOD_3(objNameStr, methodNameStr, a, b, c) (([objNameStr, a, b, c]) call GET_METHOD(OBJECT_PARENT_CLASS_STR(objNameStr), methodNameStr))
#define CALL_METHOD_4(objNameStr, methodNameStr, a, b, c, d) (([objNameStr, a, b, c, d]) call GET_METHOD(OBJECT_PARENT_CLASS_STR(objNameStr), methodNameStr))

#define CALL_CLASS_METHOD(classNameStr, objNameStr, methodNameStr, extraParams) (([objNameStr] + extraParams) call GET_METHOD(classNameStr, methodNameStr))

#define CALL_STATIC_METHOD(classNameStr, methodNameStr, extraParams) (([classNameStr] + extraParams) call GET_METHOD(classNameStr, methodNameStr))
#define CALL_STATIC_METHOD_0(classNameStr, methodNameStr) ([classNameStr] call GET_METHOD(classNameStr, methodNameStr))
#define CALL_STATIC_METHOD_1(classNameStr, methodNameStr, a) (([classNameStr, a]) call GET_METHOD(classNameStr, methodNameStr))
#define CALL_STATIC_METHOD_2(classNameStr, methodNameStr, a, b) (([classNameStr, a, b]) call GET_METHOD(classNameStr, methodNameStr))
#define CALL_STATIC_METHOD_3(classNameStr, methodNameStr, a, b, c) (([classNameStr, a, b, c]) call GET_METHOD(classNameStr, methodNameStr))
#define CALL_STATIC_METHOD_4(classNameStr, methodNameStr, a, b, c, d) (([classNameStr, a, b, c, d]) call GET_METHOD(classNameStr, methodNameStr))

// Shortened variants of macros
#define CALLM(a, b, c) CALL_METHOD(a, b, c)
#define CALLCM(a, b, c) CALL_CLASS_METHOD(a, b, c)
#define CALLSM(a, b, c) CALL_STATIC_METHOD(a, b, c)

// Macros for multiple variables
#define CALLM0(a, b) CALL_METHOD_0(a, b)
#define CALLM1(a, b, c) CALL_METHOD_1(a, b, c)
#define CALLM2(a, b, c, d) CALL_METHOD_2(a, b, c, d)
#define CALLM3(a, b, c, d, e) CALL_METHOD_3(a, b, c, d, e)
#define CALLM4(a, b, c, d, e, f) CALL_METHOD_4(a, b, c, d, e, f)

// Macros for calls to this
#define T_CALLM(a, b) CALL_METHOD(_thisObject, a, b)
#define T_CALLM0(a) CALL_METHOD_0(_thisObject, a)
#define T_CALLM1(a, b) CALL_METHOD_1(_thisObject, a, b)
#define T_CALLM2(a, b, c) CALL_METHOD_2(_thisObject, a, b, c)
#define T_CALLM3(a, b, c, d) CALL_METHOD_3(_thisObject, a, b, c, d)
#define T_CALLM4(a, b, c, d, e) CALL_METHOD_4(_thisObject, a, b, c, d, e)

#define CALLSM0(a, b) CALL_STATIC_METHOD_0(a, b)
#define CALLSM1(a, b, c) CALL_STATIC_METHOD_1(a, b, c)
#define CALLSM2(a, b, c, d) CALL_STATIC_METHOD_2(a, b, c, d)
#define CALLSM3(a, b, c, d, e) CALL_STATIC_METHOD_3(a, b, c, d, e)
#define CALLSM4(a, b, c, d, e, f) CALL_STATIC_METHOD_4(a, b, c, d, e, f)

// Remote executions
#define REMOTE_EXEC_METHOD(objNameStr, methodNameStr, extraParams, targets) [objNameStr, methodNameStr, extraParams] remoteExec ["OOP_callFromRemote", targets, false]
#define REMOTE_EXEC_CALL_METHOD(objNameStr, methodNameStr, extraParams, targets) [objNameStr, methodNameStr, extraParams] remoteExecCall ["OOP_callFromRemote", targets, false]

#ifdef OOP_ASSERT
#define REMOTE_EXEC_STATIC_METHOD(classNameStr, methodNameStr, extraParams, targets, JIP) [classNameStr, methodNameStr, extraParams] remoteExec ["OOP_callStaticMethodFromRemote", targets, JIP];
#define REMOTE_EXEC_CALL_STATIC_METHOD(classNameStr, methodNameStr, extraParams, targets, JIP) [classNameStr, methodNameStr, extraParams] remoteExecCall ["OOP_callStaticMethodFromRemote", targets, JIP];
#else
#define REMOTE_EXEC_STATIC_METHOD(classNameStr, methodNameStr, extraParams, targets, JIP) ([classNameStr] + extraParams) remoteExec [CLASS_METHOD_NAME_STR(classNameStr, methodNameStr), targets, JIP];
#define REMOTE_EXEC_CALL_STATIC_METHOD(classNameStr, methodNameStr, extraParams, targets, JIP) ([classNameStr] + extraParams) remoteExecCall [CLASS_METHOD_NAME_STR(classNameStr, methodNameStr), targets, JIP];
#endif

// ----------------------------------------
// |         A T T R I B U T E S          |
// ----------------------------------------

#define ATTR_REFCOUNTED 1
#define ATTR_SERIALIZABLE 2
#define ATTR_PRIVATE 3
// Needs more work to implement this (walking classes to find the first place a member was defined etc.)
// #define ATTR_PROTECTED 4
#define ATTR_GET_ONLY 5
#define ATTR_THREAD_AFFINITY_ID 6
#define ATTR_THREAD_AFFINITY(getThreadFn) [ATTR_THREAD_AFFINITY_ID, getThreadFn]
#define ATTR_USERBASE 1000

// -----------------------------------------------------
// |       M E M B E R   D E C L A R A T I O N S       |
// -----------------------------------------------------

#define VARIABLE(varNameStr) _oop_memList pushBackUnique [varNameStr, []]
#define VARIABLE_ATTR(varNameStr, attributes) _oop_memList pushBackUnique [varNameStr, attributes]

#define STATIC_VARIABLE(varNameStr) _oop_staticMemList pushBackUnique [varNameStr, []]
#define STATIC_VARIABLE_ATTR(varNameStr, attributes) _oop_staticMemList pushBackUnique [varNameStr, attributes]

#define MEMBER(memNameStr) VARIABLE(memNameStr)

#define STATIC_MEMBER(memNameStr) STATIC_VARIABLE(memNameStr)

#ifdef OOP_PROFILE
	#define _OOP_FUNCTION_WRAPPERS

	#define PROFILE_SCOPE_START(scopeName) \
		private _profileTStart##scopeName = time;

	#define PROFILE_SCOPE_END(scopeName, minT) \
		private _totalProfileT##scopeName = time - _profileTStart##scopeName; \
		if(_totalProfileT##scopeName > minT) then { \
			OOP_PROFILE_2("%1 %2", #scopeName, _totalProfileT##scopeName); \
		};

	#define OOP_FUNC_HEADER_PROFILE private _profileTStart = time
	#define OOP_FUNC_FOOTER_PROFILE \
		private _totalProfileT = time - _profileTStart; \
		if(_totalProfileT > OOP_PROFILE_MIN_T) then { \
			OOP_PROFILE_3("%1.%2 %3", _objOrClass, _methodNameStr, _totalProfileT); \
		}
#else
	#define PROFILE_SCOPE_START(scopeName)
	#define PROFILE_SCOPE_END(scopeName, minT)
	#define OOP_FUNC_HEADER_PROFILE
	#define OOP_FUNC_FOOTER_PROFILE
#endif

#ifdef OOP_ASSERT_ACCESS
#define _OOP_FUNCTION_WRAPPERS
#endif

// If some enabled functionality requires function wrappers we set them here. If you want to conditionally add more stuff to the wrapped functions
// (e.g. additional asserts, parameter manipulation etc.) then define them as macros and then include them in the wrapped blocks in the same manner
// that OOP_PROFILE does.
#ifdef _OOP_FUNCTION_WRAPPERS
	#define METHOD(methodNameStr) \
		_oop_methodList pushBackUnique methodNameStr;  \
		_oop_newMethodList pushBackUnique methodNameStr; \
		NAMESPACE setVariable [CLASS_METHOD_NAME_STR(_oop_classNameStr, methodNameStr), { \
			private _thisClass = nil; \
			private _methodNameStr = methodNameStr; \
			private _objOrClass = _this select 0; \
			OOP_FUNC_HEADER_PROFILE; \
			private _result = ([0] apply { _this call

	#define ENDMETHOD }) select 0;\
			OOP_FUNC_FOOTER_PROFILE; \
			if !(isNil "_result") then { _result } else { nil } \
		} ]

	#define METHOD_FILE(methodNameStr, path) \
		_oop_methodList pushBackUnique methodNameStr; \
		_oop_newMethodList pushBackUnique methodNameStr; \
		NAMESPACE setVariable [CLASS_METHOD_NAME_STR(_oop_classNameStr, INNER_METHOD_NAME_STR(methodNameStr)), compile preprocessFileLineNumbers path]; \
		NAMESPACE setVariable [CLASS_METHOD_NAME_STR(_oop_classNameStr, methodNameStr), { \
			private _thisClass = nil; \
			private _methodNameStr = methodNameStr; \
			private _objOrClass = _this select 0; \
			OOP_FUNC_HEADER_PROFILE; \
			private _fn = NAMESPACE getVariable CLASS_METHOD_NAME_STR(OBJECT_PARENT_CLASS_STR(_objOrClass), INNER_METHOD_NAME_STR(methodNameStr)); \
			private _result = ([0] apply { _this call _fn }) select 0; \
			OOP_FUNC_FOOTER_PROFILE; \
			if !(isNil "_result") then { _result } else { nil } \
		}]

	#define STATIC_METHOD(methodNameStr) \
		_oop_methodList pushBackUnique methodNameStr; \
		_oop_newMethodList pushBackUnique methodNameStr; \
		NAMESPACE setVariable [CLASS_METHOD_NAME_STR(_oop_classNameStr, methodNameStr), { \
			private _thisObject = nil; \
			private _methodNameStr = methodNameStr; \
			private _objOrClass = _this select 0; \
			OOP_FUNC_HEADER_PROFILE; \
			private _result = ([0] apply { _this call

	#define STATIC_METHOD_FILE(methodNameStr, path) \
		_oop_methodList pushBackUnique methodNameStr; \
		_oop_newMethodList pushBackUnique methodNameStr; \
		NAMESPACE setVariable [CLASS_METHOD_NAME_STR(_oop_classNameStr, INNER_METHOD_NAME_STR(methodNameStr)), compile preprocessFileLineNumbers path]; \
		NAMESPACE setVariable [CLASS_METHOD_NAME_STR(_oop_classNameStr, methodNameStr), { \
			private _thisObject = nil; \
			private _methodNameStr = methodNameStr; \
			private _objOrClass = _this select 0; \
			OOP_FUNC_HEADER_PROFILE; \
			private _fn = NAMESPACE getVariable CLASS_METHOD_NAME_STR(_objOrClass, INNER_METHOD_NAME_STR(methodNameStr)); \
			private _result = ([0] apply { _this call _fn}) select 0; \
			OOP_FUNC_FOOTER_PROFILE; \
			if !(isNil "_result") then { _result } else { nil } \
		}]
#else
	#define METHOD(methodNameStr) \
		_oop_methodList pushBackUnique methodNameStr; \
		_oop_newMethodList pushBackUnique methodNameStr; \
		NAMESPACE setVariable [CLASS_METHOD_NAME_STR(_oop_classNameStr, methodNameStr),
	#define ENDMETHOD ]

	#define METHOD_FILE(methodNameStr, path) \
		_oop_methodList pushBackUnique methodNameStr; \
		_oop_newMethodList pushBackUnique methodNameStr; \
		NAMESPACE setVariable [CLASS_METHOD_NAME_STR(_oop_classNameStr, methodNameStr), compile preprocessFileLineNumbers path]

	#define STATIC_METHOD(methodNameStr) \
		_oop_methodList pushBackUnique methodNameStr; \
		_oop_newMethodList pushBackUnique methodNameStr; \
		NAMESPACE setVariable [CLASS_METHOD_NAME_STR(_oop_classNameStr, methodNameStr),

	#define STATIC_METHOD_FILE(methodNameStr, path) \
		_oop_methodList pushBackUnique methodNameStr; \
		_oop_newMethodList pushBackUnique methodNameStr; \
		NAMESPACE setVariable [CLASS_METHOD_NAME_STR(_oop_classNameStr, methodNameStr), compile preprocessFileLineNumbers path]
#endif

// -----------------------------------------------------
// |       M E T H O D   P A R A M E T E R S           |
// -----------------------------------------------------

#define P_THISOBJECT ["_thisObject", "", [""]]
#define P_DEFAULT_PARAMS params [["_thisObject", "", [""]]]
#define P_THISCLASS ["_thisClass", "", [""]]
#define P_DEFAULT_STATIC_PARAMS params [["_thisObject", "", [""]]]
#define P_STRING(paramNameStr) [paramNameStr, "", [""]]
#define P_OBJECT(paramNameStr) [paramNameStr, objNull, [objNull]]
#define P_NUMBER(paramNameStr) [paramNameStr, 0, [0]]
#define P_SIDE(paramNameStr) [paramNameStr, WEST, [WEST]]
#define P_BOOL(paramNameStr) [paramNameStr, false, [false]]
#define P_BOOL_DEFAULT_TRUE(paramNameStr) [paramNameStr, true, [true]]
#define P_ARRAY(paramNameStr) [paramNameStr, [], [[]]]
#define P_POSITION(paramNameStr) [paramNameStr, [], [[]]]
#define P_CODE(paramNameStr) [paramNameStr, {}, [{}]]
#define P_DYNAMIC(paramNameStr) [paramNameStr, nil]

#define P_OOP_OBJECT(paramNameStr) P_STRING(paramNameStr)

// ----------------------------------------
// |              C L A S S               |
// ----------------------------------------

/*
 * Technical info:
 * First we initialize special members of the class, then we initialize new, delete and copy methods.
 * The name of this class is added to the hierarchy of its base class, if it's not "".
 * The methods of base class are copied to the methods of the derived class, except for "new" and "delete", because they will be called through the hierarchy anyway.
 */


#define CLASS(classNameStr, baseClassNameStr) \
call { \
diag_log TEXT_ format ["CLASS %1 <- %2", classNameStr, baseClassNameStr]; \
private _oop_classNameStr = classNameStr; \
SET_SPECIAL_MEM(_oop_classNameStr, NEXT_ID_STR, 0); \
private _oop_memList = []; \
private _oop_staticMemList = []; \
private _oop_parents = []; \
private _oop_methodList = []; \
private _oop_newMethodList = []; \
if (baseClassNameStr != "") then { \
	if (!([baseClassNameStr, __FILE__, __LINE__] call OOP_assert_class)) then { \
		private _msg = format ["Invalid base class for %1: %2", classNameStr, baseClassNameStr]; \
		FAILURE(_msg); \
	}; \
	_oop_parents = +GET_SPECIAL_MEM(baseClassNameStr, PARENTS_STR); _oop_parents pushBackUnique baseClassNameStr; \
	_oop_memList = +GET_SPECIAL_MEM(baseClassNameStr, MEM_LIST_STR); \
	_oop_staticMemList = +GET_SPECIAL_MEM(baseClassNameStr, STATIC_MEM_LIST_STR); \
	_oop_methodList = +GET_SPECIAL_MEM(baseClassNameStr, METHOD_LIST_STR); \
	private _oop_topParent = _oop_parents select ((count _oop_parents) - 1); \
	{ private _oop_methodCode = FORCE_GET_METHOD(_oop_topParent, _x); \
	FORCE_SET_METHOD(classNameStr, _x, _oop_methodCode); \
	_oop_methodCode = FORCE_GET_METHOD(_oop_topParent, INNER_METHOD_NAME_STR(_x)); \
	if (!isNil "_oop_methodCode") then { FORCE_SET_METHOD(classNameStr, INNER_METHOD_NAME_STR(_x), _oop_methodCode); }; \
	} forEach (_oop_methodList - ["new", "delete", "copy"]); \
}; \
SET_SPECIAL_MEM(_oop_classNameStr, PARENTS_STR, _oop_parents); \
SET_SPECIAL_MEM(_oop_classNameStr, MEM_LIST_STR, _oop_memList); \
SET_SPECIAL_MEM(_oop_classNameStr, STATIC_MEM_LIST_STR, _oop_staticMemList); \
SET_SPECIAL_MEM(_oop_classNameStr, METHOD_LIST_STR, _oop_methodList); \
PROFILER_COUNTER_INIT(_oop_classNameStr); \
METHOD("new") {} ENDMETHOD; \
METHOD("delete") {} ENDMETHOD; \
METHOD("copy") OOP_clone_default ENDMETHOD; \
METHOD("assign") OOP_assign_default ENDMETHOD; \
VARIABLE(OOP_PARENT_STR); \
VARIABLE(OOP_PUBLIC_STR);

// ----------------------------------------
// |           E N D C L A S S            |
// ----------------------------------------

/*
 * Technical info:
 * It just terminates the call block of the CLASS
 * No it doesn't do anything any more
 */

#ifdef OOP_SCRIPTNAME
#define ENDCLASS { \
private _fnc = missionNamespace getVariable CLASS_METHOD_NAME_STR(_oop_classNameStr, _x); \
private _fnc_array = toArray str _fnc; \
_fnc_array deleteAt 0; \
_fnc_array deleteAt ((count _fnc_array) - 1); \
private _fnc_str = (format ["private _fnc_scriptName = '%1';", _x]) + toString [10] + format ["#line 1 '%1'", CLASS_METHOD_NAME_STR(_oop_classNameStr, _x)] + toString [10] + (toString _fnc_array); \
missionNamespace setVariable [CLASS_METHOD_NAME_STR(_oop_classNameStr, _x), compile _fnc_str]; \
} forEach _oop_newMethodList; \
}
#else
#define ENDCLASS }
#endif

// ----------------------------------------------------------------------
// |        C O N S T R U C T O R  O F   E X I S T I N G   O B J E C T  |
// ----------------------------------------------------------------------

/*
 * Technical info:
 * Creates an object with given name, doesn't call its constructor.
 */

#define NEW_EXISTING(classNameStr, objNameStr) [] call { \
FORCE_SET_MEM(objNameStr, OOP_PARENT_STR, classNameStr); \
objNameStr \
}

// ----------------------------------------
// |        C O N S T R U C T O R         |
// ----------------------------------------

/*
 * Technical info:
 * Check the class name if needed.
 * Increase the object counter for this class.
 * Call all constructors of the base classes from base to derived classes.
 */

#ifdef OOP_ASSERT
#define CONSTRUCTOR_ASSERT_CLASS(classNameStr) if (!([classNameStr, __FILE__, __LINE__] call OOP_assert_class)) exitWith {format ["ERROR_NO_CLASS_%1", classNameStr]};
#else
#define CONSTRUCTOR_ASSERT_CLASS(classNameStr)
#endif

#define NEW(classNameStr, extraParams) ([classNameStr, extraParams] call OOP_new)

// -----------------------------------------------------------------------
// |        C O N S T R U C T O R  O F  P U B L I C   O B J E C T        |
// -----------------------------------------------------------------------

/*
 * Creates a 'public' object that will also exist across other computers in multiplayer.
 * Same as constructor, but also marks the object as public with a OOP_PUBLIC_STR variable.
 * It also transmits oop_parent and oop_public variables with publicVariable.
 * It doesn't mean the object's variables will be streamed across MP network, you still need to do it yourself.
 */

#define NEW_PUBLIC(classNameStr, extraParams) ([classNameStr, extraParams] call OOP_new_public) 

// ----------------------------------------
// |         D E S T R U C T O R          |
// ----------------------------------------

/*
 * Technical info:
 * Check object validity if needed.
 * Call all destructors of the base classes from derived classes to base classes.
 * Clean (set to nil) all members of this object.
 * If the object was global, also broadcast this.
 */

#ifdef OOP_ASSERT
#define DESTRUCTOR_ASSERT_OBJECT(objNameStr) if (!([objNameStr, __FILE__, __LINE__] call OOP_assert_object)) exitWith {};
#else
#define DESTRUCTOR_ASSERT_OBJECT(objNameStr)
#endif

#define DELETE(objNameStr) ([objNameStr] call OOP_delete)

// ----------------------------------------
// |              C L O N E               |
// ----------------------------------------

#define CLONE(objNameStr) ([objNameStr] call OOP_clone)

// ----------------------------------------
// |             A S S I G N              |
// ----------------------------------------

#define ASSIGN(destObjNameStr, srcObjNameStr) CALL_METHOD(destObjNameStr, "assign", [srcObjNameStr])

// ----------------------------------------
// |             U P D A T E              |
// ----------------------------------------
// Same as assign but copies only existing variables of an object (those that are not nil)
#define UPDATE(destObjNameStr, srcObjNameStr) [destObjNameStr, srcObjNameStr, false] call OOP_assign_default;


// ----------------------------------------
// |          S E R I A L I Z E           |
// ----------------------------------------
// Packs variables into an array and returns the array
#define SERIALIZE(objNameStr) ([objNameStr] call OOP_serialize)
#define SERIALIZED_CLASS_NAME(array) (array select 0)
#define SERIALIZED_OBJECT_NAME(array) (array select 1)

// ----------------------------------------
// |        D E S E R I A L I Z E         |
// ----------------------------------------
// Returns ref to the object passed in the array
// Object must exist before you can DESERIALIZE an array into it!
#define DESERIALIZE(objNameStr, array) ([objNameStr, array] call OOP_deserialize)

// ---------------------------------------------
// |         R E F   C O U N T I N G           |
// ---------------------------------------------

#define REF(objNameStr) CALLM0(objNameStr, "ref")
#define UNREF(objNameStr) CALLM0(objNameStr, "unref")

// ---------------------------------------------------
// |         T H R E A D I N G    U T I L S          |
// ---------------------------------------------------

#define CRITICAL_SECTION isNil

// ----------------------------------------------------------------------
// |                   L O G G I N G   M A C R O S                      |
// ----------------------------------------------------------------------

#define LOG_SCOPE(logScopeName) private _oop_logScope = logScopeName
#define LOG_0 if(!(isNil "_thisObject")) then {_thisObject} else { if(!(isNil "_thisClass")) then {_thisClass} else { if(!(isNil "_oop_logScope")) then { _oop_logScope } else { "NoClass" }}}
//#define LOG_1 _fnc_scriptName
#define LOG_1 "fnc"


#ifdef ADE
#define DUMP_CALLSTACK ade_dumpCallstack
#else
#define DUMP_CALLSTACK 
#endif

// If ofstream addon is globally enabled
#ifdef OFSTREAM_ENABLE
#define __OFSTREAM_OUT(fileName, text) ((ofstream_new fileName) ofstream_write(text))
#define WRITE_CRITICAL(text) ((ofstream_new "Critical.rpt") ofstream_write(text))
#else

#define __OFSTREAM_OUT(fileName, str) diag_log TEXT_ str
#define WRITE_CRITICAL(str)

#endif

#define _OFSTREAM_FILE OFSTREAM_FILE

#ifdef OFSTREAM_FILE
#define WRITE_LOG(str) __OFSTREAM_OUT(OFSTREAM_FILE, str)
#else
#define WRITE_LOG(str) diag_log TEXT_ str
#endif

#ifdef OOP_PROFILE
#define OOP_PROFILE_MSG(str, a) private _o_str = format ["[%1.%2] PROFILE: %3",LOG_0, LOG_1, format ([str]+a)]; __OFSTREAM_OUT("oop_profile.rpt", _o_str)
#define OOP_PROFILE_0(str) private _o_str = format ["[%1.%2] PROFILE: %3", LOG_0, LOG_1, str]; __OFSTREAM_OUT("oop_profile.rpt", _o_str)
#define OOP_PROFILE_1(str, a) private _o_str = format ["[%1.%2] PROFILE: %3",LOG_0, LOG_1, format [str, a]]; __OFSTREAM_OUT("oop_profile.rpt", _o_str)
#define OOP_PROFILE_2(str, a, b) private _o_str = format ["[%1.%2] PROFILE: %3", LOG_0, LOG_1, format [str, a, b]]; __OFSTREAM_OUT("oop_profile.rpt", _o_str)
#define OOP_PROFILE_3(str, a, b, c) private _o_str = format ["[%1.%2] PROFILE: %3", LOG_0, LOG_1, format [str, a, b, c]]; __OFSTREAM_OUT("oop_profile.rpt", _o_str)
#define OOP_PROFILE_4(str, a, b, c, d) private _o_str = format ["[%1.%2] PROFILE: %3", LOG_0, LOG_1, format [str, a, b, c, d]]; __OFSTREAM_OUT("oop_profile.rpt", _o_str)
#define OOP_PROFILE_5(str, a, b, c, d, e) private _o_str = format ["[%1.%2] PROFILE: %3", LOG_0, LOG_1, format [str, a, b, c, d, e]]; __OFSTREAM_OUT("oop_profile.rpt", _o_str)
#else
#define OOP_PROFILE_MSG(str, a)
#define OOP_PROFILE_0(str)
#define OOP_PROFILE_1(str, a)
#define OOP_PROFILE_2(str, a, b)
#define OOP_PROFILE_3(str, a, b, c)
#define OOP_PROFILE_4(str, a, b, c, d)
#define OOP_PROFILE_5(str, a, b, c, d, e)
#endif

#ifdef OOP_INFO
#define OOP_INFO_MSG(str, a) private _o_str = format ["[%1.%2] INFO: %3",LOG_0, LOG_1, format ([str]+a)]; WRITE_LOG(_o_str)
#define OOP_INFO_0(str) private _o_str = format ["[%1.%2] INFO: %3", LOG_0, LOG_1, str]; WRITE_LOG(_o_str)
#define OOP_INFO_1(str, a) private _o_str = format ["[%1.%2] INFO: %3",LOG_0, LOG_1, format [str, a]]; WRITE_LOG(_o_str)
#define OOP_INFO_2(str, a, b) private _o_str = format ["[%1.%2] INFO: %3", LOG_0, LOG_1, format [str, a, b]]; WRITE_LOG(_o_str)
#define OOP_INFO_3(str, a, b, c) private _o_str = format ["[%1.%2] INFO: %3", LOG_0, LOG_1, format [str, a, b, c]]; WRITE_LOG(_o_str)
#define OOP_INFO_4(str, a, b, c, d) private _o_str = format ["[%1.%2] INFO: %3", LOG_0, LOG_1, format [str, a, b, c, d]]; WRITE_LOG(_o_str)
#define OOP_INFO_5(str, a, b, c, d, e) private _o_str = format ["[%1.%2] INFO: %3", LOG_0, LOG_1, format [str, a, b, c, d, e]]; WRITE_LOG(_o_str)
#else
#define OOP_INFO_MSG(str, a)
#define OOP_INFO_0(str)
#define OOP_INFO_1(str, a)
#define OOP_INFO_2(str, a, b)
#define OOP_INFO_3(str, a, b, c)
#define OOP_INFO_4(str, a, b, c, d)
#define OOP_INFO_5(str, a, b, c, d, e)
#endif

#ifdef OOP_WARNING
#define OOP_WARNING_MSG(str, a) private _o_str = format ["[%1.%2] WARNING: %3", LOG_0, LOG_1, format ([str]+a)]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_WARNING_0(str) private _o_str = format ["[%1.%2] WARNING: %3", LOG_0, LOG_1, str]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_WARNING_1(str, a) private _o_str = format ["[%1.%2] WARNING: %3", LOG_0, LOG_1, format [str, a]]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_WARNING_2(str, a, b) private _o_str = format ["[%1.%2] WARNING: %3", LOG_0, LOG_1, format [str, a, b]]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_WARNING_3(str, a, b, c) private _o_str = format ["[%1.%2] WARNING: %3", LOG_0, LOG_1, format [str, a, b, c]]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_WARNING_4(str, a, b, c, d) private _o_str = format ["[%1.%2] WARNING: %3", LOG_0, LOG_1, format [str, a, b, c, d]]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_WARNING_5(str, a, b, c, d, e) private _o_str = format ["[%1.%2] WARNING: %3", LOG_0, LOG_1, format [str, a, b, c, d, e]]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#else
#define OOP_WARNING_MSG(str, a)
#define OOP_WARNING_0(str)
#define OOP_WARNING_1(str, a)
#define OOP_WARNING_2(str, a, b)
#define OOP_WARNING_3(str, a, b, c)
#define OOP_WARNING_4(str, a, b, c, d)
#define OOP_WARNING_5(str, a, b, c, d, e)
#endif

#ifdef OOP_ERROR
#define OOP_ERROR_MSG(str, a) private _o_str = format ["[%1.%2] ERROR: %3", LOG_0, LOG_1, format ([str]+a) ]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_ERROR_0(str) private _o_str = format ["[%1.%2] ERROR: %3", LOG_0, LOG_1, str]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_ERROR_1(str, a) private _o_str = format ["[%1.%2] ERROR: %3", LOG_0, LOG_1, format [str, a]]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_ERROR_2(str, a, b) private _o_str = format ["[%1.%2] ERROR: %3", LOG_0, LOG_1, format [str, a, b]]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_ERROR_3(str, a, b, c) private _o_str = format ["[%1.%2] ERROR: %3", LOG_0, LOG_1, format [str, a, b, c]]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_ERROR_4(str, a, b, c, d) private _o_str = format ["[%1.%2] ERROR: %3", LOG_0, LOG_1, format [str, a, b, c, d]]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#define OOP_ERROR_5(str, a, b, c, d, e) private _o_str = format ["[%1.%2] ERROR: %3", LOG_0, LOG_1, format [str, a, b, c, d, e]]; WRITE_LOG(_o_str); WRITE_CRITICAL(_o_str)
#else
#define OOP_ERROR_MSG(str, a)
#define OOP_ERROR_0(str)
#define OOP_ERROR_1(str, a)
#define OOP_ERROR_2(str, a, b)
#define OOP_ERROR_3(str, a, b, c)
#define OOP_ERROR_4(str, a, b, c, d)
#define OOP_ERROR_5(str, a, b, c, d, e)
#endif

#ifdef OOP_DEBUG
#define OOP_DEBUG_MSG(str, a) private _o_str = format ["[%1.%2] DEBUG: %3", LOG_0, LOG_1, format ([str]+a) ]; WRITE_LOG(_o_str)
#define OOP_DEBUG_0(str) private _o_str = format ["[%1.%2] DEBUG: %3", LOG_0, LOG_1, str]; WRITE_LOG(_o_str)
#define OOP_DEBUG_1(str, a) private _o_str = format ["[%1.%2] DEBUG: %3", LOG_0, LOG_1, format [str, a]]; WRITE_LOG(_o_str)
#define OOP_DEBUG_2(str, a, b) private _o_str = format ["[%1.%2] DEBUG: %3", LOG_0, LOG_1, format [str, a, b]]; WRITE_LOG(_o_str)
#define OOP_DEBUG_3(str, a, b, c) private _o_str = format ["[%1.%2] DEBUG: %3", LOG_0, LOG_1, format [str, a, b, c]]; WRITE_LOG(_o_str)
#define OOP_DEBUG_4(str, a, b, c, d) private _o_str = format ["[%1.%2] DEBUG: %3", LOG_0, LOG_1, format [str, a, b, c, d]]; WRITE_LOG(_o_str)
#define OOP_DEBUG_5(str, a, b, c, d, e) private _o_str = format ["[%1.%2] DEBUG: %3", LOG_0, LOG_1, format [str, a, b, c, d, e]]; WRITE_LOG(_o_str)
#else
#define OOP_DEBUG_MSG(str, a)
#define OOP_DEBUG_0(str)
#define OOP_DEBUG_1(str, a)
#define OOP_DEBUG_2(str, a, b)
#define OOP_DEBUG_3(str, a, b, c)
#define OOP_DEBUG_4(str, a, b, c, d)
#define OOP_DEBUG_5(str, a, b, c, d, e)
#endif

// ----------------------------------------------------------------------
// |                A S S E R T I O N S  A N D   C H E C K S            |
// ----------------------------------------------------------------------
// ASSERT_OBJECT_CLASS(objNameStr, classNameStr)
// Exits current scope if provided object's class doesn't match specified class
#ifdef OOP_ASSERT
	#define ASSERT_OBJECT(objNameStr) ([objNameStr, __FILE__, __LINE__] call OOP_assert_object)
	#define ASSERT_OBJECT_CLASS(objNameStr, classNameStr) ([objNameStr, classNameStr, __FILE__, __LINE__] call OOP_assert_objectClass)
	#define ASSERT_MSG(condition, msg) \
		if (!(condition)) then { \
			private _str = str({ condition; }); \
			OOP_ERROR_2("Assertion failed: %2", _str, msg); \
			DUMP_CALLSTACK; \
			throw [__FILE__, __LINE__, msg]; \
		}
	#define ASSERT(condition) \
		if (!(condition)) then { \
			private _str = str({ condition; }); \
			OOP_ERROR_1("Assertion failed (%1)", _str); \
			DUMP_CALLSTACK; \
			throw [__FILE__, __LINE__, msg]; \
		}
	#define FAILURE(msg) \
		OOP_ERROR_1("Failure: %1", msg); \
		DUMP_CALLSTACK; \
		throw [__FILE__, __LINE__, msg]
#else
	#define ASSERT_OBJECT(object)
	#define ASSERT_OBJECT_CLASS(objNameStr, classNameStr)
	#define ASSERT_MSG(condition, msg)
	#define ASSERT(condition)
	#define FAILURE(msg)
#endif

// Returns true if given object is public, i.e. was created with NEW_PUBLIC
#define IS_PUBLIC(objNameStr) (! (isNil {GET_MEM(objNameStr, OOP_PUBLIC_STR)} ) )

// ----------------------------------------------------------------------
// |                               M I S C                              |
// ----------------------------------------------------------------------
#define ARG ,

// Index find and findIf return when they don't find anything
#define NOT_FOUND -1
// For use with sort
#define ASCENDING true
#define DESCENDING false
// Is the object handle valid?
//#define NOT_NULL_OBJECT(object) ((object isEqualType "") and {!(object isEqualTo "")})
#define IS_NULL_OBJECT(object) (!(object isEqualType "") or {object isEqualTo ""})
// Value to assign to an object handle to indicate it is deliberately invalid.
#define NULL_OBJECT objNull