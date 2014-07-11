#include "MovePluginPrivatePCH.h"

#include "IMovePlugin.h"

#include "FMovePlugin.h"
#include "MoveDelegate.h"

#include <iostream>
#include <stdexcept>
#include <vector>

#include <psmove.h>
#include <psmove_tracker.h>
#include <psmove_fusion.h>

#include <windows.h>


IMPLEMENT_MODULE(FMovePlugin, MovePlugin)


//Private API - This is where the magic happens


//DLL import definition

// PS Move API
typedef int (*dll_psmove_pair)(PSMove*);

typedef int (*dll_psmove_init)(int);
typedef PSMove* (*dll_psmove_connect)(void);
typedef PSMove* (*dll_psmove_connect_by_id)(int);
typedef void (*dll_psmove_disconnect)(PSMove*);
typedef int (*dll_psmove_count_connected)(void);
typedef enum PSMoveConnectionType (*dll_psmove_connection_type)(PSMove*);

typedef int (*dll_psmove_poll)(PSMove*);

typedef char (*dll_psmove_get_trigger)(PSMove*);
typedef unsigned int (*dll_psmove_get_buttons)(PSMove*);
typedef unsigned int (*dll_psmove_get_button_events)(PSMove*, unsigned int*, unsigned int*);

typedef void (*dll_psmove_set_leds)(PSMove*, unsigned char, unsigned char, unsigned char);
typedef int (*dll_psmove_update_leds)(PSMove*);

typedef void (*dll_psmove_set_rumble)(PSMove*, char);

typedef int (*dll_psmove_has_calibration)(PSMove*);

typedef void (*dll_psmove_get_accelerometer)(PSMove*, int*, int*, int*);
typedef void (*dll_psmove_get_accelerometer_frame)(PSMove*, PSMove_Frame, float*, float*, float*);
typedef void (*dll_psmove_get_gyroscope)(PSMove*, int*, int*, int*);
typedef void (*dll_psmove_get_gyroscope_frame)(PSMove*, PSMove_Frame, float*, float*, float*);
typedef void (*dll_psmove_get_magnetometer)(PSMove*, int*, int*, int*);
typedef void (*dll_psmove_get_orientation)(PSMove*, float* w, float* x, float* y, float* z);
typedef void (*dll_psmove_enable_orientation)(PSMove*);

typedef float (*dll_psmove_get_temperature)(PSMove*);
typedef PSMove_Battery_Level (*dll_psmove_get_battery)(PSMove*);
typedef char*  (*dll_psmove_get_serial)(PSMove*);
typedef void (*dll_psmove_set_rate_limiting)(PSMove*, int);

dll_psmove_pair MovePair;

dll_psmove_init MoveInit;
dll_psmove_connect MoveConnect;
dll_psmove_connect_by_id MoveConnectByID;
dll_psmove_disconnect MoveDisconnect;
dll_psmove_count_connected MoveCountConnected;
dll_psmove_connection_type MoveConnectionType;

dll_psmove_poll MovePoll;

dll_psmove_get_trigger MoveGetTrigger;
dll_psmove_get_buttons MoveGetButtons;
dll_psmove_get_button_events MoveGetButtonEvents;

dll_psmove_has_calibration MoveHasCalibration;

dll_psmove_set_leds MoveSetLEDs;
dll_psmove_update_leds MoveUpdateLEDs;
dll_psmove_set_rumble MoveSetRumble;

dll_psmove_get_accelerometer MoveGetAccelerometer;
dll_psmove_get_accelerometer_frame MoveGetAccelerometerFrame;
dll_psmove_get_gyroscope MoveGetGyroscope;
dll_psmove_get_gyroscope_frame MoveGetGyroscopeFrame;
dll_psmove_get_magnetometer MoveGetMagnetometer;
dll_psmove_get_orientation MoveGetOrientation;
dll_psmove_enable_orientation MoveEnableOrientation;

