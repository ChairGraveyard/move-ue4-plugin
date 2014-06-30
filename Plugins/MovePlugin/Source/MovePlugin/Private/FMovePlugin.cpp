#include "HydraPluginPrivatePCH.h"

#include "IHydraPlugin.h"

#include "FHydraPlugin.h"
#include "HydraDelegate.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <sixense.h>
#include <windows.h>

#include "HydraPlugin.generated.inl"

IMPLEMENT_MODULE(FHydraPlugin, HydraPlugin)


//Private API - This is where the magic happens

//DLL import definition

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
		hydraDelegate = NULL;
		allData = new sixenseAllControllerData;
		allDataUE = new sixenseAllControllerDataUE;
	}
	~DataCollector()
	{
		delete allData;
		delete allDataUE;
	}

	sixenseControllerDataUE ConvertData(sixenseControllerData* data)
	{
		sixenseControllerDataUE converted;

		//Convert Sixense Axis to Unreal: UnrealX = - SixenseZ   UnrealY = SixenseX   UnrealZ = SixenseY
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

	sixenseAllControllerDataUE* allDataUE;
	sixenseAllControllerData* allData;
	HydraDelegate* hydraDelegate;
};

//Init and Runtime
void FHydraPlugin::StartupModule()
{
	UE_LOG(LogClass, Log, TEXT("Attempting to startup Hydra Module."));
	try {
		collector = new DataCollector;

		FString DllFilename = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GameDir(),
			TEXT("Plugins"), TEXT("HydraPlugin"), TEXT("Binaries/Win64")), TEXT("sixense_x64.dll"));

		DLLHandle = NULL;
		DLLHandle = FPlatformProcess::GetDllHandle(*DllFilename);
		
		if (!DLLHandle){
			UE_LOG(LogClass, Error, TEXT("DLL missing, Hydra Unavailable."));
			return;
		}

		HydraInit = (dll_sixenseInit)FPlatformProcess::GetDllExport(DLLHandle, TEXT("sixenseInit"));
		HydraExit = (dll_sixenseExit)FPlatformProcess::GetDllExport(DLLHandle, TEXT("sixenseExit"));
		HydraGetAllNewestData = (dll_sixenseGetAllNewestData)FPlatformProcess::GetDllExport(DLLHandle, TEXT("sixenseGetAllNewestData"));


		collector->allDataUE->available = (HydraInit() == SIXENSE_SUCCESS);

		if (collector->allDataUE->available)
		{
			UE_LOG(LogClass, Log, TEXT("Hydra Available."));
		}
		else
		{
			UE_LOG(LogClass, Log, TEXT("Hydra Unavailable."));
		}
	}
	catch (const std::exception& e) {
		UE_LOG(LogClass, Log, TEXT("Error: %s"),e.what());
	}
}

void FHydraPlugin::ShutdownModule()
{
	int cleanshutdown = HydraExit();

	FPlatformProcess::FreeDllHandle(DLLHandle);

	if (cleanshutdown == SIXENSE_SUCCESS)
	{
		UE_LOG(LogClass, Log, TEXT("Hydra Clean shutdown."));
	}

	delete collector;
}


//Public API Implementation

/** Public API - Required **/

void FHydraPlugin::SetDelegate(HydraDelegate* newDelegate)
{
	collector->hydraDelegate = newDelegate;
	collector->hydraDelegate->HydraLatestData = collector->allDataUE;	//set the delegate latest pointer
}

void FHydraPlugin::HydraTick(float DeltaTime)
{
	//get the freshest data
	int success = HydraGetAllNewestData(collector->allData);
	if (success == SIXENSE_FAILURE){
		UE_LOG(LogClass, Error, TEXT("Hydra Error! Failed to get freshest data."));
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
	if (collector->hydraDelegate != NULL)
	{
		collector->hydraDelegate->HydraLatestData = collector->allDataUE;	//ensure the delegate.latest is always pointing to our converted data
		collector->hydraDelegate->InternalHydraControllerTick(DeltaTime);
	}
}