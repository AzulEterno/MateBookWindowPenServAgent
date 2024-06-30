#pragma once
#include "pch.h"

namespace DLLDynamicLoading {
	class DLL_Loader_Base
	{
	protected:
		using ErrorCallback = std::function<void(const std::string&)>;
		std::wstring _dllPath;
		HMODULE _dllModule;
		int _load_status_code = -1;
		ErrorCallback _errorCallback;

		void report_error(const std::string& message) const {
			if (_errorCallback) {
				_errorCallback(message);
			}
			else {
				std::cerr << GetTimeStampStr() << message << std::endl;
			}
		}
		void report_error(const std::wstring& wmessage) const {
			std::string message(wmessage.begin(), wmessage.end());
			report_error(message);

		}

		template<typename T>
		T LoadFunction(const char* functionName);
	public:

		int GetDllLoadStatus() const {
			return _load_status_code;
		};
		DLL_Loader_Base(const std::wstring& dllPath,
			 ErrorCallback errorCallback = nullptr,
			bool autoLoad = true) :_dllPath(dllPath), _dllModule(nullptr), _errorCallback(errorCallback) {
			if (autoLoad) {
				load_dll_module();
			}
		}
		~DLL_Loader_Base() {
			if (_dllModule) {
				FreeLibrary(_dllModule);
			}
		}

		int load_dll_module() {
			_dllModule = LoadLibrary(_dllPath.c_str());
			if (_dllModule == nullptr) {
				//std::wcerr << L"Failed to load DLL: " << _dllPath << std::endl;
				_load_status_code = GetLastError();
				std::string s_dllPath(_dllPath.begin(), _dllPath.end());
				report_error("Failed to load DLL: " + s_dllPath + " with error code: " + std::to_string(_load_status_code));
				return _load_status_code;
			}
			_load_status_code = 0;
			return 0;
		}

	};





	class HuaweiPencilServiceDLL : public DLL_Loader_Base {
	private:
		using CommandSendGetPenConnectStatusFuncType = __int64(*)();
		using CommandSendGetPenSerialNoFuncType = __int64(__cdecl*)();
		using RegisterCallBackUpdatePenSerialNoFuncType = __int64(*)(void*);
		using RegisterCallBackUpdatePenConnectStatusFuncType = __int64(*)(void*);
		using RegisterLogFuncType = void(*)(void(*)(const wchar_t*));
		using CommandSendPenCurrentFuncType = __int64(*)( UINT8);
		using CommandSendSetPenKeyFuncType = CommandSendPenCurrentFuncType;

		CommandSendGetPenConnectStatusFuncType _commandSendGetPenConnectStatus = nullptr;
		CommandSendGetPenSerialNoFuncType _commandSendGetPenSerialNo = nullptr;
		RegisterCallBackUpdatePenSerialNoFuncType _registerCallBackUpdatePenSerialNo = nullptr;
		RegisterCallBackUpdatePenConnectStatusFuncType _registerCallBackUpdatePenConnectStatus = nullptr;
		RegisterLogFuncType _registerLogFunc = nullptr;
		CommandSendPenCurrentFuncType _commandSendPenCurrentFunc = nullptr;
		CommandSendSetPenKeyFuncType _commandSendSetPenKeyFunc = nullptr;
		

		static HuaweiPencilServiceDLL* _instance;

		static void receiveAndRedirectLog(const wchar_t* wlog) {
			if (_instance) {
				std::wstring wlog_string(wlog);
				std::string log(wlog_string.begin(), wlog_string.end());
				_instance->report_error(log);
			}
		}

	public:
		static HuaweiPencilServiceDLL* GetSingletonInstancePtr(
			const std::wstring& dllPath = DefaultDLLPath
			, ErrorCallback errorCallback = nullptr
		) {
			if (_instance == nullptr) {
				_instance = new HuaweiPencilServiceDLL(dllPath,errorCallback);
			}
			return _instance;
		}

		static const std::wstring DefaultDLLPath;// = L"C:\\Program Files\\Huawei\\PCManager\\components\\accessories_center\\accessories_app\\AccessoryApp\\Lib\\Plugins\\Depend\\PenService.dll";
		HuaweiPencilServiceDLL(const std::wstring& dllPath = DefaultDLLPath
			, ErrorCallback errorCallback = nullptr)
			: DLL_Loader_Base(dllPath, errorCallback) {
			if (_dllModule) {
				load_functions();
			}
			//_instance = this;
		}

		~HuaweiPencilServiceDLL() {
			if (_instance == this) {

				_instance = nullptr;
			}
		}

		void load_functions();

		__int64 CommandSendGetPenConnectStatus() {
			if (_commandSendGetPenConnectStatus) {
				return _commandSendGetPenConnectStatus();
			}
			else {
				report_error("Function CommandSendGetPenConnectStatus is not loaded");
				return -1;
			}
		}

		__int64 CommandSendGetPenSerialNo() {
			if (_commandSendGetPenSerialNo) {
				return _commandSendGetPenSerialNo();
			}
			else {
				report_error("Function CommandSendGetPenSerialNo is not loaded");
				return -1;
			}
		}

		__int64 RegisterCallBackUpdatePenSerialNo(std::function<void(const wchar_t*)> callback) {
			if (_registerCallBackUpdatePenSerialNo) {
				void* callbackPtr = reinterpret_cast<void*>(&callback);
				return _registerCallBackUpdatePenSerialNo(callbackPtr);
			}
			else {
				report_error("Function RegisterCallBackUpdatePenSerialNo is not loaded");
				return -1;
			}
		}

		__int64 RegisterCallBackUpdatePenConnectStatus(std::function<void(int)> callback) {
			if (_registerCallBackUpdatePenConnectStatus) {
				void* callbackPtr = reinterpret_cast<void*>(&callback);
				return _registerCallBackUpdatePenConnectStatus(callbackPtr);
			}
			else {
				report_error("Function RegisterCallBackUpdatePenConnectStatus is not loaded");
				return -1;
			}
		}

		__int64 CommandSendPenCurrentFunc(UINT8 val) {
			if (_commandSendPenCurrentFunc) {
				return _commandSendPenCurrentFunc(val);
			}
			else {
				report_error("Function CommandSendPenCurrentFuncType is not loaded");
				return -1;
			}
		}

		__int64 CommandSendSetPenKeyFunc(UINT8 val) {
			if (_commandSendSetPenKeyFunc) {
				return _commandSendSetPenKeyFunc(val);
			}
			else {
				report_error("Function CommandSendSetPenKeyFunc is not loaded");
				return -1;
			}
		}
	};

	

}