// gtasan.cpp — Fixed Full Version
// Fix 1: proxy_ prefix + /export pragma → redefinition حل
// Fix 2: g_pfnCreate9 به جای Direct3DCreate9 در HookPresent → loop حل
// Fix 3: mmsystem.h اضافه شد → timeBeginPeriod حل

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>          // timeBeginPeriod / timeEndPeriod
#include <d3d9.h>
#include <cstdint>
#include <cstdio>
#include <cstring>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")

// ─── /export: اسم اصلی d3d9 → تابع proxy_ ما ───────────────────────────────
#pragma comment(linker, "/export:Direct3DCreate9=proxy_Direct3DCreate9")
#pragma comment(linker, "/export:Direct3DCreate9Ex=proxy_Direct3DCreate9Ex")
#pragma comment(linker, "/export:Direct3DShaderValidatorCreate9=proxy_D3DShaderValidator")
#pragma comment(linker, "/export:D3DPERF_BeginEvent=proxy_D3DPERF_BeginEvent")
#pragma comment(linker, "/export:D3DPERF_EndEvent=proxy_D3DPERF_EndEvent")
#pragma comment(linker, "/export:D3DPERF_GetStatus=proxy_D3DPERF_GetStatus")
#pragma comment(linker, "/export:D3DPERF_QueryRepeatFrame=proxy_D3DPERF_QueryRepeatFrame")
#pragma comment(linker, "/export:D3DPERF_SetMarker=proxy_D3DPERF_SetMarker")
#pragma comment(linker, "/export:D3DPERF_SetOptions=proxy_D3DPERF_SetOptions")
#pragma comment(linker, "/export:D3DPERF_SetRegion=proxy_D3DPERF_SetRegion")
#pragma comment(linker, "/export:DebugSetLevel=proxy_DebugSetLevel")
#pragma comment(linker, "/export:DebugSetMute=proxy_DebugSetMute")


// ═════════════════════════════════════════════════════════════════════════════
//  PART 0 — REAL d3d9 LOADER
// ═════════════════════════════════════════════════════════════════════════════

typedef IDirect3D9* (WINAPI* PFN_D3DCreate9)(UINT);
typedef HRESULT     (WINAPI* PFN_D3DCreate9Ex)(UINT, IDirect3D9Ex**);
typedef void*       (WINAPI* PFN_D3DShaderValidator)();
typedef int         (WINAPI* PFN_PerfBeginEvent)(D3DCOLOR, LPCWSTR);
typedef int         (WINAPI* PFN_PerfEndEvent)();
typedef DWORD       (WINAPI* PFN_PerfGetStatus)();
typedef BOOL        (WINAPI* PFN_PerfQueryRepeat)();
typedef void        (WINAPI* PFN_PerfSetMarker)(D3DCOLOR, LPCWSTR);
typedef void        (WINAPI* PFN_PerfSetOptions)(DWORD);
typedef void        (WINAPI* PFN_PerfSetRegion)(D3DCOLOR, LPCWSTR);
typedef HRESULT     (WINAPI* PFN_DebugSetLevel)(DWORD);
typedef void        (WINAPI* PFN_DebugSetMute)();

static HMODULE            g_realD3D9         = nullptr;
static PFN_D3DCreate9     g_pfnCreate9       = nullptr;
static PFN_D3DCreate9Ex   g_pfnCreate9Ex     = nullptr;
static PFN_D3DShaderValidator g_pfnShader    = nullptr;
static PFN_PerfBeginEvent g_pfnPerfBegin     = nullptr;
static PFN_PerfEndEvent   g_pfnPerfEnd       = nullptr;
static PFN_PerfGetStatus  g_pfnPerfGetStatus = nullptr;
static PFN_PerfQueryRepeat g_pfnPerfQuery    = nullptr;
static PFN_PerfSetMarker  g_pfnPerfMarker    = nullptr;
static PFN_PerfSetOptions g_pfnPerfOptions   = nullptr;
static PFN_PerfSetRegion  g_pfnPerfRegion    = nullptr;
static PFN_DebugSetLevel  g_pfnDebugLevel    = nullptr;
static PFN_DebugSetMute   g_pfnDebugMute     = nullptr;

