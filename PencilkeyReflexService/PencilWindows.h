#pragma once
#include "pch.h"



namespace PencilNotifyWindows {

    //void GetThemeMode() {
    //
    //    if (RUNNING_ON_WINDOWS_10) {
    //        winrt::Windows::UI::ViewManagement::UISettings settings;
    //        auto background = settings.GetColorValue(winrt::Windows::UI::ViewManagement::UIColorType::Background);
    //        auto foreground = settings.GetColorValue(winrt::Windows::UI::ViewManagement::UIColorType::Foreground);
    //    }


    //}

    // Linear interpolation function

    COLORREF InterpolateColorRef(COLORREF color1, COLORREF color2, float t);
    float GetColorRefInterpolateStepValue(COLORREF color1, COLORREF color2, COLORREF ref_color);

    enum class PencilModeChangeNotifyTimerMessage :WPARAM {
        HIDE,
        FADEOUT_HIDE,
        SHOW,
        FADE_ANIMATION

    };

    enum class PencilModeNotifyWindowStatus : int {
    
        PENCIL,
        ERASER
    };

    class PencilModeChangeNotifyWindow {
    public:
        PencilModeChangeNotifyWindow(HINSTANCE hInstance);
        ~PencilModeChangeNotifyWindow();


        int Create(LPCWSTR windowTitle, LPCWSTR text, int width, int height);

        bool AlterTransparency(float val);

        

        void Show();

        void FadeInShow(bool animateForceRestart);

        void FadeOutHide(bool animateForceRestart);

        void Hide();


        void SetAutoHideTimer(UINT milliseconds);

        void SetAutoFadeOutHideTimer(UINT milliseconds);

        void KillAutoHideTimer();

        void KillFadeAnimationTimer();

        void Destroy() {
            DestroyWindow(_hWnd);
            KillFadeAnimationTimer();
            KillAutoHideTimer();
        };

        void CallRedrawWindow() {
            if (use_WS_EX_LAYERED) {
                OnPaint();
            }
            else {
                InvalidateRect(_hWnd, NULL, TRUE);
            }
        }

        static int InitIconData(HINSTANCE hInstance) {
            ReleaseIconData();
            _penModeIcon = (HICON)LoadImage(hInstance,

                MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, 256, 256, LR_DEFAULTCOLOR);
            _eraserModeIcon = (HICON)LoadImage(hInstance,
                MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 256, 256, LR_DEFAULTCOLOR);
            return 0;
        }

        static int ReleaseIconData() {
            if (_penModeIcon)
                DestroyIcon(_penModeIcon);
            if (_eraserModeIcon)
                DestroyIcon(_eraserModeIcon);
            return 0;
        }

        //int SetIconPtrDisplay(HICON *iconPtr) {
        //    _displayIconPtr = iconPtr;
        //    return 0;
        //}

        int SetDisplayNotifyString(std::wstring const notiText){
            _infoText = notiText;
        }


        static HICON GetPenModeIconPtr()  {
            return _penModeIcon;
        }

        static HICON GetEraserModeIconPtr() {
            return _eraserModeIcon;
        }


        float GetWindowAlphaValue() const {
            return _blendFunction.SourceConstantAlpha / static_cast<float>(0xFF);
        }

        bool UpdateThemeData(bool auto_call_redraw = true) {
            
            winrt::Windows::UI::ViewManagement::UISettings settings;
            
            auto fgColor = settings.GetColorValue(winrt::Windows::UI::ViewManagement::UIColorType::Foreground);

            //if (fgColor == winrt::Windows::UI::Colors::White()) {
            //    bgColor = settings.GetColorValue(winrt::Windows::UI::ViewManagement::UIColorType::AccentDark3);
            //}
            //else if (fgColor == winrt::Windows::UI::Colors::Black()) {
            //    bgColor = settings.GetColorValue(winrt::Windows::UI::ViewManagement::UIColorType::AccentLight3);
            //}
            auto bgColor = settings.GetColorValue((_useDarkMode)? winrt::Windows::UI::ViewManagement::UIColorType::AccentDark3 : winrt::Windows::UI::ViewManagement::UIColorType::AccentLight3);

            sysBGColor = RGB(bgColor.R, bgColor.G, bgColor.B);
            
            
            sysFGColor = RGB(fgColor.R, fgColor.G, fgColor.B);

            if (true) {
                //wprintf_s(L"%s System BG Color : %x, System FG Color: %x\n",  GetTimeStampStr().c_str(), sysBGColor, sysFGColor);
                char color_display_str[60] = "";

                sprintf_s(color_display_str, sizeof(color_display_str), "System BG Color : %06X, System FG Color: %06X", sysBGColor, sysFGColor);
                std::cout << GetTimeStampStr() << color_display_str << std::endl;
            }

            if (auto_call_redraw) {
                ReInitCanvasObjects();
            }
            return true;
        }

