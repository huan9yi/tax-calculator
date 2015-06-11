#include <string>

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

// 字符转十六进制
std::wstring charToHex(wchar_t c)
{
	std::wstring result;
	char first, second;

	first = (c & 0xF0) / 16;
	first += first > 9 ? 'A' - 10 : '0';
	second = c & 0x0F;
	second += second > 9 ? 'A' - 10 : '0';

	result.append(1, first);
	result.append(1, second);

	return result;
}

// URL编码
std::wstring form_urlencode(const std::wstring& src)
{
	std::wstring result;
	std::wstring::const_iterator iter;

	for (iter = src.begin(); iter != src.end(); ++iter) {
		switch (*iter) {
		case ' ':
			result.append(1, '+');
			break;
		// alnum
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
		case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
		case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
		case 'V': case 'W': case 'X': case 'Y': case 'Z':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
		case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
		case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
		case 'v': case 'w': case 'x': case 'y': case 'z':
		case '0': case '1': case '2': case '3': case '4': case '5': case '6':
		case '7': case '8': case '9':
		// mark
		case '-': case '_': case '.': case '!': case '~': case '*': case '\'': case '(': case ')':
			result.append(1, *iter);
			break;
		// escape
		default:
			result.append(1, '%');
			result.append(charToHex(*iter));
			break;
		}
	}

	return result;
}

void Debug(const char *msg, HWND hMainWnd = NULL){
	MessageBox(hMainWnd, msg, "信息", MB_OK);
}

void Debug(int msg, HWND hMainWnd = NULL){
	Debug(std::to_string(msg).c_str(), hMainWnd);
}

void Debug(const wchar_t *msg, HWND hMainWnd = NULL){
	MessageBoxW(hMainWnd, msg, L"信息", MB_OK);
}

void ShowErrorCodeText()
{
	LPVOID lpMsgBuf;
	DWORD nErrorCode = GetLastError();

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (PTSTR)&lpMsgBuf, 0, NULL);
	if (lpMsgBuf){
		Debug((LPCSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
	else{
		Debug("Format failed");
		Debug(nErrorCode);
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