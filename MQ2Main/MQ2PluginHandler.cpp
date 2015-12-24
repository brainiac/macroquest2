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

#include <iterator>

CRITICAL_SECTION gPluginCS;
BOOL bPluginCS = 0;

static string s_pluginError;
static bool s_pluginFailed = false;

const char* MQ2Runtime = MQ2RUNTIMEVERSION();
static unsigned int mq2mainstamp = 0;

// The authoritative list of plugins
vector<shared_ptr<MQ2PluginInfo>> g_plugins;

static bool g_iteratingPlugins = false;


//----------------------------------------------------------------------------

DWORD GetModuleTimestamp(char *module)
{
    char *p;
    PIMAGE_DOS_HEADER pd = (PIMAGE_DOS_HEADER)module;
    PIMAGE_FILE_HEADER pf;
    p = module + pd->e_lfanew;
    p += 4;  //skip sig
    pf = (PIMAGE_FILE_HEADER) p;
    return pf->TimeDateStamp;
}

shared_ptr<MQ2PluginInfo> FindPluginShared(const char* pluginName)
{
    CAutoLock Lock(&gPluginCS);

    auto it = std::find_if(g_plugins.begin(), g_plugins.end(),
        [pluginName](const auto& plugin)
    {
        return _stricmp(plugin->name.c_str(), pluginName) == 0;
    });

    return it == g_plugins.end() ? nullptr : *it;
}

MQ2PluginInfo* FindPlugin(const char* pluginName)
{
    return FindPluginShared(pluginName).get();
}

MQ2PluginInfo* FindPluginByHandle(HMODULE module)
{
    CAutoLock Lock(&gPluginCS);

    auto it = std::find_if(g_plugins.begin(), g_plugins.end(),
        [module](const auto& plugin)
    {
        return plugin->hModule == module;
    });

    return it == g_plugins.end() ? nullptr : it->get();
}

bool IsPluginLoaded(const char* pluginName)
{
    CAutoLock Lock(&gPluginCS);

    auto it = std::find_if(g_plugins.begin(), g_plugins.end(),
        [pluginName](const auto& plugin)
    {
        return _stricmp(plugin->name.c_str(), pluginName) == 0;
    });

    return it != g_plugins.end();
}

namespace detail {
    template <class F, class... Args>
    inline auto invoke(F&& f, Args&&... args) ->
        decltype(std::forward<F>(f)(std::forward<Args>(args)...)) {
        return std::forward<F>(f)(std::forward<Args>(args)...);
    }

    template <class PMF, class Pointer, class... Args>
    inline auto invoke(PMF pmf, Pointer&& ptr, Args&&... args) ->
        decltype(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...)) {
        return ((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...);
    }
}

