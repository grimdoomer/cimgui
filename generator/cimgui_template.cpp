#ifdef IMGUI_ENABLE_FREETYPE
#ifndef CIMGUI_FREETYPE
#error "IMGUI_FREETYPE should be defined for Freetype linking"
#endif
#else
#ifdef CIMGUI_FREETYPE
#error "IMGUI_FREETYPE should not be defined without freetype generated cimgui"
#endif
#endif
#include "./imgui/imgui.h"
#ifdef IMGUI_ENABLE_FREETYPE
#include "./imgui/misc/freetype/imgui_freetype.h"
#endif
#include "./imgui/imgui_internal.h"
#include "cimgui.h"



#include "auto_funcs.cpp"


/////////////////////////////manual written functions
CIMGUI_API void igLogText(CONST char *fmt, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 256, fmt, args);
    va_end(args);

    ImGui::LogText("%s", buffer);
}
CIMGUI_API void ImGuiTextBuffer_appendf(struct ImGuiTextBuffer *buffer, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    buffer->appendfv(fmt, args);
    va_end(args);
}

CIMGUI_API float igGET_FLT_MAX()
{
    return FLT_MAX;
}

CIMGUI_API float igGET_FLT_MIN()
{
    return FLT_MIN;
}


CIMGUI_API ImVector_ImWchar* ImVector_ImWchar_create()
{
	return IM_NEW(ImVector<ImWchar>) ();
}

CIMGUI_API void ImVector_ImWchar_destroy(ImVector_ImWchar* self)
{
    IM_DELETE(self);
}

CIMGUI_API void ImVector_ImWchar_Init(ImVector_ImWchar* p)
{
	IM_PLACEMENT_NEW(p) ImVector<ImWchar>();
}
CIMGUI_API void ImVector_ImWchar_UnInit(ImVector_ImWchar* p)
{
	p->~ImVector<ImWchar>();
}


//// NOTE: Some function pointers in the ImGuiPlatformIO structure are not C-compatible because of their
//// use of a complex return type. To work around this, we store a custom CimguiStorage object inside
//// ImGuiIO::BackendLanguageUserData, which contains C-compatible function pointer variants for these
//// functions. When a user function pointer is provided, we hook up the underlying ImGuiPlatformIO
//// function pointer to a thunk which accesses the user function pointer through CimguiStorage.
//
//struct CimguiStorage
//{
//    void(*Platform_GetWindowPos)(ImGuiViewport* vp, ImVec2* out_pos);
//    void(*Platform_GetWindowSize)(ImGuiViewport* vp, ImVec2* out_pos);
//};
//
//// Gets a reference to the CimguiStorage object stored in the current ImGui context's BackendLanguageUserData.
//CimguiStorage& GetCimguiStorage()
//{
//    ImGuiIO& io = ImGui::GetIO();
//    if (io.BackendLanguageUserData == NULL)
//    {
//        io.BackendLanguageUserData = new CimguiStorage();
//    }
//
//    return *(CimguiStorage*)io.BackendLanguageUserData;
//}
//
//// Thunk satisfying the signature of ImGuiPlatformIO::Platform_GetWindowPos.
//ImVec2 Platform_GetWindowPos_hook(ImGuiViewport* vp)
//{
//    ImVec2 pos;
//    GetCimguiStorage().Platform_GetWindowPos(vp, &pos);
//    return pos;
//};
//
//// Fully C-compatible function pointer setter for ImGuiPlatformIO::Platform_GetWindowPos.
//CIMGUI_API void ImGuiPlatformIO_Set_Platform_GetWindowPos(ImGuiPlatformIO* platform_io, void(*user_callback)(ImGuiViewport* vp, ImVec2* out_pos))
//{
//    CimguiStorage& storage = GetCimguiStorage();
//    storage.Platform_GetWindowPos = user_callback;
//    platform_io->Platform_GetWindowPos = &Platform_GetWindowPos_hook;
//}
//
//// Thunk satisfying the signature of ImGuiPlatformIO::Platform_GetWindowSize.
//ImVec2 Platform_GetWindowSize_hook(ImGuiViewport* vp)
//{
//    ImVec2 size;
//    GetCimguiStorage().Platform_GetWindowSize(vp, &size);
//    return size;
//};
//
//// Fully C-compatible function pointer setter for ImGuiPlatformIO::Platform_GetWindowSize.
//CIMGUI_API void ImGuiPlatformIO_Set_Platform_GetWindowSize(ImGuiPlatformIO* platform_io, void(*user_callback)(ImGuiViewport* vp, ImVec2* out_size))
//{
//    CimguiStorage& storage = GetCimguiStorage();
//    storage.Platform_GetWindowSize = user_callback;
//    platform_io->Platform_GetWindowSize = &Platform_GetWindowSize_hook;
//}

