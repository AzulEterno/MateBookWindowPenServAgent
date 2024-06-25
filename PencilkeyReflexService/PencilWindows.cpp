#include "PencilWindows.h"


namespace PencilNotifyWindows {
    const wchar_t PencilModeChangeNotifyWindow::CLASS_NAME[] = L"BlurBackgroundWindowClass";

    HICON PencilModeChangeNotifyWindow::_penModeIcon = NULL;
    HICON PencilModeChangeNotifyWindow::_eraserModeIcon = NULL;

    PencilModeChangeNotifyWindow::PencilModeChangeNotifyWindow(HINSTANCE hInstance) : _hInstance(hInstance), _hWnd(nullptr) {
    }

    PencilModeChangeNotifyWindow::~PencilModeChangeNotifyWindow() {
        if (_hWnd) {
            DestroyWindow(_hWnd);
        }
    }

    bool PencilModeChangeNotifyWindow::Create(LPCWSTR windowTitle, LPCWSTR text, int width, int height) {
        _infoText = text;

        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = PencilModeChangeNotifyWindow::WindowProc;
        wc.hInstance = _hInstance;
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

        if (!RegisterClassEx(&wc)) {
            return false;
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
            return false;
        }

        UpdateDPIValue();

        // Apply blur effect
        DWM_BLURBEHIND bb = {};
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = TRUE;
        bb.hRgnBlur = NULL;
        DwmEnableBlurBehindWindow(_hWnd, &bb);

        
       
        //HRESULT hr = ApplySystemBackdrop(_hWnd, DWM_SYSTEMBACKDROP_TYPE::DWMSBT_MAINWINDOW);
        //if (FAILED(hr)) {
        //    return false;
        //}
        //AlterTransparency(0.9);

        return true;
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
        blendFunction.SourceConstantAlpha = cal_transp_val;

        bool result = UpdateLayeredWindow(
            _hWnd,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            RGB(255,255,255),
            &blendFunction,
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

    void PencilModeChangeNotifyWindow::OnPaint() {
        HDC hdcScreen = GetDC(_hWnd);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);
        SIZE realWindowSize = GetRealWindowSize();

        RECT wndRect = { 0, 0, realWindowSize.cx, realWindowSize.cy };
        HBITMAP hbmMem = CreateCompatibleBitmap(hdcScreen,
            realWindowSize.cx, realWindowSize.cy);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

        // Fill the background with transparency
        HBRUSH hBrush = CreateSolidBrush(bg_color);


        FillRect(hdcMem, &wndRect, hBrush);
        DeleteObject(hBrush);

        // Set text properties
        SetTextColor(hdcMem, RGB(10, 10, 10));
        SetBkMode(hdcMem, TRANSPARENT);

        // Draw text
        //DrawText(hdcMem, _infoText.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        if (_displayIconPtr)
        {
            int iconWidth = realWindowSize.cx*0.9;  // Specify your desired icon width
            int iconHeight = realWindowSize.cy * 0.9;
            // Draw the icon
            int pX = (realWindowSize.cx - iconWidth) / 2;
            int pY = (realWindowSize.cy - iconHeight) / 2;
        DrawIconEx(hdcMem, pX, pY, *_displayIconPtr, iconWidth, iconHeight,
            0, NULL, DI_NORMAL);
                
            
        }
         
        POINT ptSrc = { 0, 0 };

        UpdateLayeredWindow(_hWnd, hdcScreen, NULL,
            &realWindowSize, hdcMem,
            &ptSrc, RGB(255, 255, 255),
            &blendFunction, ULW_ALPHA);

        // Cleanup
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
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
            blendFunction.SourceConstantAlpha = 0;
        }

        SetTimer(_hWnd, static_cast<WPARAM>(PencilModeChangeNotifyTimerMessage::FADE_ANIMATION),
            _fadeAnimationIntervalMilliseconds, NULL);
    }
    void PencilModeChangeNotifyWindow::FadeOutHide(bool animateForceRestart = 0) {
        KillFadeAnimationTimer();
        KillAutoHideTimer();
        
        _fadeAnimationState = -1;

        if (animateForceRestart) {
            blendFunction.SourceConstantAlpha = 1;
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
            case WM_DPICHANGED:
                pThis->UpdateDPIValue();
                break;
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