
#include "Render.hpp"

using namespace ImGuiDx11Render;
Render CRender;

RGBA Red = { 1.0f, 0.0f, 0.0f, 1.0f };
RGBA White = { 1.0f, 1.0f, 1.0f, 1.0f };


VOID Main()
{
    CRender.DrawRect(100, 100, 100, 100, &Red, 1.0f);
    CRender.DrawRect(200, 200, 100, 100, &White, 1.0f);
}

int main()
{
    HMODULE Module = LoadLibraryA("Dll.dll");
    if (!CRender.Init(Module))
        return 1;
    HANDLE hWnd = NULL;
    HANDLE hThread = NULL;
    BOOL bRet = CRender.LoadRender(L"RenderTest", L"RenderTest", &Main, 0, 1.0f, &hWnd, &hThread);
    if (!bRet)
        return 2;
    CRender.MessageLoop();



}

