#include "MovePluginPrivatePCH.h"

#include "IMovePlugin.h"

#include "FMovePlugin.h"
#include "MoveDelegate.h"

#include <iostream>
#include <stdexcept>
#include <vector>

// TODO: Include PSMoveAPI here!
//#include <sixense.h>
#include <psmove_config.h>
#include <psmove_fusion.h>

#include <windows.h>


IMPLEMENT_MODULE(FMovePlugin, MovePlugin)


//Private API - This is where the magic happens
typedef PSMove* (*dll_psmove_connect)(void);
typedef PSMove* (*dll_psmove_connect_by_id)(int);
typedef int (*dll_psmove_poll)(PSMove*);
typedef unsigned int (*dll_psmove_get_buttons)(PSMove*);
typedef int (*dll_psmove_init)(int);
typedef int (*dll_psmove_count_connected)(void);
typedef void(*dll_psmove_set_leds)(PSMove*, unsigned char, unsigned char, unsigned char);

dll_psmove_connect MoveConnect;
dll_psmove_connect_by_id MoveConnectByID;
dll_psmove_poll MovePoll;
dll_psmove_get_buttons MoveGetButtons;
dll_psmove_init MoveInit;
dll_psmove_count_connected MoveCountConnected;
dll_psmove_set_leds MoveSetLEDs;
//DLL import definition
/*


typedef void (*dll_psmove_disconnect)(PSMove*);

typedef int (*dll_psmove_pair)(PSMove*);
//typedef PSMoveConnectionType (*dll_psmove_connection_type)(PSMove*);
typedef int (*dll_psmove_has_calibration)(PSMove*);

typedef int (*dll_psmove_update_leds)(PSMove*);
typedef void (*dll_psmove_set_rumble)(PSMove*,char);*/
//
/*
typedef uint (*dll_psmove_get_button_events)(PSMove*,uint*,uint*);
typedef char (*dll_psmove_get_trigger)(PSMove*);
typedef float (*dll_psmove_get_temperature)(PSMove*);
//typedef PSMove_Battery_Level (*dll_psmove_get_battery)(PSMove*);
typedef void (*dll_psmove_get_accelerometer)(PSMove*,int*,int*,int*);
//typedef void (*dll_psmove_get_accelerometer_frame)(PSMove*,PSMove_Frame,float*,float*,float*);
typedef void (*dll_psmove_get_gyroscope)(PSMove*,int*,int*,int*);
//typedef void (*dll_psmove_get_gyroscope_frame)(PSMove*,PSMove_Frame,float*,float*,float*);
typedef void (*dll_psmove_get_magnetometer)(PSMove*,int*,int*,int*);
//typedef string (*dll_psmove_get_serial)(PSMove*);
*/
/*


dll_psmove_disconnect MoveDisconnect;

dll_psmove_pair MovePair;
//dll_psmove_connection_type MoveConnectionType;
dll_psmove_has_calibration MoveHasCalibration;
dll_psmove_set_leds MoveSetLEDs;
dll_psmove_update_leds MoveUpdateLEDs;
dll_psmove_set_rumble MoveSetRumble;


dll_psmove_get_button_events MoveGetButtonEvents;
dll_psmove_get_trigger MoveGetTrigger;
dll_psmove_get_temperature MoveGetTemperature;
//dll_psmove_get_battery MoveGetBattery;
dll_psmove_get_accelerometer MoveGetAccelerometer;
//dll_psmove_get_accelerometer_frame MoveGetAccelerometerFrame;
dll_psmove_get_gyroscope MoveGetGyroscope;
//dll_psmove_get_gyroscope_frame MoveGetGyroscopeFrame;
dll_psmove_get_magnetometer MoveGetMagnetometer;
//dll_psmove_get_serial MoveGetSerial;
*/
class DataCollector
{
public:
	DataCollector()
	{
		moveDelegate = NULL;
		//allData = new sixenseAllControllerData;
		allDataUE = new moveAllControllerDataUE;
		for (int i = 0; i < 1; i++)
		{

		
			memset(&allDataUE->controllers[i], 0, sizeof(moveControllerDataUE));

			allDataUE->controllers[i].acceleration = FVector::ZeroVector;
			allDataUE->controllers[i].velocity = FVector::ZeroVector;
			allDataUE->controllers[i].position = FVector::ZeroVector;
			allDataUE->controllers[i].rotation = FQuat(0.0f, 0.0f, 0.0f, 1.0f);
			allDataUE->controllers[i].move = NULL;
			allDataUE->controllers[i].buttons = 0;
			allDataUE->controllers[i].enabled = false;
			allDataUE->enabledCount = 0;
			allDataUE->available = false;
	
		}
	}
	~DataCollector()
	{
		delete allDataUE;
	}
	/*
	moveControllerDataUE ConvertData(sixenseControllerData* data)
	{
		moveControllerDataUE converted;

		//Convert Sixense Axis to Unreal: UnrealX = - SixenseZ   UnrealY = SixenseX   UnrealZ = SixenseY
		// TODO: Figure out coordinate space for PS Move
		converted.position = FVector(-data->pos[2], data->pos[0], data->pos[1]);	//converted
		converted.rotation = FQuat(data->rot_quat[2], -data->rot_quat[0], -data->rot_quat[1], data->rot_quat[3]);	//converted & rotation values inverted
		converted.joystick = FVector2D(data->joystick_x, data->joystick_y);
		converted.trigger = data->trigger;
		converted.buttons = data->buttons;
		converted.sequence_number = data->sequence_number;
		converted.firmware_revision = data->firmware_revision;
		converted.hardware_revision = data->hardware_revision;
		converted.packet_type = data->packet_type;
		converted.magnetic_frequency = data->magnetic_frequency;
		converted.enabled = (data->enabled != 0);
		converted.controller_index = data->controller_index;
		converted.is_docked = (data->is_docked != 0);
		converted.which_hand = data->which_hand;
		converted.hemi_tracking_enabled = (data->hemi_tracking_enabled != 0);

		return converted;
	}

	void ConvertAllData()
	{
		allDataUE->enabledCount = 0;

		for (int i = 0; i < 4; i++)
		{
			allDataUE->controllers[i] = ConvertData(&allData->controllers[i]);
			if (allDataUE->controllers[i].enabled){
				allDataUE->enabledCount++;
			}
		}
	}
	*/
	moveAllControllerDataUE* allDataUE;
	//sixenseAllControllerData* allData;
	MoveDelegate* moveDelegate;
};

