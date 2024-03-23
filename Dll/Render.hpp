#include "Helper/imgui.h"
#include "Helper/imgui_impl_dx11.h"
#include "Helper/imgui_impl_win32.h"

#include "Font.h"

#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <stdio.h>



#pragma comment(lib, "d3d11.lib")

typedef VOID(*ImGuiRenderCallback)(VOID* UserData);
static HWND Hwnd = NULL;                                                 // Window handle
static ID3D11Device* Device = NULL;                                      // Direct3D device
static ID3D11DeviceContext* Context = NULL;                              // Direct3D device context
static IDXGISwapChain* SwapChain = NULL;                                 // Direct3D swap chain
static ID3D11RenderTargetView* RenderTargetView = NULL;                  // Direct3D render target view
static ImGuiRenderCallback RenderCallBack = NULL;                        // Render callback
static UINT UsePresent = 0;                                              // Use present
static ImDrawList* DrawList = NULL;                                      // Draw list
static BOOL Run = TRUE;                                                  // Run
static BOOL IsExit = FALSE;                                              // Is exit

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
/*
* @brief 渲染主函数
*/
static VOID RenderMain()
{
	while (!IsExit)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		DrawList = ImGui::GetBackgroundDrawList();
		if (Run)
		{
			if (RenderCallBack)
				RenderCallBack(NULL);
		}
		ImGui::Render();
		Context->OMSetRenderTargets(1, &RenderTargetView, NULL);
		FLOAT Alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		Context->ClearRenderTargetView(RenderTargetView, Alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		SwapChain->Present(UsePresent, 0);
	}
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (RenderTargetView)
		RenderTargetView->Release();
	if (SwapChain)
		SwapChain->Release();
	if (Context)
		Context->Release();
	if (Device)
		Device->Release();
	if (Hwnd)
		DestroyWindow(Hwnd);


}
/*
* @brief 窗口过程
* @param Hwnd 窗口句柄
* @param Message 消息
* @param wParam 参数
* @param lParam 参数
* @return 返回值
*/
static LRESULT WINAPI WndProc(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(Hwnd, Message, wParam, lParam))
		return TRUE;
	switch (Message)
	{
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(Hwnd, Message, wParam, lParam);
}

class ImGuiDx11Render
{
private:
	WNDCLASSEXW WndClass = { 0 };                                     // Window class
	CHAR Buffer[1024] = { 0 };                                        // Buffer
public:

	/*
	* @brief 创建窗口
	* @return 窗口句柄
	*/
	HWND Create(LPCWSTR WinName, LPCWSTR ClsName)
	{
		this->WndClass.cbSize = sizeof(WNDCLASSEXW);
		this->WndClass.style = CS_CLASSDC;
		this->WndClass.lpfnWndProc = WndProc;
		this->WndClass.cbClsExtra = 0;
		this->WndClass.cbWndExtra = 0;
		this->WndClass.hInstance = GetModuleHandle(NULL);
		this->WndClass.hIcon = NULL;
		this->WndClass.hCursor = NULL;
		this->WndClass.hbrBackground = NULL;
		this->WndClass.lpszMenuName = NULL;
		this->WndClass.lpszClassName = ClsName;
		this->WndClass.hIconSm = NULL;
		RegisterClassExW(&this->WndClass);
		Hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_NOACTIVATE, this->WndClass.lpszClassName, WinName, WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP, 0, 0, GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN), NULL, NULL, this->WndClass.hInstance, NULL);
		if (Hwnd)
		{
			ShowWindow(Hwnd, SW_SHOWDEFAULT);
			UpdateWindow(Hwnd);
			SetWindowLongW(Hwnd, GWL_EXSTYLE, GetWindowLongW(Hwnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED);
			DwmExtendFrameIntoClientArea(Hwnd, new MARGINS{ -1 });
			SetLayeredWindowAttributes(Hwnd, 0, 255, LWA_ALPHA);
			//SetWindowDisplayAffinity(Hwnd, WDA_EXCLUDEFROMCAPTURE);
		}
		else
			UnregisterClassW(WndClass.lpszClassName, WndClass.hInstance);
		return Hwnd;
	}
	/*
	* @brief 加载ImGui
	* @param CallBack 回调函数
	* @param Present 是否使用垂直同步 0: 不使用 1: 使用
	* @param ThickNess 线条粗细
	* @return 绘制线程句柄
	*/
	HANDLE Init(ImGuiRenderCallback CallBack, UINT Present, FLOAT ThickNess)
	{
		DXGI_SWAP_CHAIN_DESC SwapChainDesc = { 0 };
		SwapChainDesc.BufferCount = 2;
		SwapChainDesc.BufferDesc.Width = 0;
		SwapChainDesc.BufferDesc.Height = 0;
		SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.OutputWindow = Hwnd;
		SwapChainDesc.SampleDesc.Count = 1;
		SwapChainDesc.SampleDesc.Quality = 0;
		SwapChainDesc.Windowed = TRUE;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		UINT CreateFlags = 0;
		D3D_FEATURE_LEVEL Level = {};
		CONST D3D_FEATURE_LEVEL FeatureLevels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
		HRESULT Result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, CreateFlags, FeatureLevels, 2, D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &Device, &Level, &Context);
		if (Result == DXGI_ERROR_UNSUPPORTED)
			Result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, CreateFlags, FeatureLevels, 2, D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &Device, &Level, &Context);
		if (Result != S_OK)
			return 0;
		ID3D11Texture2D* BackBuffer = NULL;
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (VOID**)&BackBuffer);
		Device->CreateRenderTargetView(BackBuffer, NULL, &RenderTargetView);
		BackBuffer->Release();
		ImGui::CreateContext();
		ImGuiIO& Io = ImGui::GetIO();
		Io.IniFilename = NULL;
		Io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		Io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		//加载字体
		//如果没有加载字体，imgui将使用默认字体。您还可以加载多种字体，并使用ImGui:：PushFont（）/PopFont 选择它们
		//AddFontFromFileTTF（）将返回ImFont*，因此如果需要在多个字体中选择字体，则可以存储它。
		//如果无法加载文件，则函数将返回一个nullptr。请处理应用程序中的这些错误（例如，使用断言，或显示错误并退出）。
		//当调用ImFontAtlas:：Build（）/GetTexDataAsXXXXX（）时，字体将以给定的大小光栅化（w/过采样）并存储到纹理中，下面的ImGui_ImplXXXX_NewFrame将调用该纹理。
		//在您的imconfig文件中使用“#define IMGUI_ABLE_FREETYPE”可以使用FREETYPE进行更高质量的字体渲染。
		//有关更多说明和详细信息，请阅读“docs/FONTS.md”。
		//请记住，在C/C++中，如果要在字符串文字中包含反斜杠，则需要写一个双反斜杠
		//Io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Consolas.ttf", ThickNess, NULL, Io.Fonts->GetGlyphRangesChineseFull());   //加载本地字体
		Io.Fonts->AddFontFromMemoryTTF((void*)FontData, FontSize, ThickNess, nullptr, Io.Fonts->GetGlyphRangesChineseFull());    //加载内存字体
		ImGui_ImplWin32_Init(Hwnd);
		ImGui_ImplDX11_Init(Device, Context);
		RenderCallBack = CallBack;
		UsePresent = Present;
		return CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)RenderMain, NULL, NULL, NULL);
	}
	VOID SetRender()
	{
		Run = !Run;
	}
	BOOL Exit()
	{
		IsExit = TRUE;
		return TRUE;
	}
	/*
	* @brief 消息循环
	*/
	VOID MessageLoop()
	{
		MSG Msg = { 0 };
		while (GetMessageW(&Msg, NULL, 0U, 0U))
		{
			TranslateMessage(&Msg);
			DispatchMessageW(&Msg);
			Sleep(1);
		}
	
	}
	/*
	* @brief 获取FPS
	* @return FPS
	*/
	FLOAT GetFPS()
	{
		ImGuiIO& Io = ImGui::GetIO();
		return Io.Framerate;
	}
	/*
	* @brief 绘制矩形
	* @param Left 左上角X坐标
	* @param Top 左上角Y坐标
	* @param Width 宽度
	* @param Height 高度
	* @param Color 颜色
	* @param ThickNess 线条粗细
	*/
	VOID DrawRect(FLOAT Left, FLOAT Top, FLOAT Width, FLOAT Height, ImColor Color, FLOAT ThickNess)
	{
		DrawList->AddLine(ImVec2(Left, Top), ImVec2(Left + Width, Top), Color, ThickNess);
		DrawList->AddLine(ImVec2(Left + Width, Top), ImVec2(Left + Width, Top + Height), Color, ThickNess);
		DrawList->AddLine(ImVec2(Left + Width, Top + Height), ImVec2(Left, Top + Height), Color, ThickNess);
		DrawList->AddLine(ImVec2(Left, Top + Height), ImVec2(Left, Top), Color, ThickNess);
	}
	/*
	* @brief 绘制字符串
	* @param Left 左上角X坐标
	* @param Top 左上角Y坐标
	* @param Color 颜色
	* @param ThinkNess 线条粗细
	* @param _Format 格式化字符串
	*/
	VOID DrawString(FLOAT Left, FLOAT Top, ImColor Color, CHAR CONST* CONST _Format, ...)
	{
		va_list Args;
		va_start(Args, _Format);
		vsprintf_s(Buffer, _Format, Args);
		va_end(Args);
		DrawList->AddText(ImVec2(Left, Top), Color, Buffer);
	
	}
	/*
	* @brief 绘制线条
	* @param StartX 起始X坐标
	* @param StartY 起始Y坐标
	* @param EndX 结束X坐标
	* @param EndY 结束Y坐标
	* @param Color 颜色
	* @param ThickNess 线条粗细
	*/
	VOID DrawLine(FLOAT StartX, FLOAT StartY, FLOAT EndX, FLOAT EndY, ImColor Color, FLOAT ThickNess)
	{
		DrawList->AddLine(ImVec2(StartX, StartY), ImVec2(EndX, EndY), Color, ThickNess);
	}
	/*
	* @brief 绘制圆
	* @param X 圆心X坐标
	* @param Y 圆心Y坐标
	* @param Radius 半径
	* @param Color 颜色
	*/
	VOID DrawCircle(FLOAT X, FLOAT Y, FLOAT Radius, ImColor Color, INT Segments, FLOAT ThickNess)
	{
		DrawList->AddCircle(ImVec2(X, Y), Radius, Color, Segments, ThickNess);
	}
	/*
	* @brief 绘制实心矩形
	* @param Left 左上角X坐标
	* @param Top 左上角Y坐标
	* @param Width 宽度
	* @param Height 高度
	* @param Color 颜色
	*/
	VOID DrawFilledRect(FLOAT Left, FLOAT Top, FLOAT Width, FLOAT Height, ImColor Color)
	{
		DrawList->AddRectFilled(ImVec2(Left, Top), ImVec2(Left + Width, Top + Height), Color);
	}


};

namespace RenderColor
{
	ImColor Red = ImColor(255, 0, 0, 255);
	ImColor Green = ImColor(0, 255, 0, 255);
	ImColor Blue = ImColor(0, 0, 255, 255);
	ImColor White = ImColor(255, 255, 255, 255);
	ImColor Black = ImColor(0, 0, 0, 255);
	ImColor Yellow = ImColor(255, 255, 0, 255);
	ImColor Cyan = ImColor(0, 255, 255, 255);
	ImColor Magenta = ImColor(255, 0, 255, 255);
	ImColor Silver = ImColor(192, 192, 192, 255);
	ImColor Gray = ImColor(128, 128, 128, 255);
	ImColor Maroon = ImColor(128, 0, 0, 255);
	ImColor Olive = ImColor(128, 128, 0, 255);
	ImColor Purple = ImColor(128, 0, 128, 255);
	ImColor Teal = ImColor(0, 128, 128, 255);
	ImColor Navy = ImColor(0, 0, 128, 255);
}

inline ImGuiDx11Render Render;