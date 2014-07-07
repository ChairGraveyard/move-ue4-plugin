#include "MovePluginPrivatePCH.h"

#include "IMovePlugin.h"
#include "MoveDelegate.h"
#include "MovePlayerController.h"

//Constructor/Initializer
AMovePlayerController::AMovePlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
}

//Override implementation
void AMovePlayerController::EventMoveControllerEnabled(int32 controller)
{
	MoveControllerEnabled(controller);
}
void AMovePlayerController::EventMoveControllerDisabled(int32 controller)
{
	MoveControllerDisabled(controller);
}
void AMovePlayerController::EventMovePluggedIn()
{
	MovePluggedIn();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Move Plugged in.")));
}
void AMovePlayerController::EventMoveUnplugged()
{
	MoveUnplugged();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Move Unplugged.")));
}
void AMovePlayerController::EventMoveDocked(int32 controller)
{
	MoveDocked(controller);
}
void AMovePlayerController::EventMoveUndocked(int32 controller)
{
	MoveUndocked(controller);
}
void AMovePlayerController::EventMoveAnyButtonPressed(int32 controller)
{
	MoveAnyButtonPressed(controller);
}
void AMovePlayerController::EventMoveB1Pressed(int32 controller)
{
	MoveB1Pressed(controller);
}
void AMovePlayerController::EventMoveB1Released(int32 controller)
{
	MoveB1Released(controller);
}
void AMovePlayerController::EventMoveB2Pressed(int32 controller)
{
	MoveB2Pressed(controller);
}
void AMovePlayerController::EventMoveB2Released(int32 controller)
{
	MoveB2Released(controller);
}
void AMovePlayerController::EventMoveB3Pressed(int32 controller)
{
	MoveB3Pressed(controller);
}
void AMovePlayerController::EventMoveB3Released(int32 controller)
{
	MoveB3Released(controller);
}
void AMovePlayerController::EventMoveB4Pressed(int32 controller)
{
	MoveB4Pressed(controller);
}
void AMovePlayerController::EventMoveB4Released(int32 controller)
{
	MoveB4Released(controller);
}
void AMovePlayerController::EventMoveTriggerPressed(int32 controller)
{
	MoveTriggerPressed(controller);
}
void AMovePlayerController::EventMoveTriggerReleased(int32 controller)
{
	MoveTriggerReleased(controller);
}
void AMovePlayerController::EventMoveTriggerChanged(int32 controller, float value)
{
	MoveTriggerChanged(controller, value);
}
void AMovePlayerController::EventMoveStartPressed(int32 controller)
{
	MoveStartPressed(controller);
}
void AMovePlayerController::EventMoveStartReleased(int32 controller)
{
	MoveStartReleased(controller);
}

void AMovePlayerController::EventMoveControllerMoved(int32 controller,
	FVector position, FVector velocity, FVector acceleration,
	FRotator rotation)
{
	MoveControllerMoved(controller, position, velocity, acceleration, rotation);
}

bool AMovePlayerController::MoveIsAvailable()
{
	return MoveDelegate::MoveIsAvailable();
}

/** Poll for latest data.*/
bool AMovePlayerController::MoveGetLatestData(int32 controller, FVector& position, FVector& velocity, FVector& acceleration, FRotator& rotation,
	FVector2D& joystick, int32& buttons, float& trigger, bool& docked)
{
	if (controller > 4 || controller < 0){ return false; }

	return MoveGetHistoricalData(controller, 0, position, velocity, acceleration, rotation, joystick, buttons, trigger, docked);
}

/** Poll for historical data. Valid index is 0-9 */
bool AMovePlayerController::MoveGetHistoricalData(int32 controller, int32 historyIndex, FVector& position, FVector& velocity, FVector& acceleration, FRotator& rotation,
	FVector2D& joystick, int32& buttons, float& trigger, bool& docked)
{
	if (historyIndex<0 || historyIndex>9){ return false; }
	if (controller > 4 || controller < 0){ return false; }

	moveControllerDataUE* data = &MoveHistoryData[historyIndex].controllers[controller];

	position = data->position;
	velocity = data->velocity;
	acceleration = data->acceleration;
	rotation = FRotator(data->rotation);
	
	buttons = data->buttons;
	docked = data->is_docked;
	return data->enabled;
}

//Required Overrides
void AMovePlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (IMovePlugin::IsAvailable())
	{
		//Required to Work - Set self as a delegate
		IMovePlugin::Get().SetDelegate((MoveDelegate*)this);
	}
}

void AMovePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IMovePlugin::IsAvailable())
	{
		//Required to Work - This is the plugin magic
		IMovePlugin::Get().MoveTick(DeltaTime);
	}
}

