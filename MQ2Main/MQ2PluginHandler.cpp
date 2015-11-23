/*****************************************************************************
MQ2Main.dll: MacroQuest2's extension DLL for EverQuest
Copyright (C) 2002-2003 Plazmic, 2003-2005 Lax

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
******************************************************************************/
#if !defined(ISXEQ) && !defined(ISXEQ_LEGACY)

#if !defined(CINTERFACE)
#error /DCINTERFACE
#endif

#define DBG_SPEW

//#define DEBUG_PLUGINS

#ifdef DEBUG_PLUGINS
#define PluginDebug DebugSpew
#else
#define PluginDebug //
#endif

#include "MQ2Main.h"


CRITICAL_SECTION gPluginCS;
BOOL bPluginCS=0;

DWORD checkme(char *module)
{
    char *p;
    PIMAGE_DOS_HEADER pd = (PIMAGE_DOS_HEADER)module;
    PIMAGE_FILE_HEADER pf;
    p = module + pd->e_lfanew;
    p += 4;  //skip sig
    pf = (PIMAGE_FILE_HEADER) p;
    return pf->TimeDateStamp;
}

const char* MQ2Runtime = MQ2RUNTIMEVERSION();

static unsigned int mq2mainstamp = 0;

// The authoritative list of plugins
vector<unique_ptr<MQPLUGIN>> g_plugins;

static bool g_iteratingPlugins = false;

PMQPLUGIN FindPlugin(const char* pluginName)
{
    CAutoLock Lock(&gPluginCS);

    auto it = std::find_if(g_plugins.begin(), g_plugins.end(),
        [pluginName](const unique_ptr<MQPLUGIN>& plugin)
    {
        return _stricmp(plugin->szFilename, pluginName) == 0;
    });

    return it == g_plugins.end() ? nullptr : it->get();
}

PMQPLUGIN FindPluginByHandle(HMODULE module)
{
    CAutoLock Lock(&gPluginCS);

    auto it = std::find_if(g_plugins.begin(), g_plugins.end(),
        [module](const unique_ptr<MQPLUGIN>& plugin)
    {
        return plugin->hModule == module;
    });

    return it == g_plugins.end() ? nullptr : it->get();
}


template <typename T, typename... Args>
inline static void InvokePlugins(const T& func, Args&&... args)
{
    CAutoLock Lock(&gPluginCS);
    g_iteratingPlugins = true;

    for (auto iter = g_plugins.begin(); iter != g_plugins.end(); ++iter)
    {
        const auto& plugin = *iter;

        if (!plugin->bRemoved)
            (plugin->plugin.get()->*func)(std::forward<Args>(args)...);
    }

    g_iteratingPlugins = false;
}

static void AddPluginToLinkedList(PMQPLUGIN pPlugin)
{
    CAutoLock Lock(&gPluginCS);

    pPlugin->pLast = 0;
    pPlugin->pNext = pPlugins;
    if (pPlugins)
        pPlugins->pLast = pPlugin;
    pPlugins = pPlugin;
}

static void RemovePluginFromLinkedList(PMQPLUGIN pPlugin)
{
    CAutoLock Lock(&gPluginCS);

    if (pPlugin->pLast)
        pPlugin->pLast->pNext = pPlugin->pNext;
    else
        pPlugins = pPlugin->pNext;
    if (pPlugin->pNext)
        pPlugin->pNext->pLast = pPlugin->pLast;
}

