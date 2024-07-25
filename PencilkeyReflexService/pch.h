#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <functional>
#include <iomanip>
#include <ctime>
#include <stdexcept>
#include <fstream>
#include <chrono>
#include <windows.h>
#include <dwmapi.h>
#include <winuser.h>
#include <powrprof.h>
#include <algorithm> 

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "PowrProf.lib")

#include "resource.h"

#include "utilities.h"
#include <winrt/Windows.UI.ViewManagement.h>
// Define a hotkey identifier
constexpr auto PenButtonClick = 1;
constexpr auto PenButtonDoubleClick = 2;


constexpr auto TestHotKeyFunction = 16;