        bool UpdateDPIValue(bool auto_resize_window=true, bool auto_call_redraw =true) {
            UINT32 newDPI = GetDpiForWindow(
                _hWnd
            );
            if (newDPI!= _DPI_Value) {
                _DPI_Value = newDPI;
                if (auto_resize_window) {

                    AutoResizeWindow();
                    ModifyRoundedCorner();


                }

                if (auto_call_redraw) {
                    ReInitCanvasObjects(true);
                }

                return true;
            }
            return false;
        }
        bool SetWindowLogicSize(SIZE newSize) {
            
            if (newSize.cx > 0 && newSize.cy > 0 &&
                (newSize.cx != wndLogicSize.cx || newSize.cy != wndLogicSize.cy)){
                wndLogicSize.cx = newSize.cx;
                wndLogicSize.cy = newSize.cy;
                return true;
            }
            return false;
        }
        bool AutoResizeWindow() {
            SIZE screenRealSize = GetRealScreenSize();
            SIZE realWindowSize = GetRealWindowSize();


            POINT screenPos = { (screenRealSize.cx - realWindowSize.cx) / 2 ,
            (screenRealSize.cy - realWindowSize.cy) / 2 };
            return UpdateLayeredWindow(_hWnd, NULL,
                &screenPos ,
                &realWindowSize, NULL,
                NULL, _render_BG_color,
                &_blendFunction, ULW_ALPHA);
        }

        int ModifyRoundedCorner() {
            SIZE screenRealSize = GetRealScreenSize();
            SIZE realWindowSize = GetRealWindowSize();
            // Create a rounded-rectangle region for the window shape
            HRGN hRgn = CreateRoundRectRgn(0, 0,
                realWindowSize.cx, realWindowSize.cy,
                30 * GetDPIScale(), 30 * GetDPIScale()); // Adjust the radius as needed
            SetWindowRgn(_hWnd, hRgn, TRUE);
            DeleteObject(hRgn);
            return 0;
        }

        float GetDPIScale() const {

            float val = static_cast<float>(_DPI_Value) / static_cast<float>(USER_DEFAULT_SCREEN_DPI);
            return val;
        }

        SIZE GetRealWindowSize() {
            SIZE rSize = { wndLogicSize.cx *GetDPIScale(), wndLogicSize.cy*GetDPIScale()};
            return rSize;
        }

        SIZE GetRealScreenSize() {
            SIZE sSize = { GetSystemMetrics(SM_CXSCREEN) ,
                GetSystemMetrics(SM_CYSCREEN) };
            return sSize;
        }

        bool SwitchPencilWindowStatus(PencilModeNotifyWindowStatus newStatus) {
            if (newStatus != _PMNWS) {
                _PMNWS = newStatus;
                switch (_PMNWS) {
                case PencilModeNotifyWindowStatus::PENCIL:
                {
                    if (_hbm_PencilNotification != nullptr)
                        _currentHbmMem = _hbm_PencilNotification;
                }break;
                case PencilModeNotifyWindowStatus::ERASER:
                {
                    if (_hbm_EraserNotification != nullptr)
                        _currentHbmMem = _hbm_EraserNotification;
                }break;
                default:
                    break;
                }


                return true;
            }
            return false;
        }


        PencilModeNotifyWindowStatus GetPencilModeNotifyWindowStatus() const {
            return _PMNWS;
        }