// A LegacyPlugin is a plugin that implements the old style of plugin.
// This will redirect all calls to the global exports of the dll
class LegacyPlugin : public IPlugin
{
public:
    LegacyPlugin(HMODULE hmod)
        : capabilities_(0)
    {
        initialize_ = (fMQInitializePlugin)GetProcAddress(hmod, "InitializePlugin");
        shutdown_ = (fMQShutdownPlugin)GetProcAddress(hmod, "ShutdownPlugin");
        incoming_chat_ = (fMQIncomingChat)GetProcAddress(hmod, "OnIncomingChat");
        pulse_ = (fMQPulse)GetProcAddress(hmod, "OnPulse");
        write_chat_color_ = (fMQWriteChatColor)GetProcAddress(hmod, "OnWriteChatColor");
        zoned_ = (fMQZoned)GetProcAddress(hmod, "OnZoned");
        clean_ui_ = (fMQCleanUI)GetProcAddress(hmod, "OnCleanUI");
        reload_ui_ = (fMQReloadUI)GetProcAddress(hmod, "OnReloadUI");
        draw_hud_ = (fMQDrawHUD)GetProcAddress(hmod, "OnDrawHUD");
        set_game_state_ = (fMQSetGameState)GetProcAddress(hmod, "SetGameState");
        add_spawn_ = (fMQSpawn)GetProcAddress(hmod, "OnAddSpawn");
        remove_spawn_ = (fMQSpawn)GetProcAddress(hmod, "OnRemoveSpawn");
        add_ground_item_ = (fMQGroundItem)GetProcAddress(hmod, "OnAddGroundItem");
        remove_ground_item_ = (fMQGroundItem)GetProcAddress(hmod, "OnRemoveGroundItem");
        begin_zone_ = (fMQBeginZone)GetProcAddress(hmod, "OnBeginZone");
        end_zone_ = (fMQEndZone)GetProcAddress(hmod, "OnEndZone");

        if (add_spawn_ || remove_spawn_)
            capabilities_ |= PLUGIN_WANTS_SPAWNS;
        if (add_ground_item_ || remove_ground_item_)
            capabilities_ |= PLUGIN_WANTS_GROUNDITEMS;
    }

    virtual int GetVersion() const { return 0; }

    virtual int GetCapabilities() const { return capabilities_; }

    virtual void Initialize() override
    {
        if (initialize_) initialize_();
    }

    virtual void Shutdown() override
    {
        if (shutdown_) shutdown_();
    }

    virtual void OnCleanUI() override
    {
        if (clean_ui_) clean_ui_();
    }

    virtual void OnReloadUI() override
    {
        if (reload_ui_) reload_ui_();
    }

    virtual void OnDrawHUD() override
    {
        if (draw_hud_) draw_hud_();
    }

    virtual void OnSetGameState(DWORD GameState) override
    {
        if (set_game_state_) set_game_state_(GameState);
    }

    virtual void OnPulse() override
    {
        if (pulse_) pulse_();
    }

    virtual DWORD OnWriteChatColor(PCHAR Line, DWORD Color, DWORD Filter) override
    {
        if (write_chat_color_) return write_chat_color_(Line, Color, Filter);
        return 0;
    }

    virtual DWORD OnIncomingChat(PCHAR Line, DWORD Color) override
    {
        if (incoming_chat_) return incoming_chat_(Line, Color);
        return 0;
    }

    virtual void OnAddSpawn(PSPAWNINFO pNewSpawn) override
    {
        if (add_spawn_) add_spawn_(pNewSpawn);
    }

    virtual void OnRemoveSpawn(PSPAWNINFO pSpawn) override
    {
        if (remove_spawn_) remove_spawn_(pSpawn);
    }

    virtual void OnAddGroundItem(PGROUNDITEM pNewGroundItem) override
    {
        if (add_ground_item_) add_ground_item_(pNewGroundItem);
    }

    virtual void OnRemoveGroundItem(PGROUNDITEM pGroundItem) override
    {
        if (remove_ground_item_) remove_ground_item_(pGroundItem);
    }

    virtual void OnZoned() override
    {
        if (zoned_) zoned_();
    }

    virtual void OnBeginZone() override
    {
        if (begin_zone_) begin_zone_();
    }