template <typename T, typename... Args>
inline static void InvokePlugins(const T& func, Args&&... args)
{
    CAutoLock Lock(&gPluginCS);
    g_iteratingPlugins = true;

    for (const auto& plugin : g_plugins)
    {
        if (plugin->removed)
            continue;
        detail::invoke(func, plugin->plugin.get(), std::forward<Args>(args)...);
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

static void AddPluginToLinkedList(MQ2PluginInfo* pluginInfo)
{
    CAutoLock Lock(&gPluginCS);

    // find existing or create new
    PMQPLUGIN pPlugin = pPlugins;
    while (pPlugin)
    {
        if (_stricmp(pPlugin->szFilename, pluginInfo->name.c_str()) == 0)
            return;
        pPlugin = pPlugin->pNext;
    }
    // plugin not found
    pPlugin = new MQPLUGIN;
    strcpy_s(pPlugin->szFilename, pluginInfo->name.c_str());
    pPlugin->hModule = pluginInfo->hModule;
    pPlugin->fpVersion = pluginInfo->fpVersion;

    // special case for backwards compatibility. We need to look and see
    // if we have the version exported and copy it if we do.
    if (pPlugin->hModule)
    {
        // populate version
        if (float* ftmp = (float*)GetProcAddress(pluginInfo->hModule, "?MQ2Version@@3MA"))
            pPlugin->fpVersion = *ftmp;
    }

    pPlugin->bCustom = pluginInfo->custom;
    AddPluginToLinkedList(pPlugin);
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

static void RemovePluginFromLinkedList(MQ2PluginInfo* pluginInfo)
{
    CAutoLock Lock(&gPluginCS);

    // find existing plugin or return
    PMQPLUGIN pPlugin = pPlugins;
    while (pPlugin)
    {
        if (_stricmp(pPlugin->szFilename, pluginInfo->name.c_str()) == 0)
            break;
        pPlugin = pPlugin->pNext;
    }
    if (!pPlugin)
        return;

    RemovePluginFromLinkedList(pPlugin);
    delete pPlugin;
}

MQ2PluginInfo::MQ2PluginInfo(const char* filename_, HMODULE module_, bool custom_)
    : name(filename_)
    , custom(custom_)
    , removed(false)
    , hModule(module_)
    , fpVersion(1.0)
{
}

MQ2PluginInfo::~MQ2PluginInfo()
{
    plugin.reset();

    if (hModule)
        FreeLibrary(hModule);
}

PLUGINCONFIG::PLUGINCONFIG()
{
}

bool GetPluginConfiguration(const char* szPluginName, PPLUGINCONFIG config)
{
    HMODULE module = 0;
    bool needsRelease = false;
    bool success = false;

    bInspectingPlugins = 1;

    // first check to see if plugin is already loaded.
    if (MQ2PluginInfo* plugin = FindPlugin(szPluginName))
    {
        module = plugin->hModule;
    }
    else
    {
        // plugin not loaded, so temporarily load it.
        CHAR Filename[MAX_PATH] = { 0 };

        // strip .dll from plugin name if it exists and then lowercase
        strcpy_s(Filename, szPluginName);
        strlwr(Filename);
        PCHAR Temp = strstr(Filename, ".dll");
        if (Temp)
            Temp[0] = 0;

        CHAR FullFilename[MAX_STRING] = { 0 };
        sprintf_s(FullFilename, "%s\\%s.dll", gszINIPath, Filename);
        
        module = LoadLibrary(FullFilename);
        needsRelease = true;
    }

    if (module)
    {
        // Configure plugin (optional)
        fMQConfigurePlugin configure = (fMQConfigurePlugin)GetProcAddress(module, "ConfigurePlugin");
        if (configure)
        {
            configure(config);
        }

        if (needsRelease)
        {
            FreeLibrary(module);
        }

        success = true;
    }

    bInspectingPlugins = 0;

    return success;
}

// A LegacyPlugin is a plugin that implements the old style of plugin.
// This will redirect all calls to the global exports of the dll
class LegacyPlugin : public IPlugin
{
public:
    LegacyPlugin(HMODULE hmod)
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
    }

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
};

unique_ptr<MQ2PluginInfo> CreatePlugin(HMODULE module, BOOL bCustom, CHAR* szFilename)
{
    unique_ptr<MQ2PluginInfo> pPlugin = make_unique<MQ2PluginInfo>(szFilename, module, bCustom != 0);

    // We'll need to determine what kind of plugin we are loading.
    // New versions of the plugin interface export a function called "MQ2PluginFactory"
    fMQPluginFactory factory = (fMQPluginFactory)GetProcAddress(module,
        "?MQ2PluginFactory@@YA?AV?$unique_ptr@VIPlugin@@U?$default_delete@VIPlugin@@@std@@@std@@XZ");
    if (factory)
    {
        // new style plugin uses factory.
        pPlugin->plugin = factory();

        if (!pPlugin->plugin)
            return nullptr;
    }
    else
    {
        // old style plugin.
        pPlugin->plugin = make_unique<LegacyPlugin>(module);
    }

    // Configure plugin (optional)
    fMQConfigurePlugin configure = (fMQConfigurePlugin)GetProcAddress(module, "ConfigurePlugin");
    if (configure)
    {
        configure(pPlugin.get());
    }

    return pPlugin;
}

void PluginFailed(const char* failureReason)
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

DWORD LoadMQ2PluginEx(const char* pszFilename, BOOL bCustom, BOOL bForce)
{
    s_pluginError.clear();
    s_pluginFailed = false;

    CHAR Filename[MAX_PATH] = { 0 };

    // strip .dll from plugin name if it exists and then lowercase
    strcpy(Filename, pszFilename);
    strlwr(Filename);
    PCHAR Temp = strstr(Filename, ".dll");
    if (Temp)
        Temp[0] = 0;

    // if plugin is already loaded, check now.
    if (IsPluginLoaded(Filename)) {
        DebugSpew("LoadMQ2Plugin(%s) already loaded", Filename);
        return PLUGIN_ALREADY_LOADED;
    }

    // add it back to create the filename to load
    CHAR TheFilename[MAX_STRING] = { 0 };
    sprintf(TheFilename, "%s.dll", Filename);

    // if the plugin's dll has already been loaded
    if (HMODULE hThemod = GetModuleHandle(TheFilename)) {
        DebugSpew("LoadMQ2Plugin(%s) already loaded", TheFilename);
        return PLUGIN_ALREADY_LOADED;
    }

    CAutoLock Lock(&gPluginCS);
    DebugSpew("LoadMQ2Plugin(%s)", Filename);

    CHAR FullFilename[MAX_STRING] = { 0 };
    sprintf(FullFilename, "%s\\%s.dll", gszINIPath, Filename);

    // try to load the library. If it fails display a meaningful error.
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

        s_pluginFailed = true;
        return PLUGIN_LOAD_FAILED;
    }
    else
    {
        // check that the module isn't already loaded as another plugin (maybe
        // with a different name? maybe it was renamed. who knows)
        if (MQ2PluginInfo* pPlugin = FindPluginByHandle(hmod))
        {
            DebugSpew("LoadMQ2Plugin(%s) already loaded", Filename);

            // LoadLibrary count must match FreeLibrary count for unloading to work.
            FreeLibrary(hmod);
            return PLUGIN_ALREADY_LOADED;
        }
    }

    // do some additional validation, unless bForce is set to true!
    if (!bForce)
    {
        if (!mq2mainstamp) {
            mq2mainstamp = GetModuleTimestamp((char*)GetCurrentModule());
        }
        if (mq2mainstamp > GetModuleTimestamp((char*)hmod)) {
            char tmpbuff[MAX_PATH];
            s_pluginError = "Plugin is older than MQ2Main";
            sprintf(tmpbuff, "Please recompile %s -- it is out of date with respect to mq2main (%d>%d)", FullFilename, mq2mainstamp, GetModuleTimestamp((char*)hmod));
            DebugSpew("%s", tmpbuff);
            MessageBoxA(NULL, tmpbuff, "Plugin Load Failed", MB_OK);
            FreeLibrary(hmod);
            s_pluginFailed = true;
            return PLUGIN_LOAD_FAILED;
        }

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
            DebugSpew("%s", tmpbuff);
            MessageBoxA(NULL, tmpbuff, "Plugin Load Failed", MB_OK);
            FreeLibrary(hmod);
            s_pluginFailed = true;
            return PLUGIN_LOAD_FAILED;
        }
    }

    unique_ptr<MQ2PluginInfo> pPlugin = CreatePlugin(hmod, bCustom, Filename);

    // from this point on, if pPlugin is not null, it owns the dll module handle
    // and will free it when it is destroyed.

    if (pPlugin == nullptr) {
        DebugSpew("LoadMQ2Plugin(%s) failed: Failed to create plugin", Filename);
        s_pluginError = "Failed to create plugin";
        s_pluginFailed = true;
    }
    else {
        vector<string> missingDeps;

        // check dependencies
        if (!pPlugin->dependencies.empty()) {
            for (const std::string& dep : pPlugin->dependencies) {
                if (!IsPluginLoaded(dep.c_str())) {
                    missingDeps.push_back(dep);
                }
            }

            if (!missingDeps.empty() && bForce) {
                // try to load the dependencies
                vector<string> stillFailed;
                for (auto& dep : missingDeps) {
                    DWORD result = LoadMQ2PluginEx(dep.c_str(), bCustom, bForce);
                    if (result != PLUGIN_LOAD_SUCCESS
                           && result != PLUGIN_ALREADY_LOADED) {
                        stillFailed.push_back(dep);
                    }
                }
                std::swap(stillFailed, missingDeps);
            }
            if (!missingDeps.empty()) {
                char errorBuffer[256];
                sprintf_s(errorBuffer, "Missing plugin dependencies: ");
                int count = 0;
                for (const std::string& dep : missingDeps) {
                    if (count > 0)
                        strcat_s(errorBuffer, ", ");
                    strcat_s(errorBuffer, dep.c_str());
                    count++;
                }

                s_pluginError = errorBuffer;
                s_pluginFailed = true;
                DebugSpew("LoadMQ2Plugin(%s) failed: %s", Filename, errorBuffer);
                return PLUGIN_LOAD_FAILED;
            }
        }

        // if we made it this far, assume that initialization is successful
        s_pluginFailed = false;
        pPlugin->plugin->Initialize();
    }

    if (s_pluginFailed) {
        DebugSpew("LoadMQ2Plugin(%s) initialization failed: %s", Filename, s_pluginError.c_str());
        return PLUGIN_LOAD_FAILED;
    }

    // initialize the plugin's state
    pPlugin->plugin->OnSetGameState(gGameState);

    if (gGameState == GAMESTATE_INGAME)
    {
        PSPAWNINFO pSpawn = (PSPAWNINFO)pSpawnList;
        while (pSpawn)
        {
            pPlugin->plugin->OnAddSpawn(pSpawn);
            pSpawn = pSpawn->pNext;
        }

        PGROUNDITEM pItem = *(PGROUNDITEM*)pItemList;
        while (pItem)
        {
            pPlugin->plugin->OnAddGroundItem(pItem);
            pItem = pItem->pNext;
        }
    }

    AddPluginToLinkedList(pPlugin.get());
    g_plugins.emplace_back(std::move(pPlugin));
    InvokePlugins(&IPlugin::OnPluginLoaded, Filename);

    sprintf(FullFilename, "%s-AutoExec", Filename);
    LoadCfgFile(FullFilename, false);
    return PLUGIN_LOAD_SUCCESS;
}

