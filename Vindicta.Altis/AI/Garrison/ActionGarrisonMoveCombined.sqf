#include "common.hpp"
/*
Garrison moves on available vehicles
*/

#define pr private

#define THIS_ACTION_NAME "ActionGarrisonMoveCombined"

CLASS(THIS_ACTION_NAME, "ActionGarrison")


	VARIABLE("pos"); // The destination position
	VARIABLE("radius"); // Completion radius
	VARIABLE("virtualRoute"); // VirtualRoute object

	// ------------ N E W ------------
	METHOD("new") {
		params [P_THISOBJECT, P_OOP_OBJECT("_AI"), P_ARRAY("_parameters")];
		
		// Unpack position
		pr _pos = CALLSM2("Action", "getParameterValue", _parameters, TAG_POS);
		pr _loc = "";
		if (_pos isEqualType []) then {
			T_SETV("pos", _pos); // Set value if array if passed
			pr _locAndDist = CALLSM1("Location", "getNearestLocation", _pos);
			_loc = _locAndDist select 0;
		} else {
			// Otherwise the location object was passed probably, get pos from location object
			_loc = _pos;
			pr _locPos = CALLM0(_loc, "getPos");
			T_SETV("pos", _locPos);
		};
		
		// Unpack radius
		pr _radius = CALLSM3("Action", "getParameterValue", _parameters, TAG_MOVE_RADIUS, -1);
		if (_radius == -1) then {
			// todo Try to figure out completion radius from location
			//pr _radius = CALLM0(_loc, "getBoundingRadius"); // there is no such function
			// Just use 100 meters for now
			T_SETV("radius", 100);
		} else {
			T_SETV("radius", _radius);
		};

		T_CALLM0("createVirtualRoute");

	} ENDMETHOD;

	METHOD("delete") {
		params [P_THISOBJECT];

		// Delete the virtual route object
		pr _vr = T_GETV("virtualRoute");
		if (_vr != NULL_OBJECT) then {
			DELETE(_vr);
		};

	} ENDMETHOD;
	
	// logic to run when the goal is activated
	METHOD("activate") {
		params [P_THISOBJECT, P_BOOL("_instant")];
		
		OOP_INFO_0("ACTIVATE");
		
		// Give waypoint to the vehicle group
		pr _gar = T_GETV("gar");
		pr _AI = T_GETV("AI");
		pr _pos = T_GETV("pos");
		pr _radius = T_GETV("radius");
		
		// Reset current location of this garrison
		CALLM0(_gar, "detachFromLocation");
		pr _ws = GETV(_AI, "worldState");
		[_ws, WSP_GAR_LOCATION, ""] call ws_setPropertyValue;
		pr _pos = CALLM0(_gar, "getPos");
		[_ws, WSP_GAR_POSITION, _pos] call ws_setPropertyValue;

		// Give group goals
		pr _vehGroups = CALLM1(_gar, "findGroupsByType", GROUP_TYPE_VEH_NON_STATIC) + CALLM1(_gar, "findGroupsByType", GROUP_TYPE_VEH_STATIC);
		if (count _vehGroups > 1) then {
			OOP_ERROR_0("More than one vehicle group in the garrison!");
		};
		
		// No instant move for this action we we track unspawned progress already, and groups should be formed up and 
		// mounted already before it is called.
		pr _commonArgs = []; //[[TAG_INSTANT, _instant]];
		{
			pr _group = _x;
			pr _groupAI = CALLM0(_x, "getAI");
			
			// Add new goal to move
			pr _args = ["GoalGroupMoveGroundVehicles", 0, [
				[TAG_POS, _pos],
				[TAG_MOVE_RADIUS, _radius],
				[TAG_MAX_SPEED_KMH, 11]
			] + _commonArgs, _AI];
			CALLM2(_groupAI, "postMethodAsync", "addExternalGoal", _args);
		} forEach _vehGroups;

		// Give goals to infantry groups
		pr _infGroups = CALLM1(_gar, "findGroupsByType", [GROUP_TYPE_IDLE ARG GROUP_TYPE_PATROL]);
		{
			pr _groupAI = CALLM0(_x, "getAI");

			// Add new goal to move
			pr _args = ["GoalGroupInfantryFollowGroundVehicles", 0, _commonArgs, _AI];
			CALLM2(_groupAI, "postMethodAsync", "addExternalGoal", _args);
		} forEach _infGroups;

		// Set state
		T_SETV("state", ACTION_STATE_ACTIVE);

		// Return ACTIVE state
		ACTION_STATE_ACTIVE

	} ENDMETHOD;

	// logic to run each update-step
	METHOD("process") {
		params [P_THISOBJECT];

		pr _gar = T_GETV("gar");
		if (!CALLM0(_gar, "isSpawned")) then {
			pr _state = T_GETV("state");

			// Create a Virtual Route if it doesnt exist yet
			pr _vr = T_GETV("virtualRoute");
			if (_vr == NULL_OBJECT) then {
				_vr = T_CALLM0("createVirtualRoute");
			};

			if (_state == ACTION_STATE_INACTIVE) then {
				CALLM0(_vr, "start");
				CALLM0(_gar, "detachFromLocation");
				_state = ACTION_STATE_ACTIVE;
			};

			// Process the virtual convoy
			if (_state == ACTION_STATE_ACTIVE) then {
				// Run process of the virtual route and update position of the garrison
				CALLM0(_vr, "process");
				if(GETV(_vr, "calculated")) then {
					pr _pos = CALLM0(_vr, "getPos");
					pr _AI = T_GETV("AI");
					CALLM1(_AI, "setPos", _pos);

					// Succeed the action if the garrison is close enough to its destination
					if (_pos distance2D T_GETV("pos") < T_GETV("radius") or {GETV(_vr, "complete")}) then {
						_state = ACTION_STATE_COMPLETED;
					};
				} else { 
					if(GETV(_vr, "failed")) then {
						T_PRVAR(gar);
						pr _garPos = CALLM0(_gar, "getPos");
						T_PRVAR(pos);
						OOP_WARNING_MSG("Virtual Route from %1 to %2 failed, distance remaining : %3", [_garPos ARG _pos ARG _pos distance2D _garPos]);
						// We assume failure is due to no road between the locations
						// TODO: Return specific problem from VirtualRoute
						_state = ACTION_STATE_COMPLETED;
					};
				};
			};

			T_SETV("state", _state);
			_state
		} else {
			// Delete the Virtual Route object if it exists, we don't need it any more
			pr _vr = T_GETV("virtualRoute");
			if (_vr != NULL_OBJECT) then {
				DELETE(_vr);
				T_SETV("virtualRoute", "");
			};

			// Fail if not everyone is in vehicles
			pr _crewIsMounted = T_CALLM0("isCrewInVehicle");
			OOP_INFO_1("Crew is in vehicles: %1", _crewIsMounted);
			if (! _crewIsMounted) exitWith {
				OOP_INFO_0("ACTION FAILED because crew is not in vehicles");
				T_SETV("state", ACTION_STATE_FAILED);
				ACTION_STATE_FAILED
			};
			
			pr _state = T_CALLM0("activateIfInactive");
			
			if (_state == ACTION_STATE_ACTIVE) then {
			
				pr _gar = T_GETV("gar");
				pr _AI = T_GETV("AI");

				// Update position of this garrison object
				pr _units = CALLM0(_gar, "getUnits");
				pr _pos = T_GETV("pos");
				pr _index = _units findIf {CALLM0(_x, "isAlive")};
				if (_index != -1) then {
					pr _unit = _units select _index;
					pr _hO = CALLM0(_unit, "getObjectHandle");
					_pos = getPos _hO;
				};
				CALLM1(_AI, "setPos", _pos);
			
				pr _args = [GROUP_TYPE_VEH_NON_STATIC, GROUP_TYPE_VEH_STATIC];
				pr _vehGroups = CALLM1(_gar, "findGroupsByType", _args);
				pr _infGroups = CALLM(_gar, "findGroupsByType", [GROUP_TYPE_IDLE ARG GROUP_TYPE_PATROL]);
				
				switch true do {
					// Fail if any group has failed
					case (CALLSM3("AI_GOAP", "anyAgentFailedExternalGoal", _vehGroups, "GoalGroupMoveGroundVehicles", "")): {
						_state = ACTION_STATE_FAILED
					};
					case (CALLSM3("AI_GOAP", "anyAgentFailedExternalGoal", _infGroups, "GoalGroupInfantryFollowGroundVehicles", "")): {
						_state = ACTION_STATE_FAILED
					};
					// Succeed if all groups have completed the goal
					case (CALLSM3("AI_GOAP", "allAgentsCompletedExternalGoalRequired", _vehGroups, "GoalGroupMoveGroundVehicles", "")): {
						OOP_INFO_0("All groups have arrived");
						
						// Set pos world state property
						// todo fix this, implement AIGarrison.setVehiclesPos function
						//pr _ws = GETV(_AI, "worldState");
						//[_ws, WSP_GAR_POSITION, _pos] call ws_setPropertyValue;
						//[_ws, WSP_GAR_VEHICLES_POSITION, _pos] call ws_setPropertyValue;
						_state = ACTION_STATE_COMPLETED
					};
				};
			};
			
			// Return the current state
			T_SETV("state", _state);
			_state
		};
	} ENDMETHOD;
	
	// Returns true if everyone is in vehicles
	METHOD("isCrewInVehicle") {
		params [P_THISOBJECT];
		pr _AI = T_GETV("AI");
		pr _ws = GETV(_AI, "worldState");
		
		pr _return = 	([_ws, WSP_GAR_ALL_CREW_MOUNTED] call ws_getPropertyValue);
		
		_return
	} ENDMETHOD;
	
	// // logic to run when the action is satisfied
	// METHOD("terminate") {
	// 	params [P_THISOBJECT];
		
	// 	pr _gar = T_GETV("gar");

	// 	// Bail if not spawned
	// 	if (!CALLM0(_gar, "isSpawned")) exitWith {};

	// 	// Terminate given goals
	// 	pr _allGroups = CALLM0(_gar, "getGroups");
	// 	{
	// 		pr _groupAI = CALLM0(_x, "getAI");
	// 		CALLM(_groupAI, "postMethodAsync", ["deleteExternalGoal" ARG [ "GoalGroupMoveGroundVehicles" ARG ""]]);
	// 		CALLM(_groupAI, "postMethodAsync", ["deleteExternalGoal" ARG [ "GoalGroupInfantryFollowGroundVehicles" ARG ""]]);
	// 	} forEach _allGroups;
		
	// } ENDMETHOD;

	/* protected override */ METHOD("onGarrisonDespawned") {
		params [P_THISOBJECT];

		// Create a new VirtualRoute since old one might be invalid
		T_CALLM0("createVirtualRoute");

		T_CALLCM0("ActionGarrison", "onGarrisonDespawned");
	} ENDMETHOD;

	// Creates a new VirtualRoute object, deletes the old one
	/* private */ METHOD("createVirtualRoute") {
		params [P_THISOBJECT];


		// Delete it if it exists already
		private _vr = T_GETV("virtualRoute");
		if(!isNil "_vr" && {_vr != NULL_OBJECT}) then {
			DELETE(_vr);
		};

		// Create a new virtual route
		// pr _gar = T_GETV("gar");
		// pr _args = [CALLM0(_gar, "getPos"), T_GETV("pos"), -1, "", {3}, false]; // Set 3m/s speed
		// _vr = NEW("VirtualRoute", _args);
		// T_SETV("virtualRoute", _vr);

		// _vr
		private _gar = T_GETV("gar");

		private _side = CALLM(_gar, "getSide", []);
		private _cmdr = CALL_STATIC_METHOD("AICommander", "getAICommander", [_side]);

		private _threatCostFn = {
			params ["_base_cost", "_current", "_next", "_startRoute", "_goalRoute", "_callbackArgs"];
			_callbackArgs params ["_cmdr"];
			private _threat = CALLM(_cmdr, "getThreat", [getPos _next]);
			_base_cost + _threat * 20
		};

		private _args = [CALLM0(_gar, "getPos"), T_GETV("pos"), -1, _threatCostFn, {3}, [_cmdr], false, true];
		_vr = NEW("VirtualRoute", _args);
		T_SETV("virtualRoute", _vr);

		_vr
	} ENDMETHOD;

	/* public override */ METHOD("spawn") {
		params [P_THISOBJECT];

		pr _gar = T_GETV("gar");

		// Spawn vehicle groups on the road according to convoy positions
		pr _vr = T_GETV("virtualRoute");
		if (_vr == NULL_OBJECT) exitWith {false}; // Perform standard spawning if there is no virtual route for some reason (why???)

		// Count all vehicles in garrison
		pr _nVeh = count CALLM0(_gar, "getVehicleUnits");
		pr _posAndDir = if(!GETV(_vr, "calculated") || GETV(_vr, "failed")) then {
			pr _vals = [];
			pr _garPos = CALLM0(_gar, "getPos");
			for "_i" from 1 to _nVeh do {
				_vals pushBack [_garPos, 0];
			};
			_vals
		} else {
			CALLM1(_vr, "getConvoyPositions", _nVeh)
		};
		//reverse _posAndDir;

		// Bail if we have failed to get positions
		if ((count _posAndDir) != _nVeh) exitWith {false};

		// Iterate through all groups
		pr _currentIndex = 0;
		pr _groups = CALLM0(_gar, "getGroups");
		{
			pr _nVehThisGroup = count CALLM0(_x, "getVehicleUnits");
			if (_nVehThisGroup > 0) then {
				pr _posAndDirThisGroup = _posAndDir select [_currentIndex, _nVehThisGroup];
				CALLM1(_x, "spawnVehiclesOnRoad", _posAndDirThisGroup);

				// Make leader the first human in the group
				CALLM0(_x, "_selectNextLeader");

				_currentIndex = _currentIndex + _nVehThisGroup;
			} else {
				pr _posAndDirThisGroup = _posAndDir select [0, 1];
				CALLM1(_x, "spawnVehiclesOnRoad", _posAndDirThisGroup);
			};
		} forEach _groups;

		// todo what happens with ungrouped units?? Why are there even ungrouped units at this point???

		true
	} ENDMETHOD;

	// // Handle units/groups added/removed
	// METHOD("handleGroupsAdded") {
	// 	params [P_THISOBJECT, P_ARRAY("_groups")];
		
	// 	T_SETV("state", ACTION_STATE_REPLAN);
	// } ENDMETHOD;

	// METHOD("handleGroupsRemoved") {
	// 	params [P_THISOBJECT, P_ARRAY("_groups")];
		
	// 	T_SETV("state", ACTION_STATE_REPLAN);
	// } ENDMETHOD;
	
	// METHOD("handleUnitsRemoved") {
	// 	params [P_THISOBJECT, P_ARRAY("_units")];
		
	// 	T_SETV("state", ACTION_STATE_REPLAN);
	// } ENDMETHOD;
	
	// METHOD("handleUnitsAdded") {
	// 	params [P_THISOBJECT, P_ARRAY("_units")];
		
	// 	T_SETV("state", ACTION_STATE_REPLAN);
	// } ENDMETHOD;

ENDCLASS;