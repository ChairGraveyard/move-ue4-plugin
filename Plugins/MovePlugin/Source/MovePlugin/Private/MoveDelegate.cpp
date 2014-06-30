#pragma once

#include "MovePluginPrivatePCH.h"
#include "MoveDelegate.h"
#include "sixense.h"

/** Empty Event Functions, no Super call required, because they don't do anything! */
void MoveDelegate::EventMoveControllerEnabled(int32 controller){}
void MoveDelegate::EventMoveControllerDisabled(int32 controller){}
void MoveDelegate::EventMovePluggedIn(){}
void MoveDelegate::EventMoveUnplugged(){}
void MoveDelegate::EventMoveDocked(int32 controller){}
void MoveDelegate::EventMoveUndocked(int32 controller){}
void MoveDelegate::EventMoveAnyButtonPressed(int32 controller){}
void MoveDelegate::EventMoveB1Pressed(int32 controller){}
void MoveDelegate::EventMoveB1Released(int32 controller){}
void MoveDelegate::EventMoveB2Pressed(int32 controller){}
void MoveDelegate::EventMoveB2Released(int32 controller){}
void MoveDelegate::EventMoveB3Pressed(int32 controller){}
void MoveDelegate::EventMoveB3Released(int32 controller){}
void MoveDelegate::EventMoveB4Pressed(int32 controller){}
void MoveDelegate::EventMoveB4Released(int32 controller){}
void MoveDelegate::EventMoveTriggerPressed(int32 controller){}
void MoveDelegate::EventMoveTriggerReleased(int32 controller){}
void MoveDelegate::EventMoveTriggerChanged(int32 controller, float value){}
void MoveDelegate::EventMoveBumperPressed(int32 controller){}
void MoveDelegate::EventMoveBumperReleased(int32 controller){}
void MoveDelegate::EventMoveJoystickPressed(int32 controller){}
void MoveDelegate::EventMoveJoystickReleased(int32 controller){}
void MoveDelegate::EventMoveStartPressed(int32 controller){}
void MoveDelegate::EventMoveStartReleased(int32 controller){}

void MoveDelegate::EventMoveJoystickMoved(int32 controller, FVector2D movement){};
void MoveDelegate::EventMoveControllerMoved(int32 controller,
	FVector position, FVector velocity, FVector acceleration,
	FRotator rotation){};

/** Move Internal Functions, called by plugin.*/
void MoveDelegate::InternalMoveUpdateAllData()
{
	MoveHistoryData[9] = MoveHistoryData[8];
	MoveHistoryData[8] = MoveHistoryData[7];
	MoveHistoryData[7] = MoveHistoryData[6];
	MoveHistoryData[6] = MoveHistoryData[5];
	MoveHistoryData[5] = MoveHistoryData[4];
	MoveHistoryData[4] = MoveHistoryData[3];
	MoveHistoryData[3] = MoveHistoryData[2];
	MoveHistoryData[2] = MoveHistoryData[1];
	MoveHistoryData[1] = MoveHistoryData[0];
	//NB: MoveHistoryData[0] = *MoveLatestData gets updated after the tick to take in integrated data
}

void MoveDelegate::InternalMoveCheckEnabledCount(bool* plugNotChecked)
{
	if (!*plugNotChecked) return;

	sixenseAllControllerDataUE* previous = &MoveHistoryData[0];
	int32 oldCount = previous->enabledCount;
	int32 count = MoveLatestData->enabledCount;
	if (oldCount != count)
	{
		if (count == 2)	//Move controller number, STEM behavior undefined.
		{
			EventMovePluggedIn();
			*plugNotChecked = false;
		}
		else if (count == 0)
		{
			EventMoveUnplugged();
			*plugNotChecked = false;
		}
	}
}

