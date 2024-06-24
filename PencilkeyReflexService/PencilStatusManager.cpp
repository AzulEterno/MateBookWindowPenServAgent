#include "PencilStatusManager.h"

namespace PencilKeyReflex{
    int PencilStatusManagerBase::SwitchPencilModeCall()
    {
		
		_penMode = !_penMode;

        DoSwitchPencilModeTask();

        return 0;
    }

    bool isDebugMode = false;
    HHOOK hHook = NULL;
    HuaweiPencilStatusManager pencilStatusManager;
}