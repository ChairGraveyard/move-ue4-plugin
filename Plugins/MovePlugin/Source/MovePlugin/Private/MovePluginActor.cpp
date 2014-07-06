#include "MovePluginPrivatePCH.h"

#include "Engine.h"
#include "GameFramework/Actor.h"
#include "IMovePlugin.h"
#include "MoveDelegate.h"
#include "MovePluginActor.h"

//Constructor/Initializer
AMovePluginActor::AMovePluginActor(const FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
}

//Override implementation
void AMovePluginActor::EventMoveControllerEnabled(int32 controller)
{
	MoveControllerEnabled(controller);
}
void AMovePluginActor::EventMoveControllerDisabled(int32 controller)
{
	MoveControllerDisabled(controller);
}
void AMovePluginActor::EventMovePluggedIn()
{
	MovePluggedIn();
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Hydra Plugged in.")));
}
void AMovePluginActor::EventMoveUnplugged()
{
	MoveUnplugged();
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Hydra Unplugged.")));
}
void AMovePluginActor::EventMoveDocked(int32 controller)
{
	MoveDocked(controller);
}
void AMovePluginActor::EventMoveUndocked(int32 controller)
{
	MoveUndocked(controller);
}
void AMovePluginActor::EventMoveAnyButtonPressed(int32 controller)
{
	MoveAnyButtonPressed(controller);
}
void AMovePluginActor::EventMoveB1Pressed(int32 controller)
{
	MoveB1Pressed(controller);
}
void AMovePluginActor::EventMoveB1Released(int32 controller)
{
	MoveB1Released(controller);
}
void AMovePluginActor::EventMoveB2Pressed(int32 controller)
{
	MoveB2Pressed(controller);
}
void AMovePluginActor::EventMoveB2Released(int32 controller)
{
	MoveB2Released(controller);
}
void AMovePluginActor::EventMoveB3Pressed(int32 controller)
{
	MoveB3Pressed(controller);
}
void AMovePluginActor::EventMoveB3Released(int32 controller)
{
	MoveB3Released(controller);
}
void AMovePluginActor::EventMoveB4Pressed(int32 controller)
{
	MoveB4Pressed(controller);
}
void AMovePluginActor::EventMoveB4Released(int32 controller)
{
	MoveB4Released(controller);
}
void AMovePluginActor::EventMoveTriggerPressed(int32 controller)
{
	MoveTriggerPressed(controller);
}
void AMovePluginActor::EventMoveTriggerReleased(int32 controller)
{
	MoveTriggerReleased(controller);
}
void AMovePluginActor::EventMoveTriggerChanged(int32 controller, float value)
{
	MoveTriggerChanged(controller, value);
}
void AMovePluginActor::EventMoveStartPressed(int32 controller)
{
	MoveStartPressed(controller);
}
void AMovePluginActor::EventMoveStartReleased(int32 controller)
{
	MoveStartReleased(controller);
}

void AMovePluginActor::EventMoveControllerMoved(int32 controller,
	FVector position, FVector velocity, FVector acceleration,
	FRotator rotation)
{
	MoveControllerMoved(controller, position, velocity, acceleration, rotation);
}

bool AMovePluginActor::MoveIsAvailable()
{
	return MoveDelegate::MoveIsAvailable();
}

/** Poll for latest data.*/
bool AMovePluginActor::MoveGetLatestData(int32 controller, FVector& position, FVector& velocity, FVector& acceleration, FRotator& rotation,
	FVector2D& joystick, int32& buttons, float& trigger, bool& docked)
{
	if (controller > 4 || controller < 0) { return false; }

	return MoveGetHistoricalData(controller, 0, position, velocity, acceleration, rotation, joystick, buttons, trigger, docked);
}

/** Poll for historical data. Valid index is 0-9 */
bool AMovePluginActor::MoveGetHistoricalData(int32 controller, int32 historyIndex, FVector& position, FVector& velocity, FVector& acceleration, FRotator& rotation,
	FVector2D& joystick, int32& buttons, float& trigger, bool& docked)
{
	if (historyIndex<0 || historyIndex>9){ return false; }
	if (controller > 4 || controller < 0){ return false; }

	sixenseControllerDataUE* data = &HydraHistoryData[historyIndex].controllers[controller];

	position = data->position;
	velocity = data->velocity;
	acceleration = data->acceleration;
	rotation = FRotator(data->rotation);
	joystick = data->joystick;
	buttons = data->buttons;
	trigger = data->trigger;
	docked = data->is_docked;
	return data->enabled;
}

//Required Overrides
void AMovePluginActor::BeginPlay()
{
	Super::BeginPlay();

	if (IMovePlugin::IsAvailable())
	{
		//Debug Display - You can safely remove these two lines, retained for lazy copy-paste
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Plugin Module Loaded."));

		//Required to Work - Set self as a delegate
		IMovePlugin::Get().SetDelegate((MoveDelegate*)this);
	}
}

void AMovePluginActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IMovePlugin::IsAvailable())
	{
		//Required to Work - This is the plugin magic
		IMovePlugin::Get().MoveTick(DeltaTime);
	}
}