        bool SetWindowDarkMode(bool val, bool auto_call_theme_update = false) {
            if (val != _useDarkMode) {
                //OutputDebugString(L"Theme changed to.\n");
                std::cout << GetTimeStampStr() << "Theme changed to " << ((val) ? "Dark" :"Light") << " mode." << std::endl;
                _useDarkMode = val;
                if (auto_call_theme_update) {
                    UpdateThemeData(true);
                }
                return true;
            }

            return false;
        
        }

        bool SyncWithSystemDarkMode(bool auto_call_theme_update = true) {
            winrt::Windows::UI::ViewManagement::UISettings settings;
            auto bgColor = settings.GetColorValue(winrt::Windows::UI::ViewManagement::UIColorType::Background); 

            bool new_bm_val = (bgColor == winrt::Windows::UI::Colors::Black());
            return SetWindowDarkMode(new_bm_val, auto_call_theme_update);
        }

        bool GetIsInDarkMode() const { return _useDarkMode; }

    protected:
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static HICON _penModeIcon, _eraserModeIcon;
        //static std::vector<WNDPROC> _subWndProcHandlers;

        const static UINT32 _fadeAnimationIntervalMilliseconds = 20;

        PencilModeNotifyWindowStatus _PMNWS = PencilModeNotifyWindowStatus::PENCIL;

        COLORREF sysFGColor, sysBGColor;

        //HICON* _displayIconPtr = nullptr;


        void OnPaint();


        HRESULT ApplyBlurEffect(HWND hwnd);

        HDC _hdcMem = nullptr; 
        HBITMAP _currentHbmMem =nullptr; 
        HBITMAP _hbmOld = nullptr;

        HBITMAP _hbm_PencilNotification = nullptr, _hbm_EraserNotification = nullptr;

        bool _useDarkMode = false;

        bool ReInitCanvasObjects(bool call_redraw = true);

        bool RedrawWindowCanvas();

        void ReleaseCanvasObjects() const;

        void DrawBitMapCanvas(HBITMAP hBitmap, HICON displayIcon, std::wstring displayString = L"");

        HRESULT ApplySystemBackdrop(HWND const& hwnd, DWM_SYSTEMBACKDROP_TYPE const& bd_type);
        int _fadeAnimationState = 0;

        UINT8 _wndBaseAlpha = 200;
        SIZE wndLogicSize = {150,150};
        UINT32 
            _DPI_Value = USER_DEFAULT_SCREEN_DPI/2;

        UINT32 _fadeAnimationDurationMilliseconds = 200;
        float _fadeStepValue = 1.0/((float)_fadeAnimationDurationMilliseconds / (float)_fadeAnimationIntervalMilliseconds);

        bool SetFadeIntervalMilliSeconds(UINT32 val) {
            _fadeAnimationDurationMilliseconds = val;
            _fadeStepValue = 1.0 / ((float)_fadeAnimationDurationMilliseconds / (float)_fadeAnimationIntervalMilliseconds);
            return true;
        };

        bool use_WS_EX_LAYERED = false;
        HINSTANCE _hInstance;
        HWND _hWnd;
        UINT_PTR _hideTimerId = NULL,_fadeAnimationTimerId = NULL;

        BLENDFUNCTION _blendFunction = { 
            AC_SRC_OVER , 
            0, 
            0xFF, 
            AC_SRC_ALPHA
        };
        COLORREF _render_BG_color = RGB(0, 0, 0);

        //bool SetBgColor(COLORREF newBgColor) {
        //    canvas_bg_color = newBgColor;
        //    return true;
        //};

        std::wstring _infoText;

#ifdef TestCodeBlock1
        ID2D1Factory* pD2DFactory_;
        IDCompositionDevice* pDCompDevice_;
        IDCompositionTarget* pDCompTarget_;
         IDCompositionVisual* pDCompVisual_;
#endif
        static const wchar_t CLASS_NAME[];
    };

    //class GDIPlusBlurBackgroundWindow : public PencilModeChangeNotifyWindow {
    //protected:
    //    Gdiplus::GdiplusStartupInput _gdiplusStartupInput;
    //    //Gdiplus::GdiplusToken _gdiplusToken;

    //};

}