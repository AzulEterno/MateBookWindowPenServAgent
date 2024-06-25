#pragma once
#include "pch.h"


namespace PencilNotifyWindows {

    enum class PencilModeChangeNotifyTimerMessage :WPARAM {
        HIDE,
        FADEOUT_HIDE,
        SHOW,
        FADE_ANIMATION

    };
    class PencilModeChangeNotifyWindow {
    public:
        PencilModeChangeNotifyWindow(HINSTANCE hInstance);
        ~PencilModeChangeNotifyWindow();

        bool Create(LPCWSTR windowTitle, LPCWSTR text, int width, int height);

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

        int SetIconPtrDisplay(HICON *iconPtr) {
            _displayIconPtr = iconPtr;
            return 0;
        }

        int SetDisplayNotifyString(std::wstring const notiText){
            _infoText = notiText;
        }


        static HICON *GetPenModeIconPtr()  {
            return &_penModeIcon;
        }

        static HICON *GetEraserModeIconPtr() {
            return &_eraserModeIcon;
        }


        float GetWindowAlphaValue() const {
            return blendFunction.SourceConstantAlpha / static_cast<float>(0xFF);
        }

        bool UpdateDPIValue() {
            UINT32 newDPI = GetDpiForWindow(
                _hWnd
            );
            if (newDPI!= _DPI_Value) {
                _DPI_Value = newDPI;
                AutoResizeWindow();

                SIZE screenRealSize = GetRealScreenSize();
                SIZE realWindowSize = GetRealWindowSize();
                // Create a rounded-rectangle region for the window shape
                HRGN hRgn = CreateRoundRectRgn(0, 0,
                    realWindowSize.cx, realWindowSize.cy,
                    30*GetDPIScale(), 30 * GetDPIScale()); // Adjust the radius as needed
                SetWindowRgn(_hWnd, hRgn, TRUE);
                DeleteObject(hRgn);
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
                NULL, RGB(255, 255, 255),
                &blendFunction, ULW_ALPHA);
        }

        float GetDPIScale() const {

            float val = static_cast<float>(_DPI_Value) / static_cast<float>(USER_DEFAULT_SCREEN_DPI);
            return val;
        }

        SIZE GetRealWindowSize() {
            SIZE rSize = { wndLogicSize.cx *GetDPIScale(), wndLogicSize.cy*GetDPIScale()};
            //std::wcout << "WndSizeX " << rSize.cx << ", WndSizeY " << rSize.cy << std::endl;
            return rSize;
            
        }

        SIZE GetRealScreenSize() {
            SIZE sSize = { GetSystemMetrics(SM_CXSCREEN) ,
                GetSystemMetrics(SM_CYSCREEN) };
            return sSize;
        }
    protected:
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static HICON _penModeIcon, _eraserModeIcon;

        const static UINT32 _fadeAnimationIntervalMilliseconds = 20;

        HICON* _displayIconPtr = nullptr;


        void OnPaint();


        HRESULT ApplyBlurEffect(HWND hwnd);

        HRESULT ApplySystemBackdrop(HWND const& hwnd, DWM_SYSTEMBACKDROP_TYPE const& bd_type);
        int _fadeAnimationState = 0;

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

        BLENDFUNCTION blendFunction = { AC_SRC_OVER , 
        0, 0xFF, AC_SRC_ALPHA };
        COLORREF bg_color = RGB(200,200,200);

        bool SetBgColor(COLORREF newBgColor) {
            bg_color = newBgColor;
            return true;
        };

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