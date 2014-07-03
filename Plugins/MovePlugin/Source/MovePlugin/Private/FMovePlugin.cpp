#include "MovePluginPrivatePCH.h"

#include "IMovePlugin.h"

#include "FMovePlugin.h"
#include "MoveDelegate.h"

#include <iostream>
#include <stdexcept>
#include <vector>

// TODO: Include PSMoveAPI here!
//#include <sixense.h>

#include <windows.h>

#include "MovePlugin.generated.inl"

IMPLEMENT_MODULE(FMovePlugin, MovePlugin)


//Private API - This is where the magic happens

//DLL import definition

typedef bool (*psmove_init)(enum PSMove_Version);
typedef PSMove* (*psmove_connect)(void);
typedef PSMove* (*psmove_connect_by_id)(int);
typedef void (*psmove_disconnect)(PSMove*);
typedef int (*psmove_count_connected)(void);
typedef int (*psmove_pair)(PSMove*);
typedef PSMoveConnectionType (*psmove_connection_type)(PSMove*);
typedef int (*psmove_has_calibration)(PSMove*);
typedef void (*psmove_set_leds)(PSMove*, char, char, char);
typedef int (*psmove_update_leds)(PSMove*);
typedef void (*psmove_set_rumble)(PSMove*,char);
typedef uint (*psmove_poll)(PSMove*);
typedef uint (*psmove_get_buttons)(PSMove*);
typedef uint (*psmove_get_button_events)(PSMove*,uint*,uint*);
typedef char (*psmove_get_trigger)(PSMove*);
typedef float (*psmove_get_temperature)(PSMove*);
typedef PSMove_Battery_Level (*psmove_get_battery)(PSMove*);
typedef void (*psmove_get_accelerometer)(PSMove*,int*,int*,int*);
typedef void (*psmove_get_accelerometer_frame)(PSMove*,PSMove_Frame,float*,float*,float*);
typedef void (*psmove_get_gyroscope)(PSMove*,int*,int*,int*);
typedef void (*psmove_get_gyroscope_frame)(PSMove*,PSMove_Frame,float*,float*,float*);
typedef void (*psmove_get_magnetometer)(PSMove*,int*,int*,int*);
typedef string (*psmove_get_serial)(PSMove*);

psmove_init MoveInit;
psmove_connect MoveConnect;
psmove_connect_by_id MoveConnectByID;
psmove_disconnect MoveDisconnect;
psmove_count_connected MoveCountConnected;
psmove_pair MovePair;
psmove_connection_type MoveConnectionType;
psmove_has_calibration MoveHasCalibration;
psmove_set_leds MoveSetLEDs;
psmove_update_leds MoveUpdateLEDs;
psmove_set_rumble MoveSetRumble;
psmove_poll MovePoll;
psmove_get_buttons MoveGetButtons;
psmove_get_button_events MoveGetButtonEvents;
psmove_get_trigger MoveGetTrigger;
psmove_get_temperature MoveGetTemperature;
psmove_get_battery MoveGetBattery;
psmove_get_accelerometer MoveGetAccelerometer;
psmove_get_accelerometer_frame MoveGetAccelerometerFrame;
psmove_get_gyroscope MoveGetGyroscope;
psmove_get_gyroscope_frame MoveGetGyroscopeFrame;
psmove_get_magnetometer MoveGetMagnetometer;
psmove_get_serial MoveGetSerial;

typedef int (*dll_sixenseInit)(void);
typedef int (*dll_sixenseExit)(void);
typedef int (*dll_sixenseGetAllNewestData)(sixenseAllControllerData *);

dll_sixenseInit HydraInit;
dll_sixenseExit HydraExit;
dll_sixenseGetAllNewestData HydraGetAllNewestData;


class DataCollector
{
public:
	DataCollector()
	{
		moveDelegate = NULL;
		//allData = new sixenseAllControllerData;
		allDataUE = new moveAllControllerDataUE;
	}
	~DataCollector()
	{
		delete allData;
		delete allDataUE;
	}

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

	moveAllControllerDataUE* allDataUE;
	sixenseAllControllerData* allData;
	MoveDelegate* moveDelegate;
};

