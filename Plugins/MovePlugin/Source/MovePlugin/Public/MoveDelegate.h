#pragma once
#include "CoreUObject.h"

/** 
 * Converted Controller Data.
 * Contains converted raw and integrated 
 * sixense controller data.
 */
typedef struct _moveControllerDataUE{
	//raw converted
	FVector position;	//in hydra units (cm)
	FQuat rotation;		//raw, may need conversion
	FVector2D joystick;
	float trigger;
	uint32 buttons;
	uint8 sequence_number;
	uint16 firmware_revision;
	uint16 hardware_revision;
	uint16 packet_type;
	uint16 magnetic_frequency;
	bool enabled;
	int32 controller_index;
	bool is_docked;
	uint8 which_hand;
	bool hemi_tracking_enabled;

	//added values
	FVector velocity;
	FVector acceleration;
	bool trigger_pressed;
} moveControllerDataUE;

typedef struct _moveAllControllerDataUE{
	moveControllerDataUE controllers[4];		// current sdk max supported
	int32 enabledCount;
	bool available;
} moveAllControllerDataUE;


class MoveDelegate
{
	friend class FMovePlugin;
public:
	//Namespace Move for variables and functions, EventMove for events.

	/** Latest will always contain the freshest controller data */
	moveAllControllerDataUE* MoveLatestData;

	/** Holds last 10 controller captures, useful for gesture recognition*/
	moveAllControllerDataUE MoveHistoryData[10];

	/** Event Emitters, override to receive notifications.
	 *	int32 controller is the controller index (typically 0 or 1 for hydra) 
	 *	Call HydraWhichHand(controller index) to determine which hand is being held (determined and reset on docking)
	 */
	virtual void EventMovePluggedIn();								//called once enabledCount == 2
	virtual void EventMoveUnplugged();								//called once enabledCount == 0
	virtual void EventMoveControllerEnabled(int32 controller);		//called for each controller
	virtual void EventMoveControllerDisabled(int32 controller);	//called for each controller

	virtual void EventMoveDocked(int32 controller);
	virtual void EventMoveUndocked(int32 controller);

	virtual void EventMoveAnyButtonPressed(int32 controller);
	virtual void EventMoveB1Pressed(int32 controller);
	virtual void EventMoveB1Released(int32 controller);
	virtual void EventMoveB2Pressed(int32 controller);
	virtual void EventMoveB2Released(int32 controller);
	virtual void EventMoveB3Pressed(int32 controller);
	virtual void EventMoveB3Released(int32 controller);
	virtual void EventMoveB4Pressed(int32 controller);
	virtual void EventMoveB4Released(int32 controller);
	virtual void EventMoveTriggerPressed(int32 controller);
	virtual void EventMoveTriggerReleased(int32 controller);
	virtual void EventMoveTriggerChanged(int32 controller, float value);	//Range 0-1.0
	virtual void EventMoveBumperPressed(int32 controller);
	virtual void EventMoveBumperReleased(int32 controller);
	virtual void EventMoveJoystickPressed(int32 controller);
	virtual void EventMoveJoystickReleased(int32 controller);
	virtual void EventMoveStartPressed(int32 controller);
	virtual void EventMoveStartReleased(int32 controller);

	virtual void EventMoveJoystickMoved(int32 controller, FVector2D movement);	//Range 0-1.0, 0-1.0
	
	/*	Movement in move units (should it be converted?)
	*	triggered whenever a controller is not docked. */
	virtual void EventMoveControllerMoved(int32 controller,						
								FVector position, FVector velocity, FVector acceleration, 
								FRotator rotation);

	//** Manual Check */
	virtual bool MoveIsAvailable();
	//virtual int32 HydraWhichHand(int32 controller);	//call to determine which hand the controller is held in. Determined and reset on controller docking.

private:
	//Do not call, this will be called by the plugin, namespace InternalMove
	void InternalMoveControllerTick(float DeltaTime);
	void InternalMoveUpdateAllData();
	void InternalMoveCheckEnabledCount(bool* plugNotChecked);
};