CIMGUI_API void ImGuiPlatformIO_Monitors_Resize(int size)
{
    ImGui::GetPlatformIO().Monitors.resize(size);
}

CIMGUI_API void ImGuiPlatformIO_Monitors_PushBack(ImGuiPlatformMonitor monitor)
{
    ImGui::GetPlatformIO().Monitors.push_back(monitor);
}

CIMGUI_API void ImGuiPlatformIO_Monitors_PushFront(ImGuiPlatformMonitor monitor)
{
    ImGui::GetPlatformIO().Monitors.push_front(monitor);
}


// CImgui sets the default calling convention to __cdecl but this makes it hard to pass delegate functions from C# that can
// be called correctly. To work around this we create a PlatformIO structure that has __stdcall function pointers that match
// the calling convention of the C# delegates we assign. Then we tie this structure to the ImGui context struct (since the real ImGui
// PlatformIO structure is part of ImGuiContext) and when a C# delegate is assigned we hook in a __cdecl shim function that can
// handle the calling convention switch.

#include <map>

struct ImGuiPlatformIOStdCall
{
    // Platform function --------------------------------------------------- Called by -----
    void(__stdcall* Platform_CreateWindow)(ImGuiViewport* vp);                    // . . U . .  // Create a new platform window for the given viewport
    void(__stdcall* Platform_DestroyWindow)(ImGuiViewport* vp);                   // N . U . D  //
    void(__stdcall* Platform_ShowWindow)(ImGuiViewport* vp);                      // . . U . .  // Newly created windows are initially hidden so SetWindowPos/Size/Title can be called on them before showing the window
    void(__stdcall* Platform_SetWindowPos)(ImGuiViewport* vp, ImVec2 pos);        // . . U . .  // Set platform window position (given the upper-left corner of client area)
    void(__stdcall* Platform_GetWindowPos)(ImGuiViewport* vp, ImVec2* pos);                    // N . . . .  //
    void(__stdcall* Platform_SetWindowSize)(ImGuiViewport* vp, ImVec2 size);      // . . U . .  // Set platform window client area size (ignoring OS decorations such as OS title bar etc.)
    void(__stdcall* Platform_GetWindowSize)(ImGuiViewport* vp, ImVec2* size);                   // N . . . .  // Get platform window client area size
    void(__stdcall* Platform_SetWindowFocus)(ImGuiViewport* vp);                  // N . . . .  // Move window to front and set input focus
    bool(__stdcall* Platform_GetWindowFocus)(ImGuiViewport* vp);                  // . . U . .  //
    bool(__stdcall* Platform_GetWindowMinimized)(ImGuiViewport* vp);              // N . . . .  // Get platform window minimized state. When minimized, we generally won't attempt to get/set size and contents will be culled more easily
    void(__stdcall* Platform_SetWindowTitle)(ImGuiViewport* vp, const char* str); // . . U . .  // Set platform window title (given an UTF-8 string)
    void(__stdcall* Platform_SetWindowAlpha)(ImGuiViewport* vp, float alpha);     // . . U . .  // (Optional) Setup global transparency (not per-pixel transparency)
    void(__stdcall* Platform_UpdateWindow)(ImGuiViewport* vp);                    // . . U . .  // (Optional) Called by UpdatePlatformWindows(). Optional hook to allow the platform backend from doing general book-keeping every frame.
    void(__stdcall* Platform_RenderWindow)(ImGuiViewport* vp, void* render_arg);  // . . . R .  // (Optional) Main rendering (platform side! This is often unused, or just setting a "current" context for OpenGL bindings). 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    void(__stdcall* Platform_SwapBuffers)(ImGuiViewport* vp, void* render_arg);   // . . . R .  // (Optional) Call Present/SwapBuffers (platform side! This is often unused!). 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    float(__stdcall* Platform_GetWindowDpiScale)(ImGuiViewport* vp);               // N . . . .  // (Optional) [BETA] FIXME-DPI: DPI handling: Return DPI scale for this viewport. 1.0f = 96 DPI.
    void(__stdcall* Platform_OnChangedViewport)(ImGuiViewport* vp);               // . F . . .  // (Optional) [BETA] FIXME-DPI: DPI handling: Called during Begin() every time the viewport we are outputting into changes, so backend has a chance to swap fonts to adjust style.
    int(__stdcall* Platform_CreateVkSurface)(ImGuiViewport* vp, ImU64 vk_inst, const void* vk_allocators, ImU64* out_vk_surface); // (Optional) For a Vulkan Renderer to call into Platform code (since the surface creation needs to tie them both).

