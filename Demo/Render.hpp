#include <Windows.h>


namespace ImGuiDx11Render
{
	typedef struct _RGBA
	{
		FLOAT R;
		FLOAT G;
		FLOAT B;
		FLOAT A;

	}RGBA, * PRGBA;

	typedef BOOL(WINAPI *pLoadRender)(LPCWSTR WinName, LPCWSTR ClsName, PVOID Callback, UINT UsePresent, FLOAT ThickNess, PHANDLE hWnd, PHANDLE hThread);
	typedef BOOL(WINAPI* pUnLoadRender)();
	typedef VOID(WINAPI* pMessageLoop)();
	typedef VOID(WINAPI* pSetSwitch)();
	typedef FLOAT(WINAPI* pGetFPS)();
	typedef VOID(WINAPI* pDrawRect)(FLOAT X, FLOAT Y, FLOAT W, FLOAT H, PRGBA Color, FLOAT ThickNess);
	typedef VOID(WINAPI* pDrawLine)(FLOAT X1, FLOAT Y1, FLOAT X2, FLOAT Y2, PRGBA Color, FLOAT ThickNess);
	typedef VOID(WINAPI* pDrawCircle)(FLOAT X, FLOAT Y, FLOAT R, PRGBA Color, FLOAT Segments, FLOAT ThickNess);
	typedef VOID(WINAPI* pDrawString)(FLOAT X, FLOAT Y, PRGBA Color, LPCSTR Str);
	typedef VOID(WINAPI* pDrawFilledRect)(FLOAT X, FLOAT Y, FLOAT W, FLOAT H, PRGBA Color);

	class Render
	{
	public:
		pLoadRender LoadRender;
		pUnLoadRender UnLoadRender;
		pMessageLoop MessageLoop;
		pSetSwitch SetSwitch;
		pGetFPS GetFPS;
		pDrawRect DrawRect;
		pDrawLine DrawLine;
		pDrawCircle DrawCircle;
		pDrawString DrawString;
		pDrawFilledRect DrawFilledRect;
	public:
		BOOL Init(HMODULE Module)
		{
			LoadRender = (pLoadRender)GetProcAddress(Module, "LoadRender");
			UnLoadRender = (pUnLoadRender)GetProcAddress(Module, "UnLoadRender");
			MessageLoop = (pMessageLoop)GetProcAddress(Module, "MessageLoop");
			SetSwitch = (pSetSwitch)GetProcAddress(Module, "SetSwitch");
			GetFPS = (pGetFPS)GetProcAddress(Module, "GetFPS");
			DrawRect = (pDrawRect)GetProcAddress(Module, "DrawRect");
			DrawLine = (pDrawLine)GetProcAddress(Module, "DrawLine");
			DrawCircle = (pDrawCircle)GetProcAddress(Module, "DrawCircle");
			DrawString = (pDrawString)GetProcAddress(Module, "DrawString");
			DrawFilledRect = (pDrawFilledRect)GetProcAddress(Module, "DrawFilledRect");
			return TRUE;
		}
	};



	





}