    virtual void OnEndZone() override
    {
        if (end_zone_) end_zone_();
    }

private:
    fMQInitializePlugin initialize_;
    fMQShutdownPlugin shutdown_;
    fMQZoned zoned_;
    fMQWriteChatColor write_chat_color_;
    fMQPulse pulse_;
    fMQIncomingChat incoming_chat_;
    fMQCleanUI clean_ui_;
    fMQReloadUI reload_ui_;
    fMQDrawHUD draw_hud_;
    fMQSetGameState set_game_state_;
    fMQSpawn add_spawn_;
    fMQSpawn remove_spawn_;
    fMQGroundItem add_ground_item_;
    fMQGroundItem remove_ground_item_;
    fMQBeginZone begin_zone_;
    fMQEndZone end_zone_;
    int capabilities_;
};

unique_ptr<MQPLUGIN> CreatePlugin(HMODULE module, BOOL bCustom, CHAR* szFilename)
{
    unique_ptr<MQPLUGIN> pPlugin(new MQPLUGIN);
    pPlugin->bCustom = bCustom;
    pPlugin->hModule = module;
    strcpy_s(pPlugin->szFilename, szFilename);
    pPlugin->fpVersion = 1.0;
    pPlugin->bRemoved = false;

    // We'll need to determine what kind of plugin we are loading.
    // New versions of the plugin interface export a function called "MQ2PluginFactory"
    fMQPluginFactory factory = (fMQPluginFactory)GetProcAddress(module, "MQ2PluginFactory");
    if (factory)
    {
        // new style plugin
        pPlugin->plugin = factory();

        if (!pPlugin->plugin)
            return NULL;
    }
    else
    {
        // old style plugin.
        pPlugin->plugin.reset(new LegacyPlugin(module)); // wtb std::make_unique!

        // populate version
        if (float* ftmp = (float*)GetProcAddress(module, "?MQ2Version@@3MA"))
            pPlugin->fpVersion = *ftmp;
    }

    pPlugin->capabilities = pPlugin->plugin->GetCapabilities();

    return pPlugin;
}

static string s_pluginError;
static bool s_pluginFailed = false;

VOID PluginFailed(const char* failureReason)
{
    s_pluginFailed = true;

    if (failureReason)
        s_pluginError = failureReason;
}

const char* GetPluginError()
{
    if (s_pluginFailed)
        return s_pluginError.c_str();

    return NULL;
}