//Init and Runtime
void FMovePlugin::StartupModule()
{
	UE_LOG(LogClass, Log, TEXT("Attempting to startup Move Module."));
	try {
		collector = new DataCollector;

	
		FString DllFilename = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GameDir(),
			TEXT("Plugins"), TEXT("PSMove"), TEXT("Binaries/Win64")),TEXT("libpsmoveapi.dll")); // TODO: Fix this to point to libpsmoveapi

		DLLHandle = NULL;
		DLLHandle = FPlatformProcess::GetDllHandle(*DllFilename);
		
		if (!DLLHandle){
			UE_LOG(LogClass, Error, TEXT("DLL missing, Move Unavailable."));
			return;
		}

		MoveInit = (dll_psmove_init)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_init"));
		MoveConnect = (dll_psmove_connect)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_connect"));
		MoveConnectByID = (dll_psmove_connect_by_id)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_connect_by_id"));
		MovePoll = (dll_psmove_poll)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_poll"));
		MoveGetButtons = (dll_psmove_get_buttons)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_buttons"));
		MoveCountConnected = (dll_psmove_count_connected)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_count_connected"));
		MoveSetLEDs = (dll_psmove_set_leds)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_set_leds"));

		// TODO: Get DLL functions.
		/*
		
		

		MoveDisconnect = (dll_psmove_disconnect)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_disconnect"));
		
		MovePair = (dll_psmove_pair)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_pair"));
		//MoveConnectionType = (dll_psmove_connection_type)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_connection_type"));
		MoveHasCalibration = (dll_psmove_has_calibration)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_has_calibration"));
		
		MoveSetRumble = (dll_psmove_set_rumble)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_set_rumble"));
		*/

		/*
		
		MoveGetButtonEvents = (dll_psmove_get_button_events)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_button_events"));

		MoveGetTrigger = (dll_psmove_get_trigger)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_trigger"));
		MoveGetTemperature = (dll_psmove_get_temperature)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_temperature"));
		//MoveGetBattery = (dll_psmove_get_button_events)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_battery"));
		MoveGetAccelerometer = (dll_psmove_get_accelerometer)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_accelerometer"));

		//MoveGetAccelerometerFrame = (dll_psmove_get_accelerometer_frame)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_accelerometer_frame"));
		MoveGetGyroscope = (dll_psmove_get_gyroscope)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_gyroscope"));
		//MoveGetGyroscopeFrame = (dll_psmove_get_gyroscope_frame)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_gyroscope_frame"));
		MoveGetMagnetometer = (dll_psmove_get_magnetometer)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_magnetometer"));
		//MoveGetSerial = (dll_psmove_get_serial)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_serial"));
		*/
		// TODO: Set this up to store the PSMove* correctly after calling MoveConnect.
		int init_status = MoveInit(0x030001);

		collector->allDataUE->available = init_status == 1 ? true : false;

		if (collector->allDataUE->available)
		{
			isStarted = true;
			UE_LOG(LogClass, Log, TEXT("Move Available."));
		}
		else
		{
			UE_LOG(LogClass, Log, TEXT("Move Unavailable."));
		}
	}
	catch (const std::exception& e) {
		UE_LOG(LogClass, Log, TEXT("Error: %s"),e.what());
	}
}

