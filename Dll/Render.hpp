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
* @brief ��Ⱦ������
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
* @brief ���ڹ���
* @param Hwnd ���ھ��
* @param Message ��Ϣ
* @param wParam ����
* @param lParam ����
* @return ����ֵ
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
	* @brief ��������
	* @return ���ھ��
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
	* @brief ����ImGui
	* @param CallBack �ص�����
	* @param Present �Ƿ�ʹ�ô�ֱͬ�� 0: ��ʹ�� 1: ʹ��
	* @param ThickNess ������ϸ
	* @return �����߳̾��
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
		//��������
		//���û�м������壬imgui��ʹ��Ĭ�����塣�������Լ��ض������壬��ʹ��ImGui:��PushFont����/PopFont ѡ������
		//AddFontFromFileTTF����������ImFont*����������Ҫ�ڶ��������ѡ�����壬����Դ洢����
		//����޷������ļ�������������һ��nullptr���봦��Ӧ�ó����е���Щ�������磬ʹ�ö��ԣ�����ʾ�����˳�����
		//������ImFontAtlas:��Build����/GetTexDataAsXXXXX����ʱ�����彫�Ը����Ĵ�С��դ����w/�����������洢�������У������ImGui_ImplXXXX_NewFrame�����ø�����
		//������imconfig�ļ���ʹ�á�#define IMGUI_ABLE_FREETYPE������ʹ��FREETYPE���и���������������Ⱦ��
		//�йظ���˵������ϸ��Ϣ�����Ķ���docs/FONTS.md����
		//���ס����C/C++�У����Ҫ���ַ��������а�����б�ܣ�����Ҫдһ��˫��б��
		//Io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Consolas.ttf", ThickNess, NULL, Io.Fonts->GetGlyphRangesChineseFull());   //���ر�������
		Io.Fonts->AddFontFromMemoryTTF((void*)FontData, FontSize, ThickNess, nullptr, Io.Fonts->GetGlyphRangesChineseFull());    //�����ڴ�����
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
	* @brief ��Ϣѭ��
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
	* @brief ��ȡFPS
	* @return FPS
	*/
	FLOAT GetFPS()
	{
		ImGuiIO& Io = ImGui::GetIO();
		return Io.Framerate;
	}
	/*
	* @brief ���ƾ���
	* @param Left ���Ͻ�X����
	* @param Top ���Ͻ�Y����
	* @param Width ���
	* @param Height �߶�
	* @param Color ��ɫ
	* @param ThickNess ������ϸ
	*/
	VOID DrawRect(FLOAT Left, FLOAT Top, FLOAT Width, FLOAT Height, ImColor Color, FLOAT ThickNess)
	{
		DrawList->AddLine(ImVec2(Left, Top), ImVec2(Left + Width, Top), Color, ThickNess);
		DrawList->AddLine(ImVec2(Left + Width, Top), ImVec2(Left + Width, Top + Height), Color, ThickNess);
		DrawList->AddLine(ImVec2(Left + Width, Top + Height), ImVec2(Left, Top + Height), Color, ThickNess);
		DrawList->AddLine(ImVec2(Left, Top + Height), ImVec2(Left, Top), Color, ThickNess);
	}
	/*
	* @brief �����ַ���
	* @param Left ���Ͻ�X����
	* @param Top ���Ͻ�Y����
	* @param Color ��ɫ
	* @param ThinkNess ������ϸ
	* @param _Format ��ʽ���ַ���
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
	* @brief ��������
	* @param StartX ��ʼX����
	* @param StartY ��ʼY����
	* @param EndX ����X����
	* @param EndY ����Y����
	* @param Color ��ɫ
	* @param ThickNess ������ϸ
	*/
	VOID DrawLine(FLOAT StartX, FLOAT StartY, FLOAT EndX, FLOAT EndY, ImColor Color, FLOAT ThickNess)
	{
		DrawList->AddLine(ImVec2(StartX, StartY), ImVec2(EndX, EndY), Color, ThickNess);
	}
	/*
	* @brief ����Բ
	* @param X Բ��X����
	* @param Y Բ��Y����
	* @param Radius �뾶
	* @param Color ��ɫ
	*/
	VOID DrawCircle(FLOAT X, FLOAT Y, FLOAT Radius, ImColor Color, INT Segments, FLOAT ThickNess)
	{
		DrawList->AddCircle(ImVec2(X, Y), Radius, Color, Segments, ThickNess);
	}
	/*
	* @brief ����ʵ�ľ���
	* @param Left ���Ͻ�X����
	* @param Top ���Ͻ�Y����
	* @param Width ���
	* @param Height �߶�
	* @param Color ��ɫ
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