DWORD LoadMQ2Plugin(const PCHAR pluginName, BOOL bCustom /* = 0 */, BOOL bForce /* = 0 */)
{
    DWORD result = LoadMQ2PluginEx(pluginName, bCustom, bForce);

    switch (result)
    { 
    case PLUGIN_LOAD_FAILED:
    case PLUGIN_LOAD_SUCCESS:
    case PLUGIN_ALREADY_LOADED:
        return result;

    default:
        return PLUGIN_LOAD_FAILED;
    }
}

static void CleanupMQ2Plugin(MQ2PluginInfo* pPlugin)
{
    RemovePluginFromLinkedList(pPlugin);
    InvokePlugins(&IPlugin::OnPluginUnloaded, pPlugin->name.c_str());

    pPlugin->plugin->OnCleanUI();
    pPlugin->plugin->Shutdown();
}

// get a list of plugins that are currently depending on this one.
vector<string> GetDependentPlugins(MQ2PluginInfo* plugin)
{
    vector<string> plugins;

    for (auto& otherPlugin : g_plugins)
    {
        if (otherPlugin.get() == plugin)
            continue;

        for (auto& dep : otherPlugin->dependencies)
        {
            if (stricmp(dep.c_str(), plugin->name.c_str()) == 0)
                plugins.push_back(otherPlugin->name);
        }
    }

    return plugins;
}