void FMovePlugin::ShutdownModule()
{
	//MoveDisconnect();

	FPlatformProcess::FreeDllHandle(DLLHandle);

	// TODO: Actually check this.
	//if (cleanshutdown == SIXENSE_SUCCESS)
	{
		UE_LOG(LogClass, Log, TEXT("Move Clean shutdown."));
	}

	delete collector;
}

bool MoveGetAllNewestData(moveAllControllerDataUE* allDataUE)
{
	if (!MoveConnectByID || !MovePoll || !MoveGetButtons)
		return false; 

	if (!allDataUE->available)
		return false;

	for (int i = 0; i < 1; i++)
	{
		moveControllerDataUE *controller = &allDataUE->controllers[i];

		// Check for PSMove pointer.
		if (controller->move == NULL)
		{
			MoveInit(0x030001);
			// Attempt to connect.
			controller->move = MoveConnect();
		}

		allDataUE->enabledCount = MoveCountConnected();

		// Bail if still not valid.
		if (controller->move == NULL)
		{
			controller->enabled = false;
			continue;
		}

		MoveSetLEDs(controller->move, 255, 0, 0);

		MovePoll(controller->move);
		
		controller->enabled = true;

		//MoveGetAccelerometer(controller->move, &controller->acceleration.X, &controller->acceleration.Y, &controller->acceleration.Z);
		controller->buttons = MoveGetButtons(controller->move);
	}

	return true;
}

//Public API Implementation

/** Public API - Required **/

void FMovePlugin::SetDelegate(MoveDelegate* newDelegate)
{
	collector->moveDelegate = newDelegate;
	collector->moveDelegate->MoveLatestData = collector->allDataUE;	//set the delegate latest pointer
}

void FMovePlugin::MoveTick(float DeltaTime)
{
	if (!isStarted)
	{
		//isStarted = true;
		StartupModule();
	}

	//get the freshest data
	bool success = MoveGetAllNewestData(collector->allDataUE);
	if (success == false){
		UE_LOG(LogClass, Error, TEXT("Move Error! Failed to get freshest data."));
		return;
	}

	//if the hydras are unavailable don't try to get more information
	if (!collector->allDataUE->available){
		UE_LOG(LogClass, Log, TEXT("Collector data not available."));
		return;
	}

	//convert and pass the data to the delegate
	//collector->ConvertAllData();

	//update our delegate pointer to point to the freshest data (may be redundant but has to be called once)
	if (collector->moveDelegate != NULL)
	{
		collector->moveDelegate->MoveLatestData = collector->allDataUE;	//ensure the delegate.latest is always pointing to our converted data
		collector->moveDelegate->InternalMoveControllerTick(DeltaTime);
	}
}