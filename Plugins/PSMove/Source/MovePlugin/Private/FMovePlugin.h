#pragma once

class DataCollector;
class MoveDelegate;

class FMovePlugin : public IMovePlugin
{
public:
	/** IModuleInterface implementation */
	void StartupModule();
	void ShutdownModule();

	/** To subscribe to event calls, only supports one listener for now */
	void SetDelegate(MoveDelegate* newDelegate);

	/** Call this in your class Tick to update information */
	void MoveTick(float DeltaTime);

private:
	DataCollector *collector;
	void* DLLHandle;

	void SetupPSMoveFunctions();
};