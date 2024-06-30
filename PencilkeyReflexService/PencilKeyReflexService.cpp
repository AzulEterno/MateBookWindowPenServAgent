// PencilKeyReflexService.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include "DLL_Loader.h"
#include "PencilStatusManager.h"
#include "PencilWindows.h"
#include "utilities.h"

using namespace PencilNotifyWindows;

int DoMessageMonitorJob(HINSTANCE hInstance, int nCmdShow, const std::wstring dllPath) {
    static const bool use_set_windows_hook_ex = true;
    PencilModeChangeNotifyWindow::InitIconData(hInstance);
    PencilModeChangeNotifyWindow window(hInstance);

    if (!window.Create(L"Blur Information Window", L"Information Text", 150, 150)) {
        return 0;
    }

    //window.Show();
    //window.SetAutoHideTimer(1000);

    window.AlterTransparency(0.0);

    PencilKeyReflex::pencilStatusManagerPtr = new PencilKeyReflex::HuaweiPencilStatusManager(dllPath);

    auto penServiceDll = PencilKeyReflex::pencilStatusManagerPtr->GetDllInvoker();

    if (penServiceDll->GetDllLoadStatus() != 0) {

        std::wstringstream wss;
        wss << "Failed to load dll, maybe it's not in the provided path or it's the wrong file.";
        wss << "DllTriedPath:" << dllPath << std::endl;
        
        MessageBox(NULL, wss.str().c_str(), L"Error in Loading DLL", MB_OK | MB_ICONERROR);
        return -1;
    }


    PencilKeyReflex::pencilStatusManagerPtr->AssociateNotifyWindow(&window);

    if (!use_set_windows_hook_ex) {
        int result = PencilKeyReflex::RegisterHotkeyFunctions();
        if (result) {
            std::cerr << GetTimeStampStr() << "Failed to register hotkey!" << result << std::endl;
            return 1;
        }

        std::cout << GetTimeStampStr() << "Hotkey registered. Press pencil double click to trigger it." << std::endl;
    }
    else {
        PencilKeyReflex::SetHook();

        std::cout << GetTimeStampStr() << "Use Windows Hook to register key event." << std::endl;
    }

    fflush(stdout);
    fflush(stderr);
    // Message loop
    MSG msg = {};
    while (PencilKeyReflex::isRunning &&GetMessage(&msg, nullptr, 0, 0)) {
        if (!use_set_windows_hook_ex && msg.message == WM_HOTKEY) {
            PencilKeyReflex::HandleHotkeyEvent(msg);

        }
        else if (msg.message == WM_POWERBROADCAST) {
            switch (msg.wParam) {
            case PBT_APMSUSPEND:
                std::cout << GetTimeStampStr() << "Receive sleep message!" << std::endl;
                
                //PencilKeyReflex::StopMessageLoop();
                break;
            case PBT_APMRESUMESUSPEND:
                std::cout << GetTimeStampStr() << "System has resumed from sleep mode!" << std::endl;
                PencilKeyReflex::pencilStatusManagerPtr->SetPencilFunctionToWindowsPenInput();
                
                break;
            default:
                break;
            }
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    std::cout << GetTimeStampStr() << "About to exit the program...";

    PencilKeyReflex::UnregisterHotkey();
    PencilKeyReflex::Unhook();
    window.Hide();
    window.Destroy();
    delete PencilKeyReflex::pencilStatusManagerPtr;
    PencilModeChangeNotifyWindow::ReleaseIconData();
    return 0;
}


// Function to split a command line string into individual arguments
static std::vector<std::string> splitCommandLine(const std::string& cmdLine) {
    std::istringstream iss(cmdLine);
    std::vector<std::string> args;
    std::string arg;
    while (iss >> std::quoted(arg)) {
        args.push_back(arg);
    }
    return args;
}






int APIENTRY WinMain(
    _In_ HINSTANCE hInstance,      // handle to current instance
    _In_opt_ HINSTANCE hPrevInstance,  // handle to previous instance
    _In_ LPSTR lpCmdLine,          // command line
    _In_ int nCmdShow              // show state
) {
    
    bool validateLogFilePath = false;
    int debug_val = 0;
    std::wstring dllInvokerPath = L"PenService.dll";
    //std::string logFilePath;

    // Convert lpCmdLine to a std::string
    std::string cmdLine(lpCmdLine);

    // Split the command line into individual arguments
    std::vector<std::string> args = splitCommandLine(cmdLine);

    // Parse command line arguments
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--debugtype" && i + 1 < args.size()) {
            debug_val = std::stoi(args[i + 1]);
            i++; // Skip the next argument since it's the value
        }
        else if (args[i] == "--logfile" && i + 1 < args.size()) {
            PencilKeyReflex::logSavePath = args[i + 1];

            
            i++; // Skip the next argument since it's the value
        }
        else if (args[i] == "--dllpath" && i + 1 < args.size()) {

            dllInvokerPath = std::wstring(args[i + 1].begin(), args[i + 1].end());
            i++; // Skip the next argument since it's the value
        }
        else if (args[i] == "--verifypath") {
            validateLogFilePath = true;
        }
    }

    if (validateLogFilePath) {
        std::wstringstream wss;
        wss << "LogSavePath:" <<std::wstring(PencilKeyReflex::logSavePath.begin(), PencilKeyReflex::logSavePath.end()) << std::endl;

        wss << "DllPath:" << dllInvokerPath << std::endl;
        MessageBox(NULL, wss.str().c_str(), L"Path Verification", MB_OK | MB_ICONINFORMATION);
    }


    if (debug_val < 0 || debug_val > static_cast<int>(PencilKeyReflex::DebugOutputType::DEBUG_TEMP_LOG_FILE)) {
        debug_val = 0;
    }
    PencilKeyReflex::AlterDebugSettings(static_cast<PencilKeyReflex::DebugOutputType>(debug_val));

    DoMessageMonitorJob( hInstance, nCmdShow, dllInvokerPath);



    if (PencilKeyReflex::GetDebugModeType() == PencilKeyReflex::DebugOutputType::DEBUG_WINDOW) {
        FreeConsole();
    }
    else if (PencilKeyReflex::GetDebugModeType() == PencilKeyReflex::DebugOutputType::DEBUG_TEMP_LOG_FILE) {
        fflush(stdout);
        fflush(stderr);
        
        fclose(stdin);
        fclose(stdout);
    }
    return 0;
}