DWORD LoadMQ2Plugin(const PCHAR pszFilename, BOOL bCustom, BOOL bForce)
{
    CHAR Filename[MAX_PATH] = { 0 };

    s_pluginError.clear();
    s_pluginFailed = true;

    strcpy(Filename, pszFilename);
    strlwr(Filename);
    PCHAR Temp = strstr(Filename, ".dll");
    if (Temp)
        Temp[0] = 0;
    CHAR TheFilename[MAX_STRING] = { 0 };
    sprintf(TheFilename, "%s.dll", Filename);
    if (HMODULE hThemod = GetModuleHandle(TheFilename)) {
        DebugSpew("LoadMQ2Plugin(%s) already loaded", TheFilename);
        s_pluginFailed = false;
        return PLUGIN_ALREADY_LOADED;
    }
    CAutoLock Lock(&gPluginCS);
    DebugSpew("LoadMQ2Plugin(%s)", Filename);

    CHAR FullFilename[MAX_STRING]={0};
    sprintf(FullFilename,"%s\\%s.dll",gszINIPath,Filename);

    HMODULE hmod = LoadLibrary(FullFilename);
    if (!hmod)
    {
        LPTSTR errorText = NULL;

        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorText, 0, NULL);
        if (errorText != NULL)
        {
            DebugSpew("LoadMQ2Plugin(%s) failed: %s", Filename, errorText);
            s_pluginError = errorText;
            LocalFree(errorText);

            errorText = NULL;
        }
        else
        {
            DebugSpew("LoadMQ2Plugin(%s) Failed", Filename);
        }
        return PLUGIN_LOAD_FAILED;
    }

    if (!bForce)
    {
        if (!mq2mainstamp) {
            mq2mainstamp = checkme((char*)GetModuleHandle("mq2main.dll"));
        }
        if (mq2mainstamp > checkme((char*)hmod)) {
            char tmpbuff[MAX_PATH];
            s_pluginError = "Plugin is older than MQ2Main";
            sprintf(tmpbuff, "Please recompile %s -- it is out of date with respect to mq2main (%d>%d)", FullFilename, mq2mainstamp, checkme((char*)hmod));
            DebugSpew(tmpbuff);
            MessageBoxA(NULL, tmpbuff, "Plugin Load Failed", MB_OK);
            FreeLibrary(hmod);
            return PLUGIN_LOAD_FAILED;
        }
    }

    if (PMQPLUGIN pPlugin = FindPluginByHandle(hmod))
    {
        DebugSpew("LoadMQ2Plugin(%s) already loaded", Filename);

        // LoadLibrary count must match FreeLibrary count for unloading to work.
        FreeLibrary(hmod);
        s_pluginFailed = false;
        return PLUGIN_ALREADY_LOADED; // already loaded
    }

    if (!bForce)
    {
        // Validate the plugin's runtime version
        const char** ppRuntimeStr = (const char**)GetProcAddress(hmod, "MQ2Runtime");
        if (!ppRuntimeStr || strcmp(*ppRuntimeStr, MQ2Runtime) != 0)
        {
            char tmpbuff[MAX_PATH];
            if (!ppRuntimeStr)
                sprintf_s(tmpbuff, "%s needs to be updated. It does not specify the compiler version.", FullFilename);
            else
                sprintf_s(tmpbuff, "%s needs to be updated. The compiler version does not match!\n\nExpected: %s\nActual: %s",
                    FullFilename, MQ2Runtime, *ppRuntimeStr);
            s_pluginError = "Compiler version mismatch";
            DebugSpew(tmpbuff);
            MessageBoxA(NULL, tmpbuff, "Plugin Load Failed", MB_OK);
            FreeLibrary(hmod);
            return PLUGIN_LOAD_FAILED;
        }
    }

    unique_ptr<MQPLUGIN> pPlugin = CreatePlugin(hmod, bCustom, Filename);

    if (pPlugin == nullptr) {
        s_pluginError = "Failed to create plugin";
    }
    else {
        // if we made it this far, assume that initialization is successful
        s_pluginFailed = false;
        pPlugin->plugin->Initialize();
    }

    if (s_pluginFailed) {
        DebugSpew("LoadMQ2Plugin(%s) initialization failed: %s", Filename, s_pluginError.c_str());
        FreeLibrary(hmod);
        return PLUGIN_LOAD_FAILED;
    }

    PluginCmdSort();

    pPlugin->plugin->OnSetGameState(gGameState);

    if (gGameState == GAMESTATE_INGAME)
    {
        // If the plugin wants spawns, give them spawns
        if ((pPlugin->capabilities & PLUGIN_WANTS_SPAWNS))
        {
            PSPAWNINFO pSpawn = (PSPAWNINFO)pSpawnList;
            while (pSpawn)
            {
                pPlugin->plugin->OnAddSpawn(pSpawn);
                pSpawn = pSpawn->pNext;
            }
        }

        // If the plugin wants grounditems, give them grounditems
        if ((pPlugin->capabilities & PLUGIN_WANTS_GROUNDITEMS))
        {
            PGROUNDITEM pItem = *(PGROUNDITEM*)pItemList;
            while (pItem)
            {
                pPlugin->plugin->OnAddGroundItem(pItem);
                pItem = pItem->pNext;
            }
        }
    }

    AddPluginToLinkedList(pPlugin.get());
    g_plugins.emplace_back(std::move(pPlugin));

    sprintf(FullFilename,"%s-AutoExec",Filename);
    LoadCfgFile(FullFilename,false);
    return PLUGIN_LOAD_SUCCESS;
}

