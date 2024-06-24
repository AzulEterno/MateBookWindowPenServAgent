// PencilKeyReflexService.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include "DLL_Loader.h"
#include "PencilStatusManager.h"
#include "PencilWindows.h"

using namespace PencilNotifyWindows;

int DoMessageMonitorJob(HINSTANCE hInstance, int nCmdShow) {
    static const bool use_set_windows_hook_ex = true;
    PencilModeChangeNotifyWindow::InitIconData(hInstance);
    PencilModeChangeNotifyWindow window(hInstance);

    if (!window.Create(L"Blur Background Window", L"Information Text", 200, 200)) {
        return 0;
    }

    //window.Show();
    //window.SetAutoHideTimer(1000);

    window.AlterTransparency(0.0);


    auto penServiceDll = PencilKeyReflex::pencilStatusManager.GetDllInvoker();


    PencilKeyReflex::pencilStatusManager.AssociateNotifyWindow(&window);

    if (!use_set_windows_hook_ex) {
        int result = PencilKeyReflex::RegisterHotkeyFunctions();
        if (result) {
            std::cerr << "Failed to register hotkey!" << result << std::endl;
            return 1;
        }

        std::cout << "Hotkey registered. Press pencil double click to trigger it." << std::endl;
    }
    else {
        PencilKeyReflex::SetHook();

    }
    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!use_set_windows_hook_ex && msg.message == WM_HOTKEY) {
            PencilKeyReflex::HandleHotkeyEvent(msg);

        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


    PencilKeyReflex::UnregisterHotkey();
    PencilKeyReflex::Unhook();
    window.Hide();
    window.Destroy();
    PencilModeChangeNotifyWindow::ReleaseIconData();
    return 0;
}

void OpenConsole() {
    AllocConsole();
    FILE* pCout;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    freopen_s(&pCout, "CONOUT$", "w", stderr);
    freopen_s(&pCout, "CONIN$", "r", stdin);
}







int APIENTRY WinMain(
    _In_ HINSTANCE hInstance,      // handle to current instance
    _In_opt_ HINSTANCE hPrevInstance,  // handle to previous instance
    _In_ LPSTR lpCmdLine,          // command line
    _In_ int nCmdShow              // show state
) {
    std::string cmdLine(lpCmdLine);
    if (cmdLine.find("--debug") != std::string::npos) {
        OpenConsole();
        PencilKeyReflex::isDebugMode = true;
        std::cout << "Debug mode enabled." << std::endl;
    }
    DoMessageMonitorJob( hInstance, nCmdShow);

    if (PencilKeyReflex::isDebugMode) {
        FreeConsole();
    }
    return 0;
}