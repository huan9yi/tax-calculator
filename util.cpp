#include <string>
#include <sstream>

#include <windows.h>

// 英文转换使用，中文会乱码，使用完要delete[]返回值
wchar_t* CharToWChar(const char *orig)
{
	size_t newsize = strlen(orig) + 1;
	wchar_t *wcstring = new wchar_t[newsize];

	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcstring, newsize, orig, _TRUNCATE);

	return wcstring;
}

// 中文UTF8转换版本，使用完要delete[]返回值
wchar_t* UTF8CharToWChar(const char *utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t *wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);

	return wstr;
}

// 英文转换使用，中文会乱码，使用完要delete[]返回值
char* WCharToChar(const wchar_t *orig)
{
	size_t origsize = wcslen(orig) + 1;
	const size_t newsize = origsize * 2;
	char *nstring = new char[newsize];

	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, nstring, newsize, orig, _TRUNCATE);

	return nstring;
}

// 使用完要delete[]返回值
char* UTF8_To_GB2312(const char *utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t *wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);

	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char *str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);

	if (wstr) delete[] wstr;

	return str;
}

void Debug(const char *msg, HWND hMainWnd = NULL){
	MessageBox(hMainWnd, msg, "信息", MB_OK);
}

void Debug(int msg, HWND hMainWnd = NULL){
	Debug(std::to_string(msg).c_str(), hMainWnd);
}

void DebugW(const wchar_t *msg, HWND hMainWnd = NULL){
	MessageBoxW(hMainWnd, msg, L"信息", MB_OK);
}

void GetErrorCodeText(DWORD nErrorCode)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (PTSTR)&lpMsgBuf, 0, NULL);
	if (lpMsgBuf){
		Debug((LPCSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
	else{
		Debug("Format failed");
	}
}

// 是否是64位的Windows系统
BOOL IsWow64()
{
	BOOL bIsWow64 = false;

	// IsWow64Process is not available on all supported versions of Windows.
	// Use GetModuleHandle to get a handle to the DLL that contains the function
	// and GetProcAddress to get a pointer to the function if available.

	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			// handle error
		}
	}

	return bIsWow64;
}

bool IsRunAfterBoot(){
	// 键名
	char *app_name = "xxx";

	// get path
	char path[MAX_PATH];
	HMODULE hModule = GetModuleHandle(NULL);
	GetModuleFileName(hModule, path, sizeof(path));

	HKEY hKey;
	char *subkey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS){
		// check the key
		char data[1024];
		DWORD size = 1024;
		if (RegGetValue(hKey, NULL, app_name, RRF_RT_REG_SZ, NULL, data, &size) == ERROR_SUCCESS){
			if (strcmp(data, path) == 0){
				RegCloseKey(hKey);
				return true;
			}
		}
	}

	RegCloseKey(hKey);
	return false;
}

void BootManagement(bool add){
	// 键名
	char *app_name = "xxx";

	// get path
	char path[MAX_PATH];
	HMODULE hModule = GetModuleHandle(NULL);
	GetModuleFileName(hModule, path, sizeof(path));

	HKEY hKey;
	bool exist = false;
	char *subkey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS){
		// check the key
		char data[1024];
		DWORD size = 1024;
		if (RegGetValue(hKey, NULL, app_name, RRF_RT_REG_SZ, NULL, data, &size) == ERROR_SUCCESS){
			if (strcmp(data, path) == 0){
				exist = true;
			}
		}

		// add
		if (add && !exist){
			RegSetValueEx(hKey, app_name, 0, REG_SZ, (const unsigned char*)path, sizeof(path));
		}
		// delete
		else if (!add && exist){
			RegDeleteValue(hKey, app_name);
		}
	}
	else{
		Debug("请以管理员身份运行");
	}

	RegCloseKey(hKey);
}