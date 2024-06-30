#pragma once
#include "DLL_Loader.h"

namespace DLLDynamicLoading {
	static std::wstring ConvertToWString(const char* charStr) {
		if (charStr == nullptr) {
			return std::wstring();
		}
		size_t length = strlen(charStr);
		std::vector<wchar_t> buffer(length + 1);
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, buffer.data(), buffer.size(), charStr, _TRUNCATE);
		return std::wstring(buffer.data());
	}

	template<typename T>
	T DLL_Loader_Base::LoadFunction(const char* functionName) {
		if (_dllModule != nullptr) {
			T func = reinterpret_cast<T>(GetProcAddress(_dllModule, functionName));
			if (func == nullptr) {

				std::wstringstream ss;
				ss << L"Failed to load function: " << ConvertToWString(functionName) << std::endl;
				report_error(ss.str());
				_load_status_code = -2;
			}
			return func;
		}
		return nullptr;
	}

	HuaweiPencilServiceDLL* HuaweiPencilServiceDLL::_instance = nullptr;
	const std::wstring HuaweiPencilServiceDLL::DefaultDLLPath = L"C:\\Program Files\\Huawei\\PCManager\\components\\accessories_center\\accessories_app\\AccessoryApp\\Lib\\Plugins\\Depend\\PenService.dll";


	void HuaweiPencilServiceDLL::load_functions() {
		_commandSendGetPenConnectStatus = LoadFunction<CommandSendGetPenConnectStatusFuncType>("CommandSendGetPenConnectStatus");
		if (!_commandSendGetPenConnectStatus) {
			report_error("Failed to load function: CommandSendGetPenConnectStatus");
		}

		_commandSendGetPenSerialNo = LoadFunction<CommandSendGetPenSerialNoFuncType>("CommandSendGetPenSerialNo");
		if (!_commandSendGetPenSerialNo) {
			report_error("Failed to load function: CommandSendGetPenSerialNo");
		}

		_registerCallBackUpdatePenSerialNo = LoadFunction<RegisterCallBackUpdatePenSerialNoFuncType>("RegisterCallBackUpdatePenSerialNo");
		if (!_registerCallBackUpdatePenSerialNo) {
			report_error("Failed to load function: RegisterCallBackUpdatePenSerialNo");
		}

		_registerCallBackUpdatePenConnectStatus = LoadFunction<RegisterCallBackUpdatePenConnectStatusFuncType>("RegisterCallBackUpdatePenConnectStatus");
		if (!_registerCallBackUpdatePenConnectStatus) {
			report_error("Failed to load function: RegisterCallBackUpdatePenConnectStatus");
		}

		_registerLogFunc = LoadFunction<RegisterLogFuncType>("RegisterLogFunc");
		if (!_registerLogFunc) {
			report_error("Failed to load function: RegisterLogFunc");
		}
		else {
			_registerLogFunc(receiveAndRedirectLog);
		}

		_commandSendPenCurrentFunc = LoadFunction<CommandSendPenCurrentFuncType>("CommandSendPenCurrentFunc");

		if (!_commandSendPenCurrentFunc) {
			report_error("Failed to load function: CommandSendPenCurrentFunc");
		}

		_commandSendSetPenKeyFunc = LoadFunction<CommandSendSetPenKeyFuncType>("CommandSendSetPenKeyFunc");

		if (!_commandSendSetPenKeyFunc) {
			report_error("Failed to load function: CommandSendPenCurrentFunc");
		}
	}
}