//what is this?
//Well, microsoft decided that it was a grat idea to SILENTLY add dlls that crash to 2 registrykeys
//since mq2 uses plugins, there is no telling which will crash now and then
//and its likely this happens on patchdays as well
//anyway if they end up in any of these 2 reg keys freelibrary will fail
//it will look like this:
//"C:\\Program Files\\Company\\MyApp.exe"="$ IgnoreFreeLibrary<externModule.dll>"
//so we need to workaround this so called "feature" by checking of we exist in the keys
//and delete us if we do so we can unload properly. -eqmule Nov 19 2015
//also this crap is checked once on launch, so
//we cant just delete the value and freelibrary will work again
//at least next time they use mq2 it will unload
//maybe i should hook the freelibraryhook...
//gonna have to spend some time on this.
//maybe there is a application compatibility api we can call to unhook it...

void DeleteLayers(PMQPLUGIN pPlugin)
{
	//work in progress, patch was needed i got interrupted.
	return;
	//HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers
	//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers
	HKEY HkeyTemp = { 0 };
	HANDLE hFile = NULL;
	VALENT vl = { 0 };
	if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", &HkeyTemp)) {
	//	RegQueryMultipleValues(HkeyTemp,&vl,
	}

}

typedef struct _FAKEGPSTRING {
	USHORT Length;
	USHORT MaximumLength;
	PCHAR  Buffer;
} FAKEGPSTRING, *PFAKEGPSTRING;
typedef BOOL(WINAPI *fLdrGetProcedureAddress)(HMODULE, PFAKEGPSTRING, WORD, PVOID);
typedef BOOL(WINAPI *fFreeLibrary)(HMODULE);
static fFreeLibrary pFreeLibrary = 0;
static fLdrGetProcedureAddress pLdrGetProcedureAddress = 0;

static void FreeMQ2Plugin(MQPLUGIN* pPlugin)
{
    RemovePluginFromLinkedList(pPlugin);

    pPlugin->plugin->OnCleanUI();
    pPlugin->plugin->Shutdown();

    //DeleteLayers(pPlugin);

    if (pFreeLibrary)
        pFreeLibrary(pPlugin->hModule);
    else
        FreeLibrary(pPlugin->hModule);
}

BOOL UnloadMQ2Plugin(const PCHAR pszFilename)
{
    DebugSpew("UnloadMQ2Plugin");
    CHAR Filename[MAX_PATH]={0};
    strcpy(Filename,pszFilename);
    strlwr(Filename);
    PCHAR Temp=strstr(Filename,".dll");
    if (Temp)
        Temp[0]=0;

    // find plugin in list
    CAutoLock Lock(&gPluginCS);

	//work in progress, move on, nothing to see here need to test this, but they keep patching on me...
	//well after messing with this for a while, I realized I couldn't bypass it by just deleting the registry key
	//need to check if we can just bypass the GetprocAddress hook and call the nt version which isnt hooked by the shim engine...
	/*if (pLdrGetProcedureAddress = (fLdrGetProcedureAddress)GetProcAddress(GetModuleHandle("ntdll.dll"), "LdrGetProcedureAddress")) {
		if (HMODULE h = GetModuleHandle("Kernel32.dll")) {
			CHAR szFreeLib[32] = { 0 };
			strcpy_s(szFreeLib, "FreeLibrary");
			FAKEGPSTRING as = { 0 };
			as.Buffer = szFreeLib;
			as.Length = 11;
			as.MaximumLength = 11;
			DWORD addr = 0;
			BOOL ret = pLdrGetProcedureAddress(h, &as, NULL, (PVOID)&pFreeLibrary);
			Sleep(0);
			//pFreeLibrary = (fFreeLibrary)addr;
		}
	}*/

    PMQPLUGIN pPlugin = nullptr;
    auto it = std::find_if(g_plugins.begin(), g_plugins.end(),
        [pszFilename](const unique_ptr<MQPLUGIN>& plugin)
    {
        return _stricmp(plugin->szFilename, pszFilename) == 0;
    });

    if (it != g_plugins.end())
    {
        pPlugin = it->get();
        FreeMQ2Plugin(pPlugin);

        g_plugins.erase(it);
        return 1;
    }

    return 0;
}