dll_psmove_get_temperature MoveGetTemperature;
dll_psmove_get_battery MoveGetBattery;
dll_psmove_get_serial MoveGetSerial;
dll_psmove_set_rate_limiting MoveSetRateLimiting;

// PS Move Tracker API

typedef PSMoveTracker*(*dll_psmove_tracker_new)(void);
typedef void(*dll_psmove_tracker_free)(PSMoveTracker*);
typedef enum PSMoveTracker_Status(*dll_psmove_tracker_enable)(PSMoveTracker*, PSMove*);
typedef enum PSMoveTracker_Status(*dll_psmove_tracker_enable_with_color)(PSMoveTracker*, PSMove*, unsigned char r, unsigned char g, unsigned char b);
typedef void(*dll_psmove_tracker_disable)(PSMoveTracker *, PSMove*);
typedef enum PSMoveTracker_Status(*dll_psmove_tracker_get_status)(PSMoveTracker*, PSMove*);
typedef void(*dll_psmove_update_image)(PSMoveTracker*);
typedef int(*dll_psmove_tracker_update)(PSMoveTracker*, PSMove*);

typedef int(*dll_psmove_tracker_get_position)(PSMoveTracker*, PSMove*, float* x, float* y, float* radius);
typedef void(*dll_psmove_tracker_get_size)(PSMoveTracker*, PSMove*, int* w, int* h);
typedef float(*dll_psmove_get_distance_from_radius)(PSMoveTracker*, float radius);

dll_psmove_tracker_new MoveTrackerNew;
dll_psmove_tracker_free MoveTrackerFree;
dll_psmove_tracker_enable MoveTrackerEnable;
dll_psmove_tracker_enable_with_color MoveTrackerEnableWithColor;
dll_psmove_tracker_disable MoveTrackerDisable;
dll_psmove_tracker_get_status MoveTrackerGetStatus;
dll_psmove_update_image MoveTrackerUpdateImage;
dll_psmove_tracker_update MoveTrackerUpdate;

dll_psmove_tracker_get_position MoveTrackerGetPosition;
dll_psmove_tracker_get_size MoveTrackerGetSize;
dll_psmove_get_distance_from_radius MoveTrackerGetDistanceFromRadius;


class DataCollector
{
public:
	DataCollector()
	{
		moveDelegate = NULL;		

		allDataUE = new moveAllControllerDataUE;
		allDataUE->tracker = NULL;
		allDataUE->calibration_status = 0;

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

	moveAllControllerDataUE* allDataUE;
	MoveDelegate* moveDelegate;
};

//Init and Runtime
void FMovePlugin::StartupModule()
{
	UE_LOG(LogClass, Log, TEXT("Attempting to startup Move Module."));
	try {
		collector = new DataCollector;

		FString DllFilename = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GameDir(),
			TEXT("Plugins"), TEXT("PSMove"), TEXT("Binaries/Win64")),TEXT("libpsmoveapi.dll")); 

		DLLHandle = NULL;
		DLLHandle = FPlatformProcess::GetDllHandle(*DllFilename);
		
		// Make sure we have a valid handle to the PS Move API DLL.
		if (!DLLHandle){
			UE_LOG(LogClass, Error, TEXT("DLL missing, Move Unavailable."));
			collector->allDataUE->available = false;
			return;
		}

		// PS Move Core
		MoveInit = (dll_psmove_init)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_init"));

		// If this failed, something went wrong even though the DLL handle is valid (wrong DLL?)
		if (!MoveInit)
		{
			UE_LOG(LogClass, Error, TEXT("DLL missing, Move Unavailable."));
			collector->allDataUE->available = false;
			return;
		}

		// Check version. 0x030001 = current version.
		int init_status = MoveInit(0x030001);

		// Is the API available? Will be checked later to avoid processing/calling invalid null pointered functions.
		collector->allDataUE->available = init_status == 1 ? true : false;		 