DWORD UnloadMQ2PluginEx(const char* pszFilename, bool force)
{
    DebugSpew("UnloadMQ2Plugin");
    CHAR Filename[MAX_PATH] = { 0 };
    strcpy(Filename, pszFilename);
    strlwr(Filename);
    PCHAR Temp = strstr(Filename, ".dll");
    if (Temp)
        Temp[0] = 0;

    // find plugin in list
    CAutoLock Lock(&gPluginCS);
    auto it = std::find_if(g_plugins.begin(), g_plugins.end(),
        [pszFilename](const shared_ptr<MQ2PluginInfo>& plugin)
    {
        return _stricmp(plugin->name.c_str(), pszFilename) == 0;
    });

    if (it == g_plugins.end()) {
        return PLUGIN_UNLOAD_NOT_FOUND;
    }

    // check if plugin is in use.
    auto otherPlugins = GetDependentPlugins(it->get());
    if (!otherPlugins.empty())
    {
        if (force) {
            for (const std::string& dep : otherPlugins) {
                DWORD result = UnloadMQ2PluginEx(dep.c_str(), true);
                if (result != PLUGIN_UNLOAD_NOT_FOUND
                        && result != PLUGIN_UNLOAD_SUCCESS) {

                    char errorBuffer[256] = { 0 };
                    sprintf_s(errorBuffer, "Failed to unload plugin: '%s'", dep.c_str());
                    s_pluginError = errorBuffer;
                    s_pluginFailed = true;

                    return PLUGIN_UNLOAD_IN_USE;
                }
                else if (result == PLUGIN_UNLOAD_SUCCESS) {
                    WriteChatf("Plugin '%s' unloaded.", dep.c_str());
                }
            }
        }
        else {
            char errorBuffer[256] = { 0 };
            int count = 0;
            for (const std::string& dep : otherPlugins) {
                if (count > 0)
                    strcat_s(errorBuffer, ", ");
                strcat_s(errorBuffer, dep.c_str());
                count++;
            }
            s_pluginError = errorBuffer;
            s_pluginFailed = true;

            return PLUGIN_UNLOAD_IN_USE;
        }
    }

    CleanupMQ2Plugin(it->get());
    g_plugins.erase(it);
    return PLUGIN_UNLOAD_SUCCESS;
}