//Init and Runtime
void FMovePlugin::StartupModule()
{
	UE_LOG(LogClass, Log, TEXT("Attempting to startup Move Module."));
	try {
		collector = new DataCollector;

		FString DllFilename = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GameDir(),
			TEXT("Plugins"), TEXT("MovePlugin"), TEXT("Binaries/Win64")), TEXT("sixense_x64.dll")); // TODO: Fix this to point to libpsmoveapi

		DLLHandle = NULL;
		DLLHandle = FPlatformProcess::GetDllHandle(*DllFilename);
		
		if (!DLLHandle){
			UE_LOG(LogClass, Error, TEXT("DLL missing, Move Unavailable."));
			return;
		}

		// TODO: Get DLL functions.
		MoveInit = (psmove_init)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_init"));
		MoveConnect = (psmove_connect)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_connect"));
		MoveConnectByID = (psmove_connect_by_id)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_connect_by_id"));
		MoveDisconnect = (psmove_disconnect)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_disconnect"));
		MoveCountConnected = (psmove_count_connected)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_count_connected"));
		MovePair = (psmove_pair)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_pair"));
		MoveConnectionType = (psmove_connection_type)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_connection_type"));
		MoveHasCalibration = (psmove_has_calibration)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_has_calibration"));
		MoveSetLEDs = (psmove_set_leds)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_set_leds"));
		MoveSetRumble = (psmove_set_rumble)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_set_rumble"));
		MovePoll = (psmove_poll)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_poll"));
		MoveGetButtons = (psmove_get_buttons)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_buttons"));
		MoveGetButtonEvents = (psmove_get_button_events)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_button_events"));

		MoveGetTrigger = (psmove_get_trigger)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_trigger"));
		MoveGetTemperature = (psmove_get_temperature)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_temperature"));
		MoveGetBattery = (psmove_get_button_events)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_battery"));
		MoveGetAccelerometer = (psmove_get_accelerometer)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_accelerometer"));

		MoveGetAccelerometerFrame = (psmove_get_accelerometer_frame)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_accelerometer_frame"));
		MoveGetGyroscope = (psmove_get_gyroscope)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_gyroscope"));
		MoveGetGyroscopeFrame = (psmove_get_gyroscope_frame)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_gyroscope_frame"));
		MoveGetMagnetometer = (psmove_get_magnetometer)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_magnetometer"));
		MoveGetSerial = (psmove_get_serial)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_serial"));



		HydraInit = (dll_sixenseInit)FPlatformProcess::GetDllExport(DLLHandle, TEXT("sixenseInit"));
		HydraExit = (dll_sixenseExit)FPlatformProcess::GetDllExport(DLLHandle, TEXT("sixenseExit"));
		HydraGetAllNewestData = (dll_sixenseGetAllNewestData)FPlatformProcess::GetDllExport(DLLHandle, TEXT("sixenseGetAllNewestData"));


		// TODO: Set this up to store the PSMove* correctly after calling MoveConnect.
		collector->allDataUE->available = (MoveInit() == true);

		if (collector->allDataUE->available)
		{
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
	MoveDisconnect();

	FPlatformProcess::FreeDllHandle(DLLHandle);

	// TODO: Actually check this.
	//if (cleanshutdown == SIXENSE_SUCCESS)
	{
		UE_LOG(LogClass, Log, TEXT("Move Clean shutdown."));
	}

	delete collector;
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
	//get the freshest data
	int success = MoveGetAllNewestData(collector->allData);
	if (success == SIXENSE_FAILURE){
		UE_LOG(LogClass, Error, TEXT("Move Error! Failed to get freshest data."));
		return;
	}
	//if the hydras are unavailable don't try to get more information
	if (!collector->allDataUE->available){
		UE_LOG(LogClass, Log, TEXT("Collector data not available."));
		return;
	}

	//convert and pass the data to the delegate
	collector->ConvertAllData();

	//update our delegate pointer to point to the freshest data (may be redundant but has to be called once)
	if (collector->moveDelegate != NULL)
	{
		collector->moveDelegate->MoveLatestData = collector->allDataUE;	//ensure the delegate.latest is always pointing to our converted data
		collector->moveDelegate->InternalMoveControllerTick(DeltaTime);
	}
}