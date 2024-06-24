# MatePadWindowPenServAgent
A tool to help you restore mate pen functionality on Windows.


## Reason
某为MatePadEGo 2023是一个极具性价比的WOA设备。但是其用户态的笔驱动经常为人保守诟病。 不光有白名单限制而且会产生大量的日志文件。
本程序在于尽可能映射双击操作使其尽可能还原Windows 原生设置。
本程序未在MATE E 上测试，但理论上可行。



## Note
### Usage
你需要保证笔服务文件存在
"C:\Program Files\Huawei\PCManager\components\accessories_center\accessories_app\AccessoryApp\Lib\Plugins\Depend\PenService.dll"
你需要停止所有原生的笔服务才能有效使用该应用程序，包括APP_Accessory 等。
推荐你顺便停止电脑管家运行。该程序自带HiView信息流不可关闭。
**Windows Widgets信息流都不好意思不给开关你怎么好意思的。**

**推荐使用Windows 计划任务运行**
![image](https://github.com/AzulEterno/MatePadWindowPenServAgent/assets/75287037/d19427df-0e30-446b-888c-6ac92319b00a)
如果使用其作为宿主，可能难以关闭该程序。

### Download

文件位于bin\\<ARCH>下
使用VS2022构建。MATE E 非GO设备下载X64架构，GO设备下载ARM64EC架构。


### Further Plan

本人精力有限，仅稍微实现了基本功能切换。如果有人能够逆向工程如何获得笔的状态欢迎告诉我。
可以编写ARM64EC UI界面以完全替代APP_Accessory.

### Reference

[Python Implementation](https://github.com/qwqVictor/HuaweiPenEraserService/blob/master/eraser_service.py)

[C++ Implementation](https://github.com/eiyooooo/MateBook-E-Pen)