bool UnloadMQ2Plugin(const PCHAR pluginName)
{
    DWORD result = UnloadMQ2PluginEx(pluginName, true);

    if (result == PLUGIN_UNLOAD_SUCCESS)
        return true;

    return false;
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

    for (auto& plugin : g_plugins)
    {
        if (!plugin->custom)
            WritePrivateProfileString("Plugins", plugin->name.c_str(), "1", gszINIFilename);
    }

    if (bChangedfileattribs) {
        SetFileAttributes(MainINI, dwAttrs);
    }
}

// case insensitive comparison operator for map and set.
struct ci_less
{
    struct nocase_compare {
        bool operator()(const unsigned char& c1, const unsigned char& c2) const {
            return tolower(c1) < tolower(c2);
        }
    };
    bool operator()(const std::string& s1, const std::string& s2) const {
        return std::lexicographical_compare(s1.begin(), s1.end(),
            s2.begin(), s2.end(), nocase_compare());
    }
};

template <typename SortedList, typename DependencyMap, typename Inputs>
void TopologicalSort(SortedList& sortedList, DependencyMap& dependencies, Inputs& pluginsToLoad)
{
    // helper to remove a plugin from all dependencies (assume dep is met)
    bool changed = false;
    auto MeetDep = [&](const string& dep) {
        sortedList.push_back(dep);
        for (auto& v : dependencies) {
            v.second.erase(dep);
        }
        pluginsToLoad.erase(dep);
        changed = true;
    };

    // Topological sort, order the plugins so that they load in proper order
    do
    {
        changed = false;
        for (auto& plugin : pluginsToLoad)
        {
            // does this plugin have any remaining dependencies:
            auto& deps = dependencies[plugin];

            if (deps.size() == 0)
                MeetDep(plugin);
        }
    } while (!pluginsToLoad.empty() && changed);

    // take the remaining plugins and push the into the sorted list
    copy(pluginsToLoad.begin(), pluginsToLoad.end(), back_inserter(sortedList));
}