VOID RewriteMQ2Plugins()
{
    CAutoLock Lock(&gPluginCS);
    CHAR PluginList[MAX_STRING * 10] = { 0 };
    CHAR szBuffer[MAX_STRING] = { 0 };
    CHAR MainINI[MAX_STRING] = { 0 };
    sprintf(MainINI, "%s\\macroquest.ini", gszINIPath);
    DWORD dwAttrs = 0, bChangedfileattribs = 0;
    if ((dwAttrs = GetFileAttributes(MainINI)) != INVALID_FILE_ATTRIBUTES) {
        if (dwAttrs & FILE_ATTRIBUTE_READONLY) {
            bChangedfileattribs = 1;
            SetFileAttributes(MainINI, FILE_ATTRIBUTE_NORMAL);
        }
    }
    GetPrivateProfileString("Plugins", NULL, "", PluginList, MAX_STRING * 10, MainINI);
    PCHAR pPluginList = PluginList;
    BOOL loadvalue = 0;
    while (pPluginList[0] != 0) {
        WritePrivateProfileString("Plugins", pPluginList, "0", gszINIFilename);
        pPluginList += strlen(pPluginList) + 1;
    }

    std::for_each(g_plugins.begin(), g_plugins.end(), [](const unique_ptr<MQPLUGIN>& plugin) {
        if (!plugin->bCustom)
            WritePrivateProfileString("Plugins", plugin->szFilename, "1", gszINIFilename);
    });

    if (bChangedfileattribs) {
        SetFileAttributes(MainINI, dwAttrs);
    }
}

VOID InitializeMQ2Plugins()
{
    DebugSpew("Initializing plugins");
    bmWriteChatColor = AddMQ2Benchmark("WriteChatColor");
    bmPluginsIncomingChat = AddMQ2Benchmark("PluginsIncomingChat");
    bmPluginsPulse = AddMQ2Benchmark("PluginsPulse");
    bmPluginsOnZoned = AddMQ2Benchmark("PluginsOnZoned");
    bmPluginsCleanUI = AddMQ2Benchmark("PluginsCleanUI");
    bmPluginsReloadUI = AddMQ2Benchmark("PluginsReloadUI");
    bmPluginsDrawHUD = AddMQ2Benchmark("PluginsDrawHUD");
    bmPluginsSetGameState = AddMQ2Benchmark("PluginsSetGameState");
    bmCalculate = AddMQ2Benchmark("Calculate");
    bmBeginZone = AddMQ2Benchmark("BeginZone");
    bmEndZone = AddMQ2Benchmark("EndZone");

    InitializeCriticalSection(&gPluginCS);
    bPluginCS = 1;

    CHAR PluginList[MAX_STRING * 10] = { 0 };
    CHAR szBuffer[MAX_STRING] = { 0 };
    CHAR MainINI[MAX_STRING] = { 0 };
    sprintf(MainINI, "%s\\macroquest.ini", gszINIPath);
    GetPrivateProfileString("Plugins", NULL, "", PluginList, MAX_STRING * 10, MainINI);
    PCHAR pPluginList = PluginList;
    BOOL loadvalue = 0;
    while (pPluginList[0] != 0) {
        GetPrivateProfileString("Plugins", pPluginList, "", szBuffer, MAX_STRING, MainINI);
        if (IsNumber(szBuffer)) {
            loadvalue = atoi(szBuffer);
            szBuffer[0] = '\0';
        }
        if (loadvalue == 1 || szBuffer[0] != 0) {
            LoadMQ2Plugin(pPluginList);
        }
        pPluginList += strlen(pPluginList) + 1;
    }
    //ok now check if user has a CustomPlugin.ini and load those as well...
    sprintf(MainINI, "%s\\CustomPlugins.ini", gszINIPath);
    GetPrivateProfileString("Plugins", NULL, "", PluginList, MAX_STRING * 10, MainINI);
    pPluginList = PluginList;
    while (pPluginList[0] != 0) {
        GetPrivateProfileString("Plugins", pPluginList, "", szBuffer, MAX_STRING, MainINI);
        if (IsNumber(szBuffer)) {
            loadvalue = atoi(szBuffer);
            szBuffer[0] = '\0';
        }
        if (loadvalue == 1 || szBuffer[0] != 0) {
            LoadMQ2Plugin(pPluginList, 1);
        }
        pPluginList += strlen(pPluginList) + 1;
    }
}

