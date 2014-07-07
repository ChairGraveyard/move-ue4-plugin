#pragma once

#include "GameFramework/PlayerController.h"
#include "MoveDelegate.h"
#include "MovePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AMovePlayerController : public APlayerController, public MoveDelegate
{
	GENERATED_UCLASS_BODY()
	
	/**
	* We Override every delegate function to forward them to blueprint events,
	* but if you're extending C++ class yourself you can override just the ones
	* you are interested in.
	*/
	virtual void EventMovePluggedIn() OVERRIDE;
	virtual void EventMoveUnplugged() OVERRIDE;
	virtual void EventMoveControllerEnabled(int32 controller) OVERRIDE;
	virtual void EventMoveControllerDisabled(int32 controller) OVERRIDE;

	virtual void EventMoveDocked(int32 controller) OVERRIDE;
	virtual void EventMoveUndocked(int32 controller) OVERRIDE;

	virtual void EventMoveAnyButtonPressed(int32 controller) OVERRIDE;
	virtual void EventMoveB1Pressed(int32 controller) OVERRIDE;
	virtual void EventMoveB1Released(int32 controller) OVERRIDE;
	virtual void EventMoveB2Pressed(int32 controller) OVERRIDE;
	virtual void EventMoveB2Released(int32 controller) OVERRIDE;
	virtual void EventMoveB3Pressed(int32 controller) OVERRIDE;
	virtual void EventMoveB3Released(int32 controller) OVERRIDE;
	virtual void EventMoveB4Pressed(int32 controller) OVERRIDE;
	virtual void EventMoveB4Released(int32 controller) OVERRIDE;
	virtual void EventMoveTriggerPressed(int32 controller) OVERRIDE;
	virtual void EventMoveTriggerReleased(int32 controller) OVERRIDE;
	virtual void EventMoveTriggerChanged(int32 controller, float value) OVERRIDE;
	virtual void EventMoveStartPressed(int32 controller) OVERRIDE;
	virtual void EventMoveStartReleased(int32 controller) OVERRIDE;

	virtual void EventMoveControllerMoved(int32 controller,
		FVector position, FVector velocity, FVector acceleration,
		FRotator rotation) OVERRIDE;

	//Define blueprint events

	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MovePluggedIn();
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveUnplugged();
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveControllerEnabled(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveControllerDisabled(int32 controller);

	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveDocked(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveUndocked(int32 controller);

	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveAnyButtonPressed(int32 controller);

	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveB1Pressed(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveB1Released(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveB2Pressed(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveB2Released(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveB3Pressed(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveB3Released(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveB4Pressed(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveB4Released(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveTriggerPressed(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveTriggerReleased(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveTriggerChanged(int32 controller, float value);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveStartPressed(int32 controller);
	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveStartReleased(int32 controller);

	UFUNCTION(BlueprintImplementableEvent, Category = MoveEvents)
		void MoveControllerMoved(int32 controller,
		FVector position, FVector velocity, FVector acceleration,
		FRotator rotation);


	//Callable Blueprint functions

	/** Check if the Move is available/plugged in.*/
	UFUNCTION(BlueprintCallable, Category = MoveFunctions)
		bool MoveIsAvailable();
	/** Poll for latest data. Returns false if data is unavailable.*/
	UFUNCTION(BlueprintCallable, Category = MoveFunctions)
		bool MoveGetLatestData(int32 controller, FVector& position, FVector& velocity, FVector& acceleration, FRotator& rotation,
		FVector2D& joystick, int32& buttons, float& trigger, bool& docked);
	/** Poll for historical data. Valid index is 0-9. Returns false if data is unavailable.*/
	UFUNCTION(BlueprintCallable, Category = MoveFunctions)
		bool MoveGetHistoricalData(int32 controller, int32 historyIndex, FVector& position, FVector& velocity, FVector& acceleration, FRotator& rotation,
		FVector2D& joystick, int32& buttons, float& trigger, bool& docked);

	//Required for delegate to function
	virtual void BeginPlay() OVERRIDE;
	virtual void Tick(float DeltaTime) OVERRIDE;
};