/** Internal Tick - Called by the Plugin */
void MoveDelegate::InternalMoveControllerTick(float DeltaTime)
{
	//Update Data History
	InternalMoveUpdateAllData();

	sixenseControllerDataUE* controller;
	sixenseControllerDataUE* previous;
	bool plugNotChecked = true;

	//Trigger any delegate events
	for (int i = 0; i < 4; i++)
	{
		controller = &MoveLatestData->controllers[i];
		previous = &MoveHistoryData[0].controllers[i];

		//If it is enabled run through all the event notifications and data integration
		if (controller->enabled)
		{
			//Enable Check
			if (controller->enabled != previous->enabled)
			{
				InternalMoveCheckEnabledCount(&plugNotChecked);
				EventMoveControllerEnabled(i);
			}

			//Docking
			if (controller->is_docked != previous->is_docked)
			{
				if (controller->is_docked)
				{
					EventMoveDocked(i);
				}
				else{
					EventMoveUndocked(i);
				}
			}

			//** Buttons */

			//Trigger
			if (controller->trigger < 0.5)
			{
				controller->trigger_pressed = false;
			}
			else{
				controller->trigger_pressed = true;
			}

			if (controller->trigger != previous->trigger)
			{
				EventMoveTriggerChanged(i, controller->trigger);

				if (controller->trigger_pressed != previous->trigger_pressed)
				{
				
					if (controller->trigger_pressed)
					{
						EventMoveAnyButtonPressed(i);
						EventMoveTriggerPressed(i);
					}
					else{
						EventMoveTriggerReleased(i);
					}
				}
			}

			//Bumper
			if ((controller->buttons & SIXENSE_BUTTON_BUMPER) != (previous->buttons & SIXENSE_BUTTON_BUMPER))
			{
				if ((controller->buttons & SIXENSE_BUTTON_BUMPER) == SIXENSE_BUTTON_BUMPER)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveBumperPressed(i);
				}
				else{
					EventMoveBumperReleased(i);
				}
			}

			//B1
			if ((controller->buttons & SIXENSE_BUTTON_1) != (previous->buttons & SIXENSE_BUTTON_1))
			{
				if ((controller->buttons & SIXENSE_BUTTON_1) == SIXENSE_BUTTON_1)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveB1Pressed(i);
				}
				else{
					EventMoveB1Released(i);
				}
			}
			//B2
			if ((controller->buttons & SIXENSE_BUTTON_2) != (previous->buttons & SIXENSE_BUTTON_2))
			{
				if ((controller->buttons & SIXENSE_BUTTON_2) == SIXENSE_BUTTON_2)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveB2Pressed(i);
				}
				else{
					EventMoveB2Released(i);
				}
			}
			//B3
			if ((controller->buttons & SIXENSE_BUTTON_3) != (previous->buttons & SIXENSE_BUTTON_3))
			{
				if ((controller->buttons & SIXENSE_BUTTON_3) == SIXENSE_BUTTON_3)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveB3Pressed(i);
				}
				else{
					EventMoveB3Released(i);
				}
			}
			//B4
			if ((controller->buttons & SIXENSE_BUTTON_4) != (previous->buttons & SIXENSE_BUTTON_4))
			{
				if ((controller->buttons & SIXENSE_BUTTON_4) == SIXENSE_BUTTON_4)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveB4Pressed(i);
				}
				else{
					EventMoveB4Released(i);
				}
			}

			//Start
			if ((controller->buttons & SIXENSE_BUTTON_START) != (previous->buttons & SIXENSE_BUTTON_START))
			{
				if ((controller->buttons & SIXENSE_BUTTON_START) == SIXENSE_BUTTON_START)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveStartPressed(i);
				}
				else{
					EventMoveStartReleased(i);
				}
			}

			//Joystick Click
			if ((controller->buttons & SIXENSE_BUTTON_JOYSTICK) != (previous->buttons & SIXENSE_BUTTON_JOYSTICK))
			{
				if ((controller->buttons & SIXENSE_BUTTON_JOYSTICK) == SIXENSE_BUTTON_JOYSTICK)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveJoystickPressed(i);
				}
				else{
					EventMoveJoystickReleased(i);
				}
			}

			/** Movement */

			//Joystick
			if (controller->joystick.X != previous->joystick.X ||
				controller->joystick.Y != previous->joystick.Y)
			{
				EventMoveJoystickMoved(i, controller->joystick);
			}

			//Controller

			//Calculate Velocity and Acceleration
			controller->velocity = (controller->position - previous->position) / DeltaTime;
			controller->acceleration = (controller->velocity - previous->velocity) / DeltaTime;

			if (!controller->is_docked){

				//If the controller isn't docked, it's moving
				EventMoveControllerMoved(i,
					controller->position, controller->velocity, controller->acceleration,
					FRotator(controller->rotation));
			}
		}//end enabled
		else{
			if (controller->enabled != previous->enabled)
			{
				InternalMoveCheckEnabledCount(&plugNotChecked);
				EventMoveControllerDisabled(i);
			}
		}
	}//end controller for loop

	//Update the stored data with the integrated data obtained from latest
	MoveHistoryData[0] = *MoveLatestData;
}

/** Availability */
bool MoveDelegate::MoveIsAvailable()
{
	//Move will always have an enabled count of 2 when plugged in and working, stem functionality undefined.
	return MoveLatestData->enabledCount == 2;
}

/** Call to determine which hand you're holding the controller in. Determine by last docking position.*/
int32 MoveDelegate::MoveWhichHand(int32 controller)
{
	return MoveLatestData->controllers[controller].which_hand;
}