VOID UnloadMQ2Plugins()
{
    CAutoLock Lock(&gPluginCS);

    std::for_each(g_plugins.begin(), g_plugins.end(),
        [](const unique_ptr<MQPLUGIN>& plugin)
    {
        DebugSpew("%s->Unload()", plugin->szFilename);
        FreeMQ2Plugin(plugin.get());
    });

    g_plugins.clear();
}

VOID ShutdownMQ2Plugins()
{
    bPluginCS = 0;
    UnloadMQ2Plugins();

    Sleep(50); // fixes crash on Windows 7 (Vista too?) in RtlpWaitOnCriticalSection
    DeleteCriticalSection(&gPluginCS);
}

VOID WriteChatColor(PCHAR Line, DWORD Color, DWORD Filter)
{
    if (!bPluginCS)
        return;
    if (gFilterMQ)
        return;
    PluginDebug("Begin WriteChatColor()");
    EnterMQ2Benchmark(bmWriteChatColor);

    if (size_t len = strlen(Line)) {
        if (char *PlainText = (char*)LocalAlloc(LPTR, len + 1)) {
            StripMQChat(Line, PlainText);
            CheckChatForEvent(PlainText);
            LocalFree(PlainText);
        }
        DebugSpew("WriteChatColor(%s)", Line);
    }

    InvokePlugins(&IPlugin::OnWriteChatColor, Line, Color, Filter);

    ExitMQ2Benchmark(bmWriteChatColor);
}

BOOL PluginsIncomingChat(PCHAR Line, DWORD Color)
{
    PluginDebug("PluginsIncomingChat()");
    if (!bPluginCS)
        return 0;
    if (!Line[0])
        return 0;

    BOOL Ret = 0;
    InvokePlugins(&IPlugin::OnIncomingChatHelper, Line, Color, Ret);

    return Ret;
}

VOID PulsePlugins()
{
    PluginDebug("PulsePlugins()");
    if (!bPluginCS)
        return;

    InvokePlugins(&IPlugin::OnPulse);
}

VOID PluginsZoned()
{
    PluginDebug("PluginsZoned()");
    if (!bPluginCS)
        return;

    InvokePlugins(&IPlugin::OnZoned);

    char szTemp[256];
    sprintf(szTemp, "You have entered %s.", ((PZONEINFO)pZoneInfo)->LongName);
    CheckChatForEvent(szTemp);
}

VOID PluginsCleanUI()
{
    PluginDebug("PluginsCleanUI()");
    if (!bPluginCS)
        return;

    DeleteMQ2NewsWindow();

    InvokePlugins(&IPlugin::OnCleanUI);
}

VOID PluginsReloadUI()
{
    PluginDebug("PluginsReloadUI()");
    if (!bPluginCS)
        return;

    InvokePlugins(&IPlugin::OnReloadUI);
}