		MovePair = (dll_psmove_pair)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_pair"));   		                		
		MoveConnect = (dll_psmove_connect)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_connect"));
		MoveConnectByID = (dll_psmove_connect_by_id)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_connect_by_id"));
		MoveDisconnect = (dll_psmove_disconnect)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_disconnect"));
		MoveCountConnected = (dll_psmove_count_connected)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_count_connected"));
		MoveConnectionType = (dll_psmove_connection_type)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_connection_type"));

		MovePoll = (dll_psmove_poll)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_poll"));		

		MoveGetTrigger = (dll_psmove_get_trigger)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_trigger"));
		MoveGetButtons = (dll_psmove_get_buttons)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_buttons"));		
		MoveGetButtonEvents = (dll_psmove_get_button_events)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_button_events"));	

		MoveHasCalibration = (dll_psmove_has_calibration)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_has_calibration"));
		
		MoveSetLEDs = (dll_psmove_set_leds)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_set_leds"));
		MoveUpdateLEDs = (dll_psmove_update_leds)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_update_leds"));
		MoveSetRumble = (dll_psmove_set_rumble)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_set_rumble"));

		MoveGetAccelerometer = (dll_psmove_get_accelerometer)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_accelerometer"));
		MoveGetAccelerometerFrame = (dll_psmove_get_accelerometer_frame)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_accelerometer_frame"));
		MoveGetGyroscope = (dll_psmove_get_gyroscope)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_gyroscope"));
		MoveGetGyroscopeFrame = (dll_psmove_get_gyroscope_frame)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_gyroscope_frame"));
		MoveGetMagnetometer = (dll_psmove_get_magnetometer)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_magnetometer"));
		MoveGetOrientation = (dll_psmove_get_orientation)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_orientation"));
		MoveEnableOrientation = (dll_psmove_enable_orientation)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_enable_orientation"));