static bool LoadRealD3D9()
{
    char path[MAX_PATH];
    GetSystemDirectoryA(path, MAX_PATH);
    strcat_s(path, "\\d3d9.dll");
    g_realD3D9 = LoadLibraryA(path);
    if (!g_realD3D9) return false;

#define GP(name, pfn) pfn = (decltype(pfn))GetProcAddress(g_realD3D9, name)
    GP("Direct3DCreate9",                 g_pfnCreate9);
    GP("Direct3DCreate9Ex",               g_pfnCreate9Ex);
    GP("Direct3DShaderValidatorCreate9",  g_pfnShader);
    GP("D3DPERF_BeginEvent",              g_pfnPerfBegin);
    GP("D3DPERF_EndEvent",                g_pfnPerfEnd);
    GP("D3DPERF_GetStatus",               g_pfnPerfGetStatus);
    GP("D3DPERF_QueryRepeatFrame",        g_pfnPerfQuery);
    GP("D3DPERF_SetMarker",               g_pfnPerfMarker);
    GP("D3DPERF_SetOptions",              g_pfnPerfOptions);
    GP("D3DPERF_SetRegion",               g_pfnPerfRegion);
    GP("DebugSetLevel",                   g_pfnDebugLevel);
    GP("DebugSetMute",                    g_pfnDebugMute);
#undef GP
    return true;
}


// ═════════════════════════════════════════════════════════════════════════════
//  PART 0B — PROXY EXPORTS (prefix: proxy_ — conflict نداره با d3d9.h)
// ═════════════════════════════════════════════════════════════════════════════

extern "C" {

IDirect3D9* WINAPI proxy_Direct3DCreate9(UINT sdkVer)
{ return g_pfnCreate9 ? g_pfnCreate9(sdkVer) : nullptr; }

HRESULT WINAPI proxy_Direct3DCreate9Ex(UINT sdkVer, IDirect3D9Ex** pp)
{ return g_pfnCreate9Ex ? g_pfnCreate9Ex(sdkVer, pp) : E_FAIL; }

void* WINAPI proxy_D3DShaderValidator()
{ return g_pfnShader ? g_pfnShader() : nullptr; }

int WINAPI proxy_D3DPERF_BeginEvent(D3DCOLOR c, LPCWSTR n)
{ return g_pfnPerfBegin ? g_pfnPerfBegin(c, n) : 0; }

int WINAPI proxy_D3DPERF_EndEvent()
{ return g_pfnPerfEnd ? g_pfnPerfEnd() : 0; }

DWORD WINAPI proxy_D3DPERF_GetStatus()
{ return g_pfnPerfGetStatus ? g_pfnPerfGetStatus() : 0; }

BOOL WINAPI proxy_D3DPERF_QueryRepeatFrame()
{ return g_pfnPerfQuery ? g_pfnPerfQuery() : FALSE; }

void WINAPI proxy_D3DPERF_SetMarker(D3DCOLOR c, LPCWSTR n)
{ if (g_pfnPerfMarker) g_pfnPerfMarker(c, n); }

void WINAPI proxy_D3DPERF_SetOptions(DWORD o)
{ if (g_pfnPerfOptions) g_pfnPerfOptions(o); }

void WINAPI proxy_D3DPERF_SetRegion(D3DCOLOR c, LPCWSTR n)
{ if (g_pfnPerfRegion) g_pfnPerfRegion(c, n); }

HRESULT WINAPI proxy_DebugSetLevel(DWORD lvl)
{ return g_pfnDebugLevel ? g_pfnDebugLevel(lvl) : S_OK; }

void WINAPI proxy_DebugSetMute()
{ if (g_pfnDebugMute) g_pfnDebugMute(); }

} // extern "C"


// ═════════════════════════════════════════════════════════════════════════════
//  PART 1 — MEMORY PATCH HELPERS
// ═════════════════════════════════════════════════════════════════════════════

static void PatchBytes(uintptr_t addr, const uint8_t* data, size_t len)
{
    DWORD old;
    void* ptr = (void*)addr;
    VirtualProtect(ptr, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy(ptr, data, len);
    VirtualProtect(ptr, len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), ptr, len);
}

template<typename T>
static void PatchVal(uintptr_t addr, T val)
{ PatchBytes(addr, (const uint8_t*)&val, sizeof(T)); }


// ═════════════════════════════════════════════════════════════════════════════
//  PART 2 — 200 FPS UNLOCK
// ═════════════════════════════════════════════════════════════════════════════

static constexpr int   TARGET_FPS      = 200;
static constexpr float TARGET_FRAME_MS = 1000.0f / TARGET_FPS;

static constexpr uintptr_t ADDR_FPS_BOOL  = 0x00C1703C;
static constexpr uintptr_t ADDR_FPS_DELAY = 0x00C17040;
static constexpr uintptr_t ADDR_FPS_CALL  = 0x0053E943;

