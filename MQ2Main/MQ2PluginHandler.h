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

#pragma once

struct PLUGINCONFIG
{
	string author;
	string description;
	string version;

	vector<string> dependencies;          // enforced by plugin loader

	PLUGINCONFIG();
};
typedef PLUGINCONFIG* PPLUGINCONFIG;


// basic interface for plugins
class IPlugin
{
public:
	virtual ~IPlugin() {}

	//----------------------------------------------------------------------------
	// notifications to the plugin. These all have the same behavior as the more
	// traditional plugin functions.

	// Called once, when the plugin is to initialize
	virtual void Initialize() = 0;

	// Called once, when the plugin is to shut down
	virtual void Shutdown() = 0;

	// Called once directly before shutdown of the ui system, and also
	// every time the game calls CDisplay::CleanGameUI()
	virtual void OnCleanUI() {}

	// Called once directly after the game ui is reloaded, after issuing /loadskin
	virtual void OnReloadUI() {}

	// Called every frame that the "HUD" is drawn -- e.g. net status / packet loss bar
	virtual void OnDrawHUD() {}

	// Called once directly after initialization, and then every time the gamestate changes.
	virtual void OnSetGameState(DWORD GameState) {}

	// This is called every time MQ pulses
	virtual void OnPulse() {}

	// This is called every time WriteChatColor is called by MQ2Main or any plugin,
	// IGNORING FILTERS, IF YOU NEED THEM MAKE SURE TO IMPLEMENT THEM. IF YOU DONT
	// CALL CEverQuest::dsp_chat MAKE SURE TO IMPLEMENT EVENTS HERE (for chat plugins)
	virtual DWORD OnWriteChatColor(PCHAR Line, DWORD Color, DWORD Filter) { return 0; }

	// This is called every time EQ shows a line of chat with CEverQuest::dsp_chat,
	// but after MQ filters and chat events are taken care of.
	virtual DWORD OnIncomingChat(PCHAR Line, DWORD Color) { return 0; }

	// This is called each time a spawn is added to a zone (inserted into EQ's list of spawns),
	// or for each existing spawn when a plugin first initializes
	// NOTE: When you zone, these will come BEFORE OnZoned
	virtual void OnAddSpawn(PSPAWNINFO pNewSpawn) {}

	// This is called each time a spawn is removed from a zone (removed from EQ's list of spawns).
	// It is NOT called for each existing spawn when a plugin shuts down.
	virtual void OnRemoveSpawn(PSPAWNINFO pSpawn) {}

	// This is called each time a ground item is added to a zone
	// or for each existing ground item when a plugin first initializes
	// NOTE: When you zone, these will come BEFORE OnZoned
	virtual void OnAddGroundItem(PGROUNDITEM pNewGroundItem) {}

	// This is called each time a ground item is removed from a zone.
	// It is NOT called for each existing ground item when a plugin shuts down.
	virtual void OnRemoveGroundItem(PGROUNDITEM pGroundItem) {}

	// Called after entering a new zone. This is an old API, prefer to use
	// OnBeginZone and OnEndZone
	virtual void OnZoned() {}

	// This called before you actually leave the zone. Your character info, zone info,
	// and spawn table info is still valid for the previous zone.
	virtual void OnBeginZone() {}

	// This is called immediately after zoning, where zone info, character info, etc
	// has already been updated by the server.
	virtual void OnEndZone() {}

	// This is called immediately after a plugin has been loaded. If checking names, make sure
	// to use a case insensitive comparison.
	virtual void OnPluginLoaded(const char* pluginName) {}

	// This is called immediately before a plugin is unloaded. If checking names, make sure
	// to use a case insensitive comparison.
	virtual void OnPluginUnloaded(const char* pluginName) {}

	// temp/internal: do not call this. This is here for the plugin system. TODO: Improve InvokePlugins
	// so this can be removed.
	void OnIncomingChatHelper(PCHAR Line, DWORD Color, BOOL& Ret)
	{
		Ret |= OnIncomingChat(Line, Color);
	}
};

// typedefs for functions that plugins can export
typedef VOID(__cdecl *fMQConfigurePlugin)(PPLUGINCONFIG);
typedef unique_ptr<IPlugin>(__cdecl *fMQPluginFactory)();

typedef DWORD(__cdecl *fMQWriteChatColor)(PCHAR Line, DWORD Color, DWORD Filter);
typedef VOID(__cdecl *fMQPulse)(VOID);
typedef DWORD(__cdecl *fMQIncomingChat)(PCHAR Line, DWORD Color);
typedef VOID(__cdecl *fMQInitializePlugin)(VOID);
typedef VOID(__cdecl *fMQShutdownPlugin)(VOID);
typedef VOID(__cdecl *fMQZoned)(VOID);
typedef VOID(__cdecl *fMQReloadUI)(VOID);
typedef VOID(__cdecl *fMQCleanUI)(VOID);
typedef VOID(__cdecl *fMQDrawHUD)(VOID);
typedef VOID(__cdecl *fMQSetGameState)(DWORD GameState);
typedef VOID(__cdecl *fMQSpawn)(PSPAWNINFO);
typedef VOID(__cdecl *fMQGroundItem)(PGROUNDITEM);
typedef VOID(__cdecl *fMQBeginZone)(VOID);
typedef VOID(__cdecl *fMQEndZone)(VOID);


// A class that manages information about a single plugin.
class MQ2PluginInfo : public PLUGINCONFIG
{
public:
	MQ2PluginInfo(const char* filename, HMODULE module = 0, bool custom = false);
	~MQ2PluginInfo();

	// all the fields of the PLUGINCONFIG struct are accessible here

	unique_ptr<IPlugin> plugin;  // pointer to the plugin object
	HMODULE hModule;             // handle to the dll

	string name;                 // name of the plugin (ex: "MQ2ChatWnd")
	bool custom;                 // true if loaded from CustomPlugins.ini

	bool loaded;                 // indicates if this plugin is actually loaded
	float fpVersion;             // old: this is set if you export float MQ2Version; you should
	                             // prefer to use the new string version in PLUGINCONFIG

	bool removed;                // internal: for tracking removal of plugins during iteration
};
