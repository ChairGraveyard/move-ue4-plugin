#pragma once

#include "MovePluginPrivatePCH.h"
#include "MoveDelegate.h"


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
void MoveDelegate::EventMoveStartPressed(int32 controller){}
void MoveDelegate::EventMoveStartReleased(int32 controller){}

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

	moveAllControllerDataUE* previous = &MoveHistoryData[0];
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

	moveControllerDataUE* controller;
	moveControllerDataUE* previous;
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

			//B1
			if ((controller->buttons & Btn_TRIANGLE) != (previous->buttons & Btn_TRIANGLE))
			{
				if ((controller->buttons & Btn_TRIANGLE) == Btn_TRIANGLE)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveB1Pressed(i);
				}
				else{
					EventMoveB1Released(i);
				}
			}
			//B2
			if ((controller->buttons & Btn_CIRCLE) != (previous->buttons & Btn_CIRCLE))
			{
				if ((controller->buttons & Btn_CIRCLE) == Btn_CIRCLE)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveB2Pressed(i);
				}
				else{
					EventMoveB2Released(i);
				}
			}
			//B3
			if ((controller->buttons & Btn_CROSS) != (previous->buttons & Btn_CROSS))
			{
				if ((controller->buttons & Btn_CROSS) == Btn_CROSS)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveB3Pressed(i);
				}
				else{
					EventMoveB3Released(i);
				}
			}
			//B4
			if ((controller->buttons & Btn_SQUARE) != (previous->buttons & Btn_SQUARE))
			{
				if ((controller->buttons & Btn_SQUARE) == Btn_SQUARE)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveB4Pressed(i);
				}
				else{
					EventMoveB4Released(i);
				}
			}

			//Start
			if ((controller->buttons & Btn_START) != (previous->buttons & Btn_START))
			{
				if ((controller->buttons & Btn_START) == Btn_START)
				{
					EventMoveAnyButtonPressed(i);
					EventMoveStartPressed(i);
				}
				else{
					EventMoveStartReleased(i);
				}
			}

			/** Movement */
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
	return MoveLatestData->enabledCount >= 1;
}
