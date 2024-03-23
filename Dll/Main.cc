// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "Main.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}









EXTERN_C BOOL WINAPI LoadRender(LPCWSTR WinName, LPCWSTR ClsName, PVOID Callback, UINT UsePresent, FLOAT ThickNess, PHANDLE hWnd, PHANDLE hThread)
{
    HANDLE Win = Render.Create(WinName, ClsName);
    if (!Win)
        return NULL;
    HANDLE Thread = Render.Init((ImGuiRenderCallback)Callback, UsePresent, ThickNess);
    if (!Thread)
        return NULL;
    *hWnd = Win;
    *hThread = Thread;
    return TRUE;
}
EXTERN_C BOOL WINAPI UnLoadRender()
{
    return Render.Exit();
}
EXTERN_C VOID WINAPI MessageLoop()
{
    Render.MessageLoop();
}
EXTERN_C VOID WINAPI SetSwitch()
{
    Render.SetRender();
}
EXTERN_C FLOAT WINAPI GetFPS()
{
    return Render.GetFPS();
}
EXTERN_C VOID WINAPI DrawRect(FLOAT X, FLOAT Y, FLOAT W, FLOAT H, PRGBA Color, FLOAT ThickNess)
{
    ImColor ImC = ImColor(Color->R, Color->G, Color->B, Color->A);
    Render.DrawRect(X, Y, W, H, ImC, ThickNess);
}
EXTERN_C VOID WINAPI DrawLine(FLOAT X1, FLOAT Y1, FLOAT X2, FLOAT Y2, PRGBA Color, FLOAT ThickNess)
{
    ImColor ImC = ImColor(Color->R, Color->G, Color->B, Color->A);
    Render.DrawLine(X1, Y1, X2, Y2, ImC, ThickNess);
}
EXTERN_C VOID WINAPI DrawCircle(FLOAT X, FLOAT Y, FLOAT Radius, PRGBA Color, INT Segments, FLOAT ThickNess)
{
    ImColor ImC = ImColor(Color->R, Color->G, Color->B, Color->A);
    Render.DrawCircle(X, Y, Radius, ImC, Segments, ThickNess);
}
EXTERN_C VOID WINAPI DrawString(FLOAT X, FLOAT Y, PRGBA Color, LPCSTR Str)
{
    ImColor ImC = ImColor(Color->R, Color->G, Color->B, Color->A);
    Render.DrawString(X, Y, ImC, Str);
}
EXTERN_C VOID WINAPI DrawFilledRect(FLOAT X, FLOAT Y, FLOAT W, FLOAT H, PRGBA Color)
{
    ImColor ImC = ImColor(Color->R, Color->G, Color->B, Color->A);
    Render.DrawFilledRect(X, Y, W, H, ImC);
}

