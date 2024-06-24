#pragma once
#include "pch.h"
#include "PencilWindows.h"
namespace PencilKeyReflex{
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
                _penServiceInvoker->CommandSendSetPenKeyFunc(2);
                _penServiceInvoker->CommandSendPenCurrentFunc(0);
            }
		}

		int DoSwitchPencilModeTask() override {

            const static int showTime = 1000;
			if (_penMode) {
				_penServiceInvoker->CommandSendPenCurrentFunc(0);
                _notifyWindowRef->SetIconPtrDisplay(

                    (PencilNotifyWindows::PencilModeChangeNotifyWindow::GetPenModeIconPtr())
                );
                _notifyWindowRef->FadeInShow(false);
                _notifyWindowRef->SetAutoFadeOutHideTimer(showTime);
                
                

			}
			else {
				_penServiceInvoker->CommandSendPenCurrentFunc(1);
                _notifyWindowRef->SetIconPtrDisplay(

                    (PencilNotifyWindows::PencilModeChangeNotifyWindow::GetEraserModeIconPtr())
                ); 
                _notifyWindowRef->FadeInShow(false);
                _notifyWindowRef->SetAutoFadeOutHideTimer(showTime);
               
                
			}

			return 0;
		}
	};

    extern HHOOK hHook;
    extern bool isDebugMode;

    extern HuaweiPencilStatusManager pencilStatusManager;
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
            if (isDebugMode) {
                std::wcout << "PenButtonDoubleClick hotkey pressed!" << std::endl;
            }
            break;
        case TestHotKeyFunction:
            if (isDebugMode) {
                std::wcout << "TestHotKeyFunction hotkey pressed!" << std::endl;
            }
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
                    if (isDebugMode) {
                        std::wcout << "Pencil Double click triggered." << std::endl;
                    }
                    pencilStatusManager.SwitchPencilModeCall();
                }
            }
        }

        return CallNextHookEx(hHook, nCode, wParam, lParam);
    }

    static void SetHook() {
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
        if (!hHook) {
            DWORD error = GetLastError();
            if (isDebugMode) {
                std::wcerr << "Failed to install hook! Code " << error << std::endl;
            }
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
            if (isDebugMode) {
                std::wcerr << "Cannot set program priority. Code "<< error << std::endl;

            }
        }
    }
}