VOID PluginsSetGameState(DWORD GameState)
{
    PluginDebug("PluginsSetGameState()");
    static bool AutoExec=false;
    static bool CharSelect=true;
    DrawHUDParams[0]=0;
    if (!bPluginCS)
        return;
    gGameState=GameState;
    if (GameState==GAMESTATE_INGAME)
    {
        gZoning=false;
        gbDoAutoRun=TRUE;
        if (!AutoExec)
        {
            AutoExec=true;
            LoadCfgFile("AutoExec",false);
        }
        if (CharSelect)
        {
            CharSelect=false;
            CHAR szBuffer[MAX_STRING]={0};

            DebugSpew("PluginsSetGameState( %s server)",EQADDR_SERVERNAME);
            if (PCHARINFO pCharInfo=GetCharInfo())
            {
                DebugSpew("PluginsSetGameState( %s name)",pCharInfo->Name);
                sprintf(szBuffer,"%s_%s",EQADDR_SERVERNAME,pCharInfo->Name);
                LoadCfgFile(szBuffer,false);
            }
            if (PCHARINFO2 pCharInfo2=GetCharInfo2())
            {
                DebugSpew("PluginsSetGameState( %d class)",pCharInfo2->Class);
                sprintf(szBuffer,"%s",GetClassDesc(pCharInfo2->Class));
                LoadCfgFile(szBuffer,false);
            }
        }
    }
    else if (GameState==GAMESTATE_CHARSELECT)
    {
        if (!AutoExec)
        {
            AutoExec=true;
            LoadCfgFile("AutoExec",false);
        }
		DWORD nThreadId = 0;
		CreateThread(NULL, 0, InitializeMQ2SpellDb, 0, 0, &nThreadId);
        CharSelect=true;
        LoadCfgFile("CharSelect",false);
    }

    InvokePlugins(&IPlugin::OnSetGameState, GameState);
}

VOID PluginsDrawHUD()
{
    PluginDebug("PluginsDrawHUD()");
    if (!bPluginCS)
        return;

    InvokePlugins(&IPlugin::OnDrawHUD);
}

VOID PluginsAddSpawn(PSPAWNINFO pNewSpawn)
{
    DWORD BodyType = GetBodyType(pNewSpawn);
    PluginDebug("PluginsAddSpawn(%s,%d,%d)", pNewSpawn->Name, pNewSpawn->Race, BodyType);
    if (!bPluginCS)
        return;
    if (gGameState > GAMESTATE_CHARSELECT)
        SetNameSpriteState(pNewSpawn, 1);
    if (GetBodyTypeDesc(BodyType)[0] == '*')
    {
        WriteChatf("Spawn '%s' has unknown bodytype %d", pNewSpawn->Name, BodyType);
    }

    InvokePlugins(&IPlugin::OnAddSpawn, pNewSpawn);
}

VOID PluginsRemoveSpawn(PSPAWNINFO pSpawn)
{
    PluginDebug("PluginsRemoveSpawn(%s)", pSpawn->Name);
    SpawnByName.erase(pSpawn->Name);
    if (!bPluginCS)
        return;

    InvokePlugins(&IPlugin::OnRemoveSpawn, pSpawn);
}

VOID PluginsAddGroundItem(PGROUNDITEM pNewGroundItem)
{
    PluginDebug("PluginsAddGroundItem()");
    if (!bPluginCS)
        return;
    DebugSpew("PluginsAddGroundItem(%s) %.1f,%.1f,%.1f", pNewGroundItem->Name,
        pNewGroundItem->X, pNewGroundItem->Y, pNewGroundItem->Z);

    InvokePlugins(&IPlugin::OnAddGroundItem, pNewGroundItem);
}

VOID PluginsRemoveGroundItem(PGROUNDITEM pGroundItem)
{
    PluginDebug("PluginsRemoveGroundItem()");
    if (!bPluginCS)
        return;

    InvokePlugins(&IPlugin::OnRemoveGroundItem, pGroundItem);
}

VOID PluginsBeginZone() 
{
    PluginDebug("PluginsBeginZone()");
    if (!bPluginCS)
        return;
    gbInZone = false;

    InvokePlugins(&IPlugin::OnBeginZone);
}

VOID PluginsEndZone() 
{
    PluginDebug("PluginsEndZone()");
    if (!bPluginCS)
        return;
    gbInZone = true;

    InvokePlugins(&IPlugin::OnEndZone);

    LoadCfgFile("zoned", true);
    LoadCfgFile(((PZONEINFO)pZoneInfo)->ShortName, false);
} 

#endif
