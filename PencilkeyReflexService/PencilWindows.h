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
    protected:
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static HICON _penModeIcon, _eraserModeIcon;

        const static UINT32 _fadeAnimationIntervalMilliseconds = 20;

        HICON* _displayIconPtr = nullptr;


        void OnPaint();


        HRESULT ApplyBlurEffect(HWND hwnd);

        HRESULT ApplySystemBackdrop(HWND const& hwnd, DWM_SYSTEMBACKDROP_TYPE const& bd_type);
        
        int _width = 0, _height = 0, _fadeAnimationState = 0;

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

    class GDIPlusBlurBackgroundWindow : public PencilModeChangeNotifyWindow {
    protected:
        Gdiplus::GdiplusStartupInput _gdiplusStartupInput;
        //Gdiplus::GdiplusToken _gdiplusToken;

    };

}