    // (Optional) Renderer functions (e.g. DirectX, OpenGL, Vulkan)
    void(__stdcall* Renderer_CreateWindow)(ImGuiViewport* vp);                    // . . U . .  // Create swap chain, frame buffers etc. (called after Platform_CreateWindow)
    void(__stdcall* Renderer_DestroyWindow)(ImGuiViewport* vp);                   // N . U . D  // Destroy swap chain, frame buffers etc. (called before Platform_DestroyWindow)
    void(__stdcall* Renderer_SetWindowSize)(ImGuiViewport* vp, ImVec2 size);      // . . U . .  // Resize swap chain, frame buffers etc. (called after Platform_SetWindowSize)
    void(__stdcall* Renderer_RenderWindow)(ImGuiViewport* vp, void* render_arg);  // . . . R .  // (Optional) Clear framebuffer, setup render target, then render the viewport->DrawData. 'render_arg' is the value passed to RenderPlatformWindowsDefault().
    void(__stdcall* Renderer_SwapBuffers)(ImGuiViewport* vp, void* render_arg);   // . . . R .  // (Optional) Call Present/SwapBuffers. 'render_arg' is the value passed to RenderPlatformWindowsDefault().

    ImGuiPlatformIOStdCall() { memset(this, 0, sizeof(*this)); }     // Zero clear
};

std::map<ImGuiContext*, ImGuiPlatformIOStdCall> imGuiManagedPlatformIO;


// Shim functions to handle going from __cdecl to __stdcall calling convention.

void Shim_Platform_CreateWindow(ImGuiViewport* vp)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_CreateWindow(vp);
}

void Shim_Platform_DestroyWindow(ImGuiViewport* vp)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_DestroyWindow(vp);
}

void Shim_Platform_ShowWindow(ImGuiViewport* vp)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_ShowWindow(vp);
}

void Shim_Platform_SetWindowPos(ImGuiViewport* vp, ImVec2 pos)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SetWindowPos(vp, pos);
}

ImVec2 Shim_Platform_GetWindowPos(ImGuiViewport* vp)
{
    ImVec2 pos;
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_GetWindowPos(vp, &pos);

    return pos;
}

void Shim_Platform_SetWindowSize(ImGuiViewport* vp, ImVec2 size)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SetWindowSize(vp, size);
}

ImVec2 Shim_Platform_GetWindowSize(ImGuiViewport* vp)
{
    ImVec2 size;
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_GetWindowSize(vp, &size);

    return size;
}

void Shim_Platform_SetWindowFocus(ImGuiViewport* vp)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SetWindowFocus(vp);
}

bool Shim_Platform_GetWindowFocus(ImGuiViewport* vp)
{
    return imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_GetWindowFocus(vp);
}

bool Shim_Platform_GetWindowMinimized(ImGuiViewport* vp)
{
    return imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_GetWindowMinimized(vp);
}

void Shim_Platform_SetWindowTitle(ImGuiViewport* vp, const char* str)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SetWindowTitle(vp, str);
}

void Shim_Platform_SetWindowAlpha(ImGuiViewport* vp, float alpha)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SetWindowAlpha(vp, alpha);
}

void Shim_Platform_UpdateWindow(ImGuiViewport* vp)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_UpdateWindow(vp);
}

void Shim_Platform_RenderWindow(ImGuiViewport* vp, void* render_arg)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_RenderWindow(vp, render_arg);
}

void Shim_Platform_SwapBuffers(ImGuiViewport* vp, void* render_arg)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SwapBuffers(vp, render_arg);
}

float Shim_Platform_GetWindowDpiScale(ImGuiViewport* vp)
{
    return imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_GetWindowDpiScale(vp);
}

void Shim_Platform_OnChangedViewport(ImGuiViewport* vp)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_OnChangedViewport(vp);
}

int Shim_Platform_CreateVkSurface(ImGuiViewport* vp, ImU64 vk_inst, const void* vk_allocators, ImU64* out_vk_surface)
{
    return imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_CreateVkSurface(vp, vk_inst, vk_allocators, out_vk_surface);
}

void Shim_Renderer_CreateWindow(ImGuiViewport* vp)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Renderer_CreateWindow(vp);
}

void Shim_Renderer_DestroyWindow(ImGuiViewport* vp)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Renderer_DestroyWindow(vp);
}

void Shim_Renderer_SetWindowSize(ImGuiViewport* vp, ImVec2 size)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Renderer_SetWindowSize(vp, size);
}

void Shim_Renderer_RenderWindow(ImGuiViewport* vp, void* render_arg)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Renderer_RenderWindow(vp, render_arg);
}

void Shim_Renderer_SwapBuffers(ImGuiViewport* vp, void* render_arg)
{
    imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Renderer_SwapBuffers(vp, render_arg);
}


