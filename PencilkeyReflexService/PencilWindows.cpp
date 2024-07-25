#include "PencilWindows.h"


namespace PencilNotifyWindows {
    const wchar_t PencilModeChangeNotifyWindow::CLASS_NAME[] = L"BlurBackgroundWindowClass";

    HICON PencilModeChangeNotifyWindow::_penModeIcon = NULL;
    HICON PencilModeChangeNotifyWindow::_eraserModeIcon = NULL;
    //std::vector<WNDPROC> _subWndProcHandlers = std::vector<WNDPROC>();

    COLORREF InterpolateColorRef(COLORREF color1, COLORREF color2, float t) {
        BYTE r1 = GetRValue(color1);
        BYTE g1 = GetGValue(color1);
        BYTE b1 = GetBValue(color1);
        BYTE r2 = GetRValue(color2);
        BYTE g2 = GetGValue(color2);
        BYTE b2 = GetBValue(color2);

        BYTE r = static_cast<BYTE>(r1 + t * (r2 - r1));
        BYTE g = static_cast<BYTE>(g1 + t * (g2 - g1));
        BYTE b = static_cast<BYTE>(b1 + t * (b2 - b1));

        return RGB(r, g, b);
    }

    float GetColorRefInterpolateStepValue(COLORREF color1, COLORREF color2, COLORREF ref_color) {

        // Extract the RGB components of the colors
        BYTE r1 = GetRValue(color1);
        BYTE g1 = GetGValue(color1);
        BYTE b1 = GetBValue(color1);
        BYTE r2 = GetRValue(color2);
        BYTE g2 = GetGValue(color2);
        BYTE b2 = GetBValue(color2);
        BYTE rf = GetRValue(ref_color);
        BYTE gf = GetGValue(ref_color);
        BYTE bf = GetBValue(ref_color);

        // Calculate the interpolation factor for each color channel
        float r_t = (r2 - r1) != 0 ? static_cast<float>(rf - r1) / (r2 - r1) : 0.0f;
        float g_t = (g2 - g1) != 0 ? static_cast<float>(gf - g1) / (g2 - g1) : 0.0f;
        float b_t = (b2 - b1) != 0 ? static_cast<float>(bf - b1) / (b2 - b1) : 0.0f;

        // Return the average interpolation factor
        float t = (r_t + g_t + b_t) / 3.0f;
        return t;
    }

    PencilModeChangeNotifyWindow::PencilModeChangeNotifyWindow(HINSTANCE hInstance) : _hInstance(hInstance), _hWnd(nullptr) {
    }

    PencilModeChangeNotifyWindow::~PencilModeChangeNotifyWindow() {
        ReleaseCanvasObjects();


        if (_hWnd) {
            DestroyWindow(_hWnd);
        }

    }




    int PencilModeChangeNotifyWindow::Create(LPCWSTR windowTitle, LPCWSTR text, int width, int height) {
        _infoText = text;

        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = PencilModeChangeNotifyWindow::WindowProc;
        wc.hInstance = _hInstance;
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(TRANSPARENT);

        wc.style = CS_HREDRAW | CS_VREDRAW;  // Ensure appropriate styles

        if (!RegisterClassEx(&wc)) {
            return 1;
        }


        SIZE _wndLogicSize = { width,height };
        SetWindowLogicSize(_wndLogicSize);

        SIZE screenSize = GetRealScreenSize();
        SIZE windowSize = GetRealWindowSize();
        int xPos = (screenSize.cx - windowSize.cx) / 2;
        int yPos = (screenSize.cy - windowSize.cy) / 2;

        _hWnd = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_LAYERED,
            CLASS_NAME,
            windowTitle,
            WS_POPUP,
            xPos, yPos, screenSize.cx, screenSize.cy,
            NULL,
            NULL,
            _hInstance,
            this
        );

        if (!_hWnd) {
            return -1;
        }

        UpdateDPIValue(true,false);
        SyncWithSystemDarkMode(false);
        UpdateThemeData(false);

        // Apply blur effect
        DWM_BLURBEHIND dwm_bb = {};
        dwm_bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        dwm_bb.fEnable = TRUE;
        dwm_bb.hRgnBlur = NULL;
        DwmEnableBlurBehindWindow(_hWnd, &dwm_bb);

        ReInitCanvasObjects(true);
       