void LoadSavedMQ2Plugins(const char* iniFile, bool isCustom)
{
    CHAR PluginList[MAX_STRING * 10] = { 0 };
    CHAR szBuffer[MAX_STRING] = { 0 };
    CHAR MainINI[MAX_STRING] = { 0 };

    set<string, ci_less> pluginsToLoad;

    sprintf_s(MainINI, "%s\\%s.ini", gszINIPath, iniFile);
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
            pluginsToLoad.insert(pPluginList);
        }
        pPluginList += strlen(pPluginList) + 1;
    }

    if (pluginsToLoad.empty())
        return;

    // now we need to gather their dependencies and sort them.
    vector<string> sortedList;
    map<string, set<string, ci_less>, ci_less> dependencies;

    // first populate the dependencies
    for (auto& plugin : pluginsToLoad)
    {
        PLUGINCONFIG config;
        if (GetPluginConfiguration(plugin.c_str(), &config))
        {
            auto& s = dependencies[plugin];
            auto& v = config.dependencies;
            copy(v.begin(), v.end(), inserter(s, s.end()));
        }
    }

    TopologicalSort(sortedList, dependencies, pluginsToLoad);

    for (auto& plugin : sortedList)
    {
        LoadMQ2PluginEx(plugin.c_str(), isCustom, false);
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

    LoadSavedMQ2Plugins("macroquest", false);
    LoadSavedMQ2Plugins("CustomPlugins", true);
}

VOID UnloadMQ2Plugins()
{
    CAutoLock Lock(&gPluginCS);

    if (g_plugins.empty())
        return;

    // now we need to gather their dependencies and sort them.
    vector<string> sortedList;
    map<string, set<string, ci_less>, ci_less> dependencies;
    set<string, ci_less> pluginsToUnload;

    // first populate the dependencies
    for (auto& plugin : g_plugins)
    {
        pluginsToUnload.insert(plugin->name);

        for (auto& dep : plugin->dependencies)
            dependencies[dep].insert(plugin->name);
    }

    TopologicalSort(sortedList, dependencies, pluginsToUnload);

    for (auto& plugin : sortedList)
    {
        auto p = FindPluginShared(plugin.c_str());
        CleanupMQ2Plugin(p.get());

        auto found = std::find(g_plugins.begin(), g_plugins.end(), p);
        g_plugins.erase(found);
    }

    g_plugins.clear();
}

VOID ShutdownMQ2Plugins()
{
    bPluginCS = 0;
    UnloadMQ2Plugins();

    Sleep(50); // fixes crash on Windows 7 (Vista too?) in RtlpWaitOnCriticalSection
    DeleteCriticalSection(&gPluginCS);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

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

static void OnIncomingChatHelper(IPlugin* plugin, PCHAR Line, DWORD Color, BOOL& Ret)
{
    Ret |= plugin->OnIncomingChat(Line, Color);
}

BOOL PluginsIncomingChat(PCHAR Line, DWORD Color)
{
    if (!bPluginCS)
        return 0;
    if (!Line[0])
        return 0;
    PluginDebug("PluginsIncomingChat()");

    BOOL Ret = 0;
    InvokePlugins(&OnIncomingChatHelper, Line, Color, Ret);

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

            DebugSpew("PluginsSetGameState(%s server)",EQADDR_SERVERNAME);
            if (PCHARINFO pCharInfo=GetCharInfo())
            {
                DebugSpew("PluginsSetGameState(%s name)",pCharInfo->Name);
                sprintf(szBuffer,"%s_%s",EQADDR_SERVERNAME,pCharInfo->Name);
                LoadCfgFile(szBuffer,false);
            }
            if (PCHARINFO2 pCharInfo2=GetCharInfo2())
            {
                DebugSpew("PluginsSetGameState(%d class)",pCharInfo2->Class);
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
        CharSelect = true;
        LoadCfgFile("CharSelect", false);
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