CIMGUI_API void ImGuiPlatformIO_SetCallback(int callback, void* funcptr)
{
    // Make sure there is a platform io structure for the current imgui context.
    if (imGuiManagedPlatformIO.find(ImGui::GetCurrentContext()) == imGuiManagedPlatformIO.end())
        imGuiManagedPlatformIO.emplace(ImGui::GetCurrentContext(), ImGuiPlatformIOStdCall());

    // Check the callback type and handle accordingly.
    void** ppPlatformIOManaged = nullptr;
    switch (callback)
    {
    case 0:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_CreateWindow;
        ImGui::GetPlatformIO().Platform_CreateWindow = Shim_Platform_CreateWindow;
        break;
    }
    case 1:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_DestroyWindow;
        ImGui::GetPlatformIO().Platform_DestroyWindow = Shim_Platform_DestroyWindow;
        break;
    }
    case 2:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_ShowWindow;
        ImGui::GetPlatformIO().Platform_ShowWindow = Shim_Platform_ShowWindow;
        break;
    }
    case 3:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SetWindowPos;
        ImGui::GetPlatformIO().Platform_SetWindowPos = Shim_Platform_SetWindowPos;
        break;
    }
    case 4:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_GetWindowPos;
        ImGui::GetPlatformIO().Platform_GetWindowPos = Shim_Platform_GetWindowPos;
        break;
    }
    case 5:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SetWindowSize;
        ImGui::GetPlatformIO().Platform_SetWindowSize = Shim_Platform_SetWindowSize;
        break;
    }
    case 6:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_GetWindowSize;
        ImGui::GetPlatformIO().Platform_GetWindowSize = Shim_Platform_GetWindowSize;
        break;
    }
    case 7:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SetWindowFocus;
        ImGui::GetPlatformIO().Platform_SetWindowFocus = Shim_Platform_SetWindowFocus;
        break;
    }
    case 8:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_GetWindowFocus;
        ImGui::GetPlatformIO().Platform_GetWindowFocus = Shim_Platform_GetWindowFocus;
        break;
    }
    case 9:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_GetWindowMinimized;
        ImGui::GetPlatformIO().Platform_GetWindowMinimized = Shim_Platform_GetWindowMinimized;
        break;
    }
    case 10:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SetWindowTitle;
        ImGui::GetPlatformIO().Platform_SetWindowTitle = Shim_Platform_SetWindowTitle;
        break;
    }
    case 11:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SetWindowAlpha;
        ImGui::GetPlatformIO().Platform_SetWindowAlpha = Shim_Platform_SetWindowAlpha;
        break;
    }
    case 12:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_UpdateWindow;
        ImGui::GetPlatformIO().Platform_UpdateWindow = Shim_Platform_UpdateWindow;
        break;
    }
    case 13:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_RenderWindow;
        ImGui::GetPlatformIO().Platform_RenderWindow = Shim_Platform_RenderWindow;
        break;
    }
    case 14:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_SwapBuffers;
        ImGui::GetPlatformIO().Platform_SwapBuffers = Shim_Platform_SwapBuffers;
        break;
    }
    case 15:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_GetWindowDpiScale;
        ImGui::GetPlatformIO().Platform_GetWindowDpiScale = Shim_Platform_GetWindowDpiScale;
        break;
    }
    case 16:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_OnChangedViewport;
        ImGui::GetPlatformIO().Platform_OnChangedViewport = Shim_Platform_OnChangedViewport;
        break;
    }
    case 17:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Platform_CreateVkSurface;
        ImGui::GetPlatformIO().Platform_CreateVkSurface = Shim_Platform_CreateVkSurface;
        break;
    }
    case 18:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Renderer_CreateWindow;
        ImGui::GetPlatformIO().Renderer_CreateWindow = Shim_Renderer_CreateWindow;
        break;
    }
    case 19:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Renderer_DestroyWindow;
        ImGui::GetPlatformIO().Renderer_DestroyWindow = Shim_Renderer_DestroyWindow;
        break;
    }
    case 20:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Renderer_SetWindowSize;
        ImGui::GetPlatformIO().Renderer_SetWindowSize = Shim_Renderer_SetWindowSize;
        break;
    }
    case 21:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Renderer_RenderWindow;
        ImGui::GetPlatformIO().Renderer_RenderWindow = Shim_Renderer_RenderWindow;
        break;
    }
    case 22:
    {
        ppPlatformIOManaged = (void**)&imGuiManagedPlatformIO[ImGui::GetCurrentContext()].Renderer_SwapBuffers;
        ImGui::GetPlatformIO().Renderer_SwapBuffers = Shim_Renderer_SwapBuffers;
        break;
    }
    }

    // Set the managed function ptr.
    *ppPlatformIOManaged = funcptr;
}