        //HRESULT hr = ApplySystemBackdrop(_hWnd, DWM_SYSTEMBACKDROP_TYPE::DWMSBT_MAINWINDOW);
        //if (FAILED(hr)) {
        //    return false;
        //}
        //AlterTransparency(0.9);

        return 0;
    }

    bool PencilModeChangeNotifyWindow::AlterTransparency(float  val) {
        if (val > 1) {
            val = 1;
        }
        else if (val < 0) {
            val = 0;
        }
        UINT8 cal_transp_val = (val * (float)0xFF);

        //bool result = SetLayeredWindowAttributes(_hWnd, NULL, cal_transp_val, LWA_ALPHA);
        //CallRedrawWindow();
        _blendFunction.SourceConstantAlpha = cal_transp_val;

        bool result = UpdateLayeredWindow(
            _hWnd,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            _render_BG_color,
            &_blendFunction,
            ULW_ALPHA
        );
        return result;
    }

    HRESULT PencilModeChangeNotifyWindow::ApplyBlurEffect(HWND hwnd) {
        HRESULT hr = S_OK;
#ifdef TestCodeBlock
        {
            hr = DCompositionCreateDevice(NULL, __uuidof(IDCompositionDevice), (void**)&pDCompDevice_);
            if (FAILED(hr)) return hr;

            hr = pDCompDevice_->CreateTargetForHwnd(hwnd, TRUE, &pDCompTarget_);
            if (FAILED(hr)) return hr;

            hr = pDCompDevice_->CreateVisual(&pDCompVisual_);
            if (FAILED(hr)) return hr;

            pDCompTarget_->SetRoot(pDCompVisual_);

            IDCompositionBlurEffect* pBlurEffect = nullptr;
            hr = pDCompDevice_->CreateBlurEffect(&pBlurEffect);
            if (FAILED(hr)) return hr;

            pBlurEffect->SetStandardDeviation(30.0f); // Increase the blur radius

            pDCompVisual_->SetEffect(pBlurEffect);
            pBlurEffect->Release();

            hr = pDCompDevice_->Commit();
        }
#endif
        return hr;
    }

    bool PencilModeChangeNotifyWindow::ReInitCanvasObjects(bool call_redraw ) {
        ReleaseCanvasObjects();
        if (_hWnd) {
            HDC hdcScreen = GetDC(_hWnd);
            _hdcMem = CreateCompatibleDC(hdcScreen);
            SIZE realWindowSize = GetRealWindowSize();

            // Initialize the first bitmap
            _hbm_PencilNotification = CreateCompatibleBitmap(hdcScreen, realWindowSize.cx, realWindowSize.cy);
            // Initialize the second bitmap
            _hbm_EraserNotification = CreateCompatibleBitmap(hdcScreen, realWindowSize.cx, realWindowSize.cy);

            

            // Draw numbers on the bitmaps
            //DrawNumberOnBitmap(_hbm_PencilNotification, 1);
            //DrawNumberOnBitmap(_hbm_EraserNotification, 2);
            
            if (call_redraw) {
                RedrawWindowCanvas();
            }

            // Set the current bitmap
            _currentHbmMem = _hbm_PencilNotification;
            // Select the first bitmap into the memory DC by default
            _hbmOld = (HBITMAP)SelectObject(_hdcMem, _currentHbmMem);
            ReleaseDC(NULL, hdcScreen);

            return true;
        }

        return false;
        
    }

    bool PencilModeChangeNotifyWindow::RedrawWindowCanvas() {
        DrawBitMapCanvas(_hbm_PencilNotification, PencilNotifyWindows::PencilModeChangeNotifyWindow::GetPenModeIconPtr(),L"Pencil");
        DrawBitMapCanvas(_hbm_EraserNotification, PencilNotifyWindows::PencilModeChangeNotifyWindow::GetEraserModeIconPtr(),L"Eraser");

        return true;
    }

    void PencilModeChangeNotifyWindow::ReleaseCanvasObjects() const{
        if (_hdcMem) {

            if (_hbmOld)
                SelectObject(_hdcMem, _hbmOld); // Restore the original object

            DeleteDC(_hdcMem);              // Delete the device context
        }
        if (_hbm_PencilNotification)
            DeleteObject(_hbm_PencilNotification);         // Delete the first bitmap
        if (_hbm_EraserNotification)
            DeleteObject(_hbm_EraserNotification);         // Delete the second bitmap
    }


    void PencilModeChangeNotifyWindow::DrawBitMapCanvas(HBITMAP hBitmap, HICON displayIcon, std::wstring displayString) {
        HDC hdcScreen = GetDC(_hWnd);
        HDC hdcTemp = CreateCompatibleDC(hdcScreen);
        HBITMAP hbmOldTemp = (HBITMAP)SelectObject(hdcTemp, hBitmap);

        SIZE realWindowSize = GetRealWindowSize();
        RECT wndRect = { 0, 0, realWindowSize.cx, realWindowSize.cy };

        // Fill the background with transparency or system bg color
        HBRUSH hBrush = CreateSolidBrush(sysBGColor);
        FillRect(hdcTemp, &wndRect, hBrush);
        DeleteObject(hBrush);


        // Set text properties
        SetTextColor(hdcTemp, sysFGColor);
        SetBkMode(hdcTemp, TRANSPARENT); // Set background mode to transparent
        

        // Draw the icon
        if (displayIcon) {
            int textHeightFactor = (displayString.empty()) ? 0 : 0.2;
            int iconWidth = realWindowSize.cx * (0.9 - textHeightFactor);
            int iconHeight = realWindowSize.cy * (0.9 - textHeightFactor);
            int pX = (realWindowSize.cx - iconWidth) / 2;
            int pY = (realWindowSize.cy - iconHeight - realWindowSize.cy * textHeightFactor) / 2;

            // Create a temporary bitmap for the icon with color transformation
            HBITMAP hbmIcon = CreateCompatibleBitmap(hdcScreen, iconWidth, iconHeight);
            HDC hdcIcon = CreateCompatibleDC(hdcScreen);
            HBITMAP hbmOldIcon = (HBITMAP)SelectObject(hdcIcon, hbmIcon);

            // Fill the temporary bitmap with transparency
            HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // Using white for transparency
            RECT iconRect = { 0, 0, iconWidth, iconHeight };
            FillRect(hdcIcon, &iconRect, hBrush);
            DeleteObject(hBrush);
            // Draw the icon onto the temporary bitmap
            DrawIconEx(hdcIcon, 0, 0, displayIcon, iconWidth, iconHeight, 0, NULL, DI_NORMAL);

            float ref_step = 0.0;
            // Transform the black color to sysFGColor while preserving transparency
            for (int y = 0; y < iconHeight; y++) {
                for (int x = 0; x < iconWidth; x++) {
                    COLORREF color = GetPixel(hdcIcon, x, y);
                    //if (color == RGB(0, 0, 0)) {
                    //    SetPixel(hdcIcon, x, y, sysFGColor);
                    //}
                    //else if (color == RGB(255, 255, 255)) {
                    //    SetPixel(hdcIcon, x, y, sysBGColor);
                    //}
                    ref_step = GetColorRefInterpolateStepValue(RGB(255, 255, 255), RGB(0, 0, 0),color);
                    //1.0 means foreground, 0.0 means background;
                    ref_step = std::clamp(ref_step,0.0f,1.0f);

                    SetPixel(hdcIcon, x, y, InterpolateColorRef(sysBGColor,sysFGColor,ref_step));

                }
            }

            // Draw the transformed icon onto the temporary bitmap
            BitBlt(hdcTemp, pX, pY, iconWidth, iconHeight, hdcIcon, 0, 0, SRCCOPY);



            // Cleanup
            SelectObject(hdcIcon, hbmOldIcon);
            DeleteObject(hbmIcon);
            DeleteDC(hdcIcon);
        }


        // Draw the text at the bottom center if it is not empty
        if (!displayString.empty()) {
            RECT textRect;
            textRect.left = 0;
            textRect.right = realWindowSize.cx;
            textRect.top = realWindowSize.cy * 0.75; // Adjust this value to fit the icon and text nicely
            textRect.bottom = realWindowSize.cy;

            // Get DPI scale and calculate font size
            float dpiScale = GetDPIScale();
            int fontSize = static_cast<int>(16 * dpiScale); // 16 is the base font size

            // Create font with scaled size
            HFONT hFont = CreateFont(
                fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI")
            );

            HFONT hOldFont = (HFONT)SelectObject(hdcTemp, hFont);

            DrawText(hdcTemp, displayString.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Cleanup font
            SelectObject(hdcTemp, hOldFont);
            DeleteObject(hFont);
        }

        // Check if the bitmap is 32-bit and set alpha channel to _wndBaseAlpha
        BITMAP bm;
        GetObject(hBitmap, sizeof(BITMAP), &bm);
        if (bm.bmBitsPixel == 32) {
            BITMAPINFO bmi = { 0 };
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = bm.bmWidth;
            bmi.bmiHeader.biHeight = -bm.bmHeight; // top-down
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            void* bits = nullptr;
            HBITMAP hbmAlpha = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);

            HDC hdcAlpha = CreateCompatibleDC(hdcScreen);
            HBITMAP hbmOldAlpha = (HBITMAP)SelectObject(hdcAlpha, hbmAlpha);

            // Copy existing bitmap to DIB section
            BitBlt(hdcAlpha, 0, 0, bm.bmWidth, bm.bmHeight, hdcTemp, 0, 0, SRCCOPY);

            // Set alpha channel to _wndBaseAlpha
            BYTE* pixel = (BYTE*)bits;
            for (int y = 0; y < bm.bmHeight; y++) {
                for (int x = 0; x < bm.bmWidth; x++) {
                    pixel[3] = _wndBaseAlpha; // set alpha to _wndBaseAlpha
                    pixel += 4; // move to next pixel
                }
            }

            // Draw the DIB section with the alpha channel
            BitBlt(hdcTemp, 0, 0, bm.bmWidth, bm.bmHeight, hdcAlpha, 0, 0, SRCCOPY);

            // Cleanup
            SelectObject(hdcAlpha, hbmOldAlpha);
            DeleteObject(hbmAlpha);
            DeleteDC(hdcAlpha);
        }
        // Cleanup
        SelectObject(hdcTemp, hbmOldTemp);
        DeleteDC(hdcTemp);
        ReleaseDC(NULL, hdcScreen);
    }


    // We may keep multiple hdc Mem in the future so we can just switch instead of redrawing...
    void PencilModeChangeNotifyWindow::OnPaint() {
        static POINT ptSrc = { 0, 0 };
        HDC hdcScreen = GetDC(_hWnd);
        SIZE realWindowSize = GetRealWindowSize();
        

        // Select the current bitmap into _hdcMem before updating the window
        SelectObject(_hdcMem, _currentHbmMem);

        UpdateLayeredWindow(_hWnd, hdcScreen, NULL, &realWindowSize, _hdcMem, &ptSrc, _render_BG_color, &_blendFunction, ULW_ALPHA);

        ReleaseDC(NULL, hdcScreen);
        //ReleaseDC(NULL, hdcScreen);
    }

    void PencilModeChangeNotifyWindow::Show() {
        ShowWindow(_hWnd, SW_SHOW);{
            UpdateWindow(_hWnd);
            //InvalidateRect(hwnd_, NULL, TRUE);  // Force a paint message
            CallRedrawWindow();
        }
        
    }

    void PencilModeChangeNotifyWindow::FadeInShow(bool animateForceRestart = 0) {
        KillFadeAnimationTimer();
        Show();
        _fadeAnimationState = 1;

        if (animateForceRestart) {
            _blendFunction.SourceConstantAlpha = 0;
        }

        SetTimer(_hWnd, static_cast<WPARAM>(PencilModeChangeNotifyTimerMessage::FADE_ANIMATION),
            _fadeAnimationIntervalMilliseconds, NULL);
    }
    void PencilModeChangeNotifyWindow::FadeOutHide(bool animateForceRestart = 0) {
        KillFadeAnimationTimer();
        KillAutoHideTimer();
        
        _fadeAnimationState = -1;

        if (animateForceRestart) {
            _blendFunction.SourceConstantAlpha = 1;
        }

        SetTimer(_hWnd, static_cast<WPARAM>(PencilModeChangeNotifyTimerMessage::FADE_ANIMATION),
            _fadeAnimationIntervalMilliseconds, nullptr);
    }

    void PencilModeChangeNotifyWindow::Hide() {
        {
            ShowWindow(_hWnd, SW_HIDE);
            KillAutoHideTimer();
            KillFadeAnimationTimer();
        }
    }

    void PencilModeChangeNotifyWindow::SetAutoHideTimer(UINT milliseconds) {
        KillAutoHideTimer();
        _hideTimerId = SetTimer(_hWnd, static_cast<WPARAM>(PencilModeChangeNotifyTimerMessage::HIDE), milliseconds, NULL);
    }
    void PencilModeChangeNotifyWindow::SetAutoFadeOutHideTimer(UINT milliseconds) {
        KillAutoHideTimer();
        _hideTimerId = SetTimer(_hWnd, static_cast<WPARAM>(PencilModeChangeNotifyTimerMessage::FADEOUT_HIDE), milliseconds, NULL);
    }
    void PencilModeChangeNotifyWindow::KillAutoHideTimer() {
        if (_hideTimerId) {
            KillTimer(_hWnd, _hideTimerId);
            _hideTimerId = 0;
        }
    }

    void PencilModeChangeNotifyWindow::KillFadeAnimationTimer() {
        if (_fadeAnimationTimerId) {
            KillTimer(_hWnd, _fadeAnimationTimerId);
            _fadeAnimationTimerId = NULL;
            _fadeAnimationState = 0;
        }
    }

    LRESULT CALLBACK PencilModeChangeNotifyWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        PencilModeChangeNotifyWindow* pThis = nullptr;

        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = reinterpret_cast<PencilModeChangeNotifyWindow*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

            pThis->_hWnd = hWnd;
        }
        else {
            pThis = reinterpret_cast<PencilModeChangeNotifyWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        }

         

        if (pThis) {
            switch (uMsg) {
            case WM_SETTINGCHANGE:
                // Format debug information for wParam and lParam
                //wchar_t debugInfo[256]; // lParam = "ImmersiveColorSet"
                //swprintf_s(debugInfo, L"WM_SETTINGCHANGE received. wParam: %lu, lParam: %p, str: %s\n", wParam, (void*)lParam, (wchar_t *)lParam);
                //OutputDebugString(debugInfo);
                if (wParam == NULL || lstrcmpW((LPCWSTR)lParam,L"ImmersiveColorSet") == 0) {
                    // Possible change in theme or appearance
                    // You may need to check if the change is related to dark mode
                    pThis->SyncWithSystemDarkMode(true);
                }
                break;
            case WM_THEMECHANGED:
                
                pThis->SyncWithSystemDarkMode(true);
                break;
            case WM_DPICHANGED:
                pThis->UpdateDPIValue(true,true);
                break;
            case WM_DISPLAYCHANGE:
                {
                    pThis->AutoResizeWindow();
                    pThis->ModifyRoundedCorner();
                    pThis->ReInitCanvasObjects(true);
                }break;
            case WM_PAINT:
                pThis->OnPaint();
                break;
            case WM_SIZE:
                pThis->CallRedrawWindow();
                break;
            case WM_DESTROY:
                DestroyWindow(pThis->_hWnd);
                break;
            case WM_TIMER:
                switch(wParam) {
                case static_cast<WPARAM>(PencilModeChangeNotifyTimerMessage::HIDE):
                    pThis->Hide();
                    break;
                case static_cast<WPARAM>(PencilModeChangeNotifyTimerMessage::FADEOUT_HIDE):
                    pThis->FadeOutHide();
                    break;
                case static_cast<WPARAM>(PencilModeChangeNotifyTimerMessage::FADE_ANIMATION): {
                    float newAlpha = pThis->GetWindowAlphaValue();
                    float deltaAlpha = (pThis->_fadeStepValue * pThis->_fadeAnimationState);
                    newAlpha += deltaAlpha;
                    {
                        if (pThis->_fadeAnimationState != 0){
                            if (newAlpha > 1.0f) {
                                newAlpha = 1.0f;
                                pThis->KillFadeAnimationTimer();
                            }
                            else if (newAlpha < 0.0f) {
                                newAlpha = 0.0f;
                                pThis->Hide();
                            }
                        pThis->AlterTransparency(newAlpha);
                        }
                    }
                }break;
                default:
                    break;
                }
                break;
            }


            //return 0;
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }


    HRESULT PencilModeChangeNotifyWindow::ApplySystemBackdrop(HWND const &hwnd, 
        DWM_SYSTEMBACKDROP_TYPE const &bd_type = DWM_SYSTEMBACKDROP_TYPE::DWMSBT_AUTO) {
        HRESULT hr = S_OK;

        // Enable Mica on the window using DWMWA_SYSTEMBACKDROP_TYPE
        const MARGINS margins = { -1 };
        hr = DwmExtendFrameIntoClientArea(hwnd, &margins);
        if (SUCCEEDED(hr)) {
            hr = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &bd_type, sizeof(bd_type));
        }


        return hr;
    }
}