#pragma once
#include "pch.h"
#include "PencilWindows.h"
#include "DLL_Loader.h"


namespace PencilKeyReflex{

    enum class DebugOutputType:int {
        DEBUG_IGNORE_OUTPUT,
        DEBUG_WINDOW,
        DEBUG_TEMP_LOG_FILE

    };

	class PencilStatusManagerBase
	{
	protected:
		bool _penMode = true;
	public:
		PencilStatusManagerBase(bool initPenMode = true):_penMode(initPenMode) {
		}
		bool IsPenMode() const { return _penMode; }

		int SwitchPencilModeCall();

		virtual int DoSwitchPencilModeTask() = 0;
	};


	class HuaweiPencilStatusManager :public PencilStatusManagerBase{
	protected:
			DLLDynamicLoading::HuaweiPencilServiceDLL* _penServiceInvoker ;

            PencilNotifyWindows::PencilModeChangeNotifyWindow *_notifyWindowRef = nullptr;
	public:
		DLLDynamicLoading::HuaweiPencilServiceDLL* GetDllInvoker() {
			return _penServiceInvoker;
		}

        int AssociateNotifyWindow(PencilNotifyWindows::PencilModeChangeNotifyWindow* notifyWindowPtr) {
            if (_notifyWindowRef != notifyWindowPtr) {
                _notifyWindowRef = notifyWindowPtr;
                return 0;
            }
            
            return 1;
        }

		HuaweiPencilStatusManager(
			const std::wstring &loadLibPath = DLLDynamicLoading::HuaweiPencilServiceDLL::DefaultDLLPath,
            bool auto_sets_pen_function = true
		): _penServiceInvoker(DLLDynamicLoading::HuaweiPencilServiceDLL::GetSingletonInstancePtr(loadLibPath)){
			//_penServiceInvoker =&PenServiceDLL(loadLibPath);

            if (auto_sets_pen_function) {
                SetPencilFunctionToWindowsPenInput();
            }
		}

        bool SetPencilFunctionToWindowsPenInput() {
            _penServiceInvoker->CommandSendSetPenKeyFunc(2);
            _penServiceInvoker->CommandSendPenCurrentFunc(0);
            return true;
        }

		int DoSwitchPencilModeTask() override {

            const static int showTime = 1000;
			if (_penMode) {
				_penServiceInvoker->CommandSendPenCurrentFunc(0);
                /*_notifyWindowRef->SetIconPtrDisplay(

                    (PencilNotifyWindows::PencilModeChangeNotifyWindow::GetPenModeIconPtr())
                );*/
                _notifyWindowRef->SwitchPencilWindowStatus(PencilNotifyWindows::PencilModeNotifyWindowStatus::PENCIL);
                _notifyWindowRef->FadeInShow(false);
                _notifyWindowRef->SetAutoFadeOutHideTimer(showTime);
                
                

			}
			else {
				_penServiceInvoker->CommandSendPenCurrentFunc(1);
                //_notifyWindowRef->SetIconPtrDisplay(

                //    (PencilNotifyWindows::PencilModeChangeNotifyWindow::GetEraserModeIconPtr())
                //); 

                _notifyWindowRef->SwitchPencilWindowStatus(PencilNotifyWindows::PencilModeNotifyWindowStatus::ERASER);
                _notifyWindowRef->FadeInShow(false);
                _notifyWindowRef->SetAutoFadeOutHideTimer(showTime);
               
                
			}

			return 0;
		}
	};

    extern HHOOK hHook;
    extern bool isRunning;
    extern std::string logSavePath;
    extern DebugOutputType _debugModeType;




    extern HuaweiPencilStatusManager *pencilStatusManagerPtr;
    // Function to register the hotkey
    static int RegisterHotkeyFunctions() {
        // Register CTRL + SHIFT + H as the hotkey

        int error_count = 0;
        if (!RegisterHotKey(nullptr, PenButtonDoubleClick, MOD_NOREPEAT, VK_F19)) {
            error_count++;
        }

        if (!RegisterHotKey(nullptr, TestHotKeyFunction, MOD_WIN, VK_F3)) {
            error_count++;
        }
        return error_count; // 0x48 is the virtual key code for 'H'
    }

