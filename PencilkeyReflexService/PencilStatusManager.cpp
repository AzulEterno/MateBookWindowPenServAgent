#include "PencilStatusManager.h"

namespace PencilKeyReflex{
    int PencilStatusManagerBase::SwitchPencilModeCall()
    {
		
		_penMode = !_penMode;

        DoSwitchPencilModeTask();

        return 0;
    }

    bool isDebugMode = false;
    bool isRunning = true;
    HHOOK hHook = NULL;
    HuaweiPencilStatusManager * pencilStatusManagerPtr = nullptr;
}