static void PatchFPS()
{
    PatchVal<uint8_t>(ADDR_FPS_BOOL,  0x00);
    PatchVal<float>  (ADDR_FPS_DELAY, TARGET_FRAME_MS);
    static const uint8_t nop5[] = {0x90,0x90,0x90,0x90,0x90};
    PatchBytes(ADDR_FPS_CALL, nop5, 5);
}

typedef HRESULT(APIENTRY* PFN_Present)(
    IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
static PFN_Present g_oPresent = nullptr;

static LARGE_INTEGER g_freq     = {};
static LARGE_INTEGER g_lastTime = {};
static bool          g_timerOK  = false;

static void CapFPS()
{
    if (!g_timerOK) return;
    LONGLONG tpf = g_freq.QuadPart / TARGET_FPS;
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    LONGLONG elapsed = now.QuadPart - g_lastTime.QuadPart;
    if (elapsed < tpf)
    {
        LONGLONG rem    = tpf - elapsed;
        LONGLONG sleepT = rem - g_freq.QuadPart / 2000;
        if (sleepT > 0)
        {
            DWORD ms = (DWORD)((sleepT * 1000) / g_freq.QuadPart);
            if (ms > 0) Sleep(ms - 1);
        }
        do { QueryPerformanceCounter(&now); }
        while ((now.QuadPart - g_lastTime.QuadPart) < tpf);
    }
    g_lastTime = now;
}

static HRESULT APIENTRY hkPresent(
    IDirect3DDevice9* dev, const RECT* s, const RECT* d,
    HWND hw, const RGNDATA* rg)
{
    CapFPS();
    return g_oPresent(dev, s, d, hw, rg);
}

static void HookPresent()
{
    // Fix 2: از g_pfnCreate9 استفاده کن — نه Direct3DCreate9
    // اگه loop میکردیم به proxy خودمون crash میشد
    if (!g_pfnCreate9) return;
    IDirect3D9* d3d = g_pfnCreate9(D3D_SDK_VERSION);
    if (!d3d) return;

    D3DPRESENT_PARAMETERS pp = {};
    pp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    pp.Windowed         = TRUE;
    pp.BackBufferFormat = D3DFMT_UNKNOWN;
    pp.BackBufferCount  = 1;
    pp.BackBufferWidth  = 2;
    pp.BackBufferHeight = 2;
    pp.hDeviceWindow    = GetDesktopWindow();

    IDirect3DDevice9* dev = nullptr;
    if (FAILED(d3d->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF,
        pp.hDeviceWindow,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &pp, &dev)))
    {
        d3d->Release();
        return;
    }

    void** vmt = *(void***)dev;
    DWORD  old = 0;
    VirtualProtect(&vmt[17], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    g_oPresent = (PFN_Present)vmt[17];
    vmt[17]    = (void*)hkPresent;
    VirtualProtect(&vmt[17], sizeof(void*), old, &old);

    dev->Release();
    d3d->Release();
}


// ═════════════════════════════════════════════════════════════════════════════
//  PART 3 — THREAD POOL EXPANSION 96 → 200
// ═════════════════════════════════════════════════════════════════════════════

static constexpr int    OLD_COUNT   = 96;
static constexpr int    NEW_COUNT   = 200;
static constexpr size_t SCRIPT_SIZE = 0xE4;

static constexpr uintptr_t ADDR_SCRIPTS_ARRAY = 0x00A8D040;

static uint8_t* g_newPool = nullptr;

static void ExpandThreadPool()
{
    size_t newBytes = NEW_COUNT * SCRIPT_SIZE;
    g_newPool = (uint8_t*)VirtualAlloc(
        nullptr, newBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!g_newPool) return;

    memcpy(g_newPool, (void*)ADDR_SCRIPTS_ARRAY, OLD_COUNT * SCRIPT_SIZE);
    memset(g_newPool + OLD_COUNT * SCRIPT_SIZE, 0,
           (NEW_COUNT - OLD_COUNT) * SCRIPT_SIZE);

    // آدرس های array ref در SA 1.0 US
    static const uintptr_t ARRAY_REFS[] = {
        0x0045D650, 0x0046A4B5, 0x00480090, 0x004800D0
    };
    for (auto a : ARRAY_REFS)
        PatchVal<uint32_t>(a + 3, (uint32_t)(uintptr_t)g_newPool);

    // thread count 96 → 200 (0xC8)
    static const uint8_t cnt200[] = {0xC8, 0x00, 0x00, 0x00};
    static const uintptr_t COUNT_REFS[] = {
        0x0045D690, 0x0046A4E0, 0x004800A5
    };
    for (auto a : COUNT_REFS)
        PatchBytes(a, cnt200, 4);
}


// ═════════════════════════════════════════════════════════════════════════════
//  PART 4 — main2.scm LOADER
// ═════════════════════════════════════════════════════════════════════════════

#pragma pack(push, 1)
struct CRunningScript
{
    uint8_t  _p0[8];
    char     m_szName[8];
    uint8_t* m_pBaseIP;
    uint8_t* m_pIP;
    uint8_t* m_pStack[8];
    uint16_t m_nSP;
    uint8_t  _p1[2];
    int32_t  m_aLocals[40];
    uint8_t  m_bIsActive;
    uint8_t  m_bCondResult;
    uint8_t  _p2[6];
};
#pragma pack(pop)

typedef CRunningScript* (__cdecl* PFN_StartScript)(uint8_t*);
static constexpr uintptr_t ADDR_START_SCRIPT = 0x0046A4B0;

static uint8_t* g_scm2Buf  = nullptr;
static DWORD    g_scm2Size = 0;

static void LoadMain2SCM()
{
    char exe[MAX_PATH];
    GetModuleFileNameA(nullptr, exe, MAX_PATH);
    char* sl = strrchr(exe, '\\');
    if (sl) *(sl + 1) = '\0';

    char scmPath[MAX_PATH];
    sprintf_s(scmPath, "%sdata\\script\\main2.scm", exe);

    HANDLE hF = CreateFileA(scmPath, GENERIC_READ, FILE_SHARE_READ,
                            nullptr, OPEN_EXISTING, 0, nullptr);
    if (hF == INVALID_HANDLE_VALUE) return;

    g_scm2Size = GetFileSize(hF, nullptr);
    if (!g_scm2Size || g_scm2Size == INVALID_FILE_SIZE)
    { CloseHandle(hF); return; }

    g_scm2Buf = (uint8_t*)VirtualAlloc(
        nullptr, g_scm2Size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_scm2Buf) { CloseHandle(hF); return; }

    DWORD rd = 0;
    ReadFile(hF, g_scm2Buf, g_scm2Size, &rd, nullptr);
    CloseHandle(hF);
    if (rd != g_scm2Size) { VirtualFree(g_scm2Buf,0,MEM_RELEASE); return; }

    auto pfn = (PFN_StartScript)ADDR_START_SCRIPT;
    CRunningScript* t = pfn(g_scm2Buf);
    if (t)
    {
        strncpy_s(t->m_szName, "MAIN2", 8);
        t->m_bIsActive   = 1;
        t->m_pBaseIP     = g_scm2Buf;
        t->m_bCondResult = 0;
    }
}


// ═════════════════════════════════════════════════════════════════════════════
//  PART 5 — ScriptsInit HOOK
// ═════════════════════════════════════════════════════════════════════════════

typedef void (__cdecl* PFN_ScriptsInit)();
static PFN_ScriptsInit    g_oScriptsInit     = nullptr;
static constexpr uintptr_t ADDR_SCRIPTS_INIT = 0x0045D440;

static void __cdecl hkScriptsInit()
{
    if (g_oScriptsInit) g_oScriptsInit();
    LoadMain2SCM();
}

static void HookScriptsInit()
{
    uint8_t* t = (uint8_t*)ADDR_SCRIPTS_INIT;
    DWORD old;
    VirtualProtect(t, 5, PAGE_EXECUTE_READWRITE, &old);
    g_oScriptsInit = (PFN_ScriptsInit)(uintptr_t)(
        ADDR_SCRIPTS_INIT + 5 + *(int32_t*)(t + 1));
    t[0] = 0xE9;
    *(int32_t*)(t + 1) = (int32_t)((uintptr_t)hkScriptsInit - ADDR_SCRIPTS_INIT - 5);
    VirtualProtect(t, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), t, 5);
}


// ═════════════════════════════════════════════════════════════════════════════
//  PART 6 — INIT THREAD + DllMain
// ═════════════════════════════════════════════════════════════════════════════

static DWORD WINAPI InitThread(LPVOID)
{
    Sleep(3000);

    if (QueryPerformanceFrequency(&g_freq))
    {
        QueryPerformanceCounter(&g_lastTime);
        g_timerOK = true;
    }

    timeBeginPeriod(1);
    PatchFPS();
    HookPresent();
    ExpandThreadPool();
    HookScriptsInit();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hMod);
        if (!LoadRealD3D9()) return FALSE;
        CloseHandle(CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr));
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        timeEndPeriod(1);
        if (g_newPool)  VirtualFree(g_newPool,  0, MEM_RELEASE);
        if (g_scm2Buf)  VirtualFree(g_scm2Buf,  0, MEM_RELEASE);
        if (g_realD3D9) FreeLibrary(g_realD3D9);
    }
    return TRUE;
}