    // Function to unregister the hotkey
    static void UnregisterHotkey() {
        UnregisterHotKey(nullptr, PenButtonDoubleClick);
        UnregisterHotKey(nullptr, TestHotKeyFunction);
    }

    static int HandleHotkeyEvent(MSG const msg) {
        switch (msg.wParam) {
        case PenButtonDoubleClick:
            std::cout << GetTimeStampStr() << "PenButtonDoubleClick hotkey pressed!" << std::endl;
            break;
        case TestHotKeyFunction:
            std::cout << GetTimeStampStr() << "TestHotKeyFunction hotkey pressed!" << std::endl;
            break;
        default:
            break;
        }

        return 0;
    }

    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode == HC_ACTION) {
            PKBDLLHOOKSTRUCT pKbdStruct = (PKBDLLHOOKSTRUCT)lParam;

            if (wParam == WM_KEYDOWN && pKbdStruct->vkCode == VK_F19)
            {
                if ((GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000)) {
                    std::cout << GetTimeStampStr() << "Pencil Double click triggered." << std::endl;
                    if (pencilStatusManagerPtr){
                        pencilStatusManagerPtr->SwitchPencilModeCall();
                    }
                    
                }
            }
        }

        return CallNextHookEx(hHook, nCode, wParam, lParam);
    }

    static void SetHook() {
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
        if (!hHook) {
            DWORD error = GetLastError();
            std::cerr << "Failed to install hook! Code " << error << std::endl;
        }
    }

    static void Unhook() {
        if (hHook) {
            UnhookWindowsHookEx(hHook);
            hHook = NULL;
        }
    }
    
    // Function to set high priority
    static void SetHighPriority() {
        HANDLE hProcess = GetCurrentProcess();
        if (!SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS)) {
            DWORD error = GetLastError();
            // Handle error if needed
            std::cerr << "Cannot set program priority. Code "<< error << std::endl;

        }
    }

    static void StopMessageLoop() {
        isRunning = false;

    }

    static void OpenConsole() {
        AllocConsole();
        FILE* pCout;
        freopen_s(&pCout, "CONOUT$", "w", stdout);
        freopen_s(&pCout, "CONOUT$", "w", stderr);
        freopen_s(&pCout, "CONIN$", "r", stdin);
    }

    static DebugOutputType GetDebugModeType() {
        return _debugModeType;
    }




    
    static bool AlterDebugSettings(DebugOutputType newType) {
        _debugModeType = newType;
        errno_t err = 0;
        FILE* stream;

        switch (newType) {
        case DebugOutputType::DEBUG_WINDOW: {
            OpenConsole();
        }
        break;
        case DebugOutputType::DEBUG_IGNORE_OUTPUT:
        {
#ifdef _WIN32
            //freopen_s("NUL", "w", stdout);
            err = freopen_s(&stream, "NUL", "at", stdout);

            freopen_s(&stream, "NUL", "at", stderr);
            
#else
            err = freopen_s(&stream, "/dev/null", "w", stdout);
            freopen_s(&stream, "/dev/null", "w", stderr);
#endif
            if (err != 0) {

                MessageBox(NULL, L"Failed to redirect stdout", L"Log Set Error", MB_OK | MB_ICONERROR);
                return false;
            }
        }
        break;
        case DebugOutputType::DEBUG_TEMP_LOG_FILE:
        {
            try {
                err = freopen_s(&stream, logSavePath.c_str(), "at", stdout);
                freopen_s(&stream, logSavePath.c_str(),"at", stderr);
                if (err != 0) {

                    MessageBox(NULL, L"Failed to redirect stdout", L"Log Set Error", MB_OK | MB_ICONERROR);
                    return false;
                }
            }
            catch (const std::exception& ex) {
                // Show error message in a Win32 message box

                std::wstringstream wss;

                wss << "Cannot set log file to \"" << logSavePath.c_str() << "\" ,\n";
                wss << ex.what();
                MessageBox(NULL, wss.str().c_str(), L"Log Set Error", MB_OK | MB_ICONERROR);

                AlterDebugSettings(DebugOutputType::DEBUG_WINDOW);
            }

        }

        break;
        default:
            break;

        }


        return true;
    }
}
