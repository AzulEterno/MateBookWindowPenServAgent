#include "PencilStatusManager.h"

namespace PencilKeyReflex{
    int PencilStatusManagerBase::SwitchPencilModeCall()
    {
		
		_penMode = !_penMode;

        DoSwitchPencilModeTask();

        return 0;
    }

    DebugOutputType _debugModeType = DebugOutputType::DEBUG_TEMP_LOG_FILE ;
    std::string logSavePath = "";
    bool isRunning = true;
    HHOOK hHook = NULL;
    HuaweiPencilStatusManager * pencilStatusManagerPtr = nullptr;
}