		MoveGetTemperature = (dll_psmove_get_temperature)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_temperature"));
		MoveGetBattery = (dll_psmove_get_battery)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_battery"));
		MoveGetSerial = (dll_psmove_get_serial)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_serial"));
		MoveSetRateLimiting = (dll_psmove_set_rate_limiting)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_set_rate_limiting"));
		// Get tracker DLL path.
		DllFilename = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GameDir(),
			TEXT("Plugins"), TEXT("PSMove"), TEXT("Binaries/Win64")), TEXT("libpsmoveapi_tracker.dll")); 

		// Get tracker DLL handle.
		DLLHandle = NULL;
		DLLHandle = FPlatformProcess::GetDllHandle(*DllFilename);

		// Make sure our DLL handle for tracking is valid.
		if (!DLLHandle){
			UE_LOG(LogClass, Error, TEXT("DLL missing, Move Unavailable."));
			collector->allDataUE->available = false;
			return;
		}

		// PS Move Tracker
		MoveTrackerNew = (dll_psmove_tracker_new)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_tracker_new"));

		// If this failed, something went wrong even though the DLL handle is valid (wrong DLL?)
		if (!MoveTrackerNew)
		{
			UE_LOG(LogClass, Error, TEXT("Tracking DLL missing, Move Unavailable."));
			collector->allDataUE->available = false;
			return;
		}

		MoveTrackerFree = (dll_psmove_tracker_free)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_tracker_free"));
		MoveTrackerEnable = (dll_psmove_tracker_enable)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_tracker_enable"));
		MoveTrackerEnableWithColor = (dll_psmove_tracker_enable_with_color)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_tracker_enable_with_color"));
		MoveTrackerDisable = (dll_psmove_tracker_disable)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_tracker_disable"));
		MoveTrackerGetStatus = (dll_psmove_tracker_get_status)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_tracker_get_status"));
		MoveTrackerUpdateImage = (dll_psmove_update_image)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_update_image"));
		MoveTrackerUpdate = (dll_psmove_tracker_update)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_tracker_update"));

		MoveTrackerGetPosition = (dll_psmove_tracker_get_position)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_tracker_get_position"));
		MoveTrackerGetSize = (dll_psmove_tracker_get_size)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_tracker_get_size"));
		MoveTrackerGetDistanceFromRadius = (dll_psmove_get_distance_from_radius)FPlatformProcess::GetDllExport(DLLHandle, TEXT("psmove_get_distance_from_radius"));

		// Set up tracker
		collector->allDataUE->tracker = MoveTrackerNew();

		

		// TODO: Set up/connect controllers and trackers here,
		// as doing it inside tick is bad.
		for (int i = 0; i < 1; i++)
		{
			// Pull out controller.
			moveControllerDataUE *controller = &collector->allDataUE->controllers[i];

			// Check for PSMove pointer.
			if (controller->move == NULL)
			{
				// Attempt to connect.
				controller->move = MoveConnect();
			}

			// Update connected count.
			collector->allDataUE->enabledCount = MoveCountConnected();

			// Bail if still not valid.
			if (controller->move == NULL)
			{
				controller->enabled = false;
			}		

			if (collector->allDataUE->tracker != NULL)
			{
				int result = collector->allDataUE->calibration_status = MoveTrackerEnable(collector->allDataUE->tracker, controller->move);

				if (result == Tracker_CALIBRATED)
				{
					UE_LOG(LogClass, Log, TEXT("Move Calibrated."));
				}
				else
				{
					UE_LOG(LogClass, Log, TEXT("Move Calibration Failed"));
				}
			}

			// Enable orientation support.
			MoveEnableOrientation(controller->move);
		}


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
	for (int i = 0; i < 1; i++)
	{ 
		moveControllerDataUE* controller = &collector->allDataUE->controllers[i];
		if (!controller->move)
			return;

		MoveDisconnect(controller->move);
	}

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
	// If the plugin is not available, bail.
	if (!allDataUE->available)
		return false;

	// Update tracker image.
	if (allDataUE->tracker != NULL)
		MoveTrackerUpdateImage(allDataUE->tracker);		

	for (int i = 0; i < 1; i++) 
	{
		moveControllerDataUE *controller = &allDataUE->controllers[i];

		// Bail if not valid.
		if (controller->move == NULL)
			continue;

		// Update tracker for this controller.
		if (allDataUE->tracker != NULL)
		{
			if (allDataUE->calibration_status != Tracker_CALIBRATED && allDataUE->calibration_status != Tracker_TRACKING)
			{
				int result = allDataUE->calibration_status = MoveTrackerEnable(allDataUE->tracker, controller->move);

				if (result == Tracker_CALIBRATED )
				{
					UE_LOG(LogClass, Log, TEXT("Move Calibrated."));
				}
				else
				{
					UE_LOG(LogClass, Log, TEXT("Move Calibration Failed"));
				}

				// Enable orientation support.
				MoveEnableOrientation(controller->move);
			}

			MoveTrackerUpdate(allDataUE->tracker, controller->move);
		}
		// Get new data.
		MovePoll(controller->move);
		 
		// Get acceleration data.
		int ax = 0, ay = 0, az = 0, aw = 0;
		MoveGetAccelerometer(controller->move, &ax, &ay, &az);
		controller->acceleration.Set((float)ax, (float)ay, (float)az);			

		// Get orientation.
		float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
		MoveGetOrientation(controller->move, &w, &x, &y, &z);
		controller->rotation.X = x, controller->rotation.Y = y, controller->rotation.Z = z, controller->rotation.W = w;

		// Get position.
		if (allDataUE->tracker != NULL)
		{
			float r = 0.0f;
			MoveTrackerGetPosition(allDataUE->tracker, controller->move, &x, &y, &r);
			controller->position.X = x, controller->position.Y = y, controller->position.Z = r;
		}

		// Get buttons and trigger.
		controller->buttons = MoveGetButtons(controller->move);
		controller->trigger = MoveGetTrigger(controller->move);
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