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

namespace MQ2Prototypes
{
	typedef PCHAR(__stdcall *fEQGetStringByID)(DWORD);
#ifndef ISXEQ
	typedef HRESULT(__stdcall *fGetDeviceState)(THIS_ DWORD, LPVOID);
#endif
	typedef DWORD(__stdcall *fEQScreenItem)(DWORD, DWORD, DWORD);
	typedef DWORD(__stdcall *fEQScreenSpawn)(DWORD, DWORD);
	typedef PCHAR(__stdcall *fEQNewUIINI)(VOID);
	typedef VOID(__cdecl *fEQCommand)(PSPAWNINFO, PCHAR);
	typedef VOID(__cdecl *fEQMemSpell)(DWORD, DWORD);
	typedef VOID(__cdecl *fEQLoadSpells)(PSPELLFAVORITE, DWORD);
	typedef VOID(__cdecl *fEQSelectItem)(int, DWORD);//public: void __thiscall CMerchantWnd::SelectBuySellSlot(int,class CTextureAnimation *)
	typedef VOID(__cdecl *fEQBuyItem)(int);//private: void __thiscall CMerchantWnd::RequestBuyItem(int)
	typedef VOID(__cdecl *fEQSellItem)(int);//private: void __thiscall CMerchantWnd::RequestSellItem(int)
	typedef VOID(__cdecl *fEQWriteMapfile)(PCHAR, int);//void __thiscall ZZZ::WriteMapfile(PCHAR zonename, int Layer);
	typedef BOOL(__cdecl *fEQProcGameEvts)(VOID);
	typedef FLOAT(__cdecl *fEQGetMelee)(class EQPlayer *, class EQPlayer *);
	typedef BOOL(__cdecl *fEQExecuteCmd)(DWORD, BOOL, PVOID);
	typedef VOID(__cdecl *fMQExecuteCmd)(PCHAR Name, BOOL Down);
	typedef VOID(__cdecl fEQSaveToUIIniFile)(PCHAR Section, PCHAR Key, PCHAR Value);
	typedef DWORD(__cdecl *fMQParm)(PCHAR, PCHAR, PSPAWNINFO);
	typedef bool(__cdecl *fGetLabelFromEQ)(int, class CXStr *, bool *, unsigned long *);
#ifndef EMU
	typedef BOOL(__cdecl *fEQToggleKeyRingItem)(BOOL RingType,PCONTENTS*itemptr, DWORD listindex);//0 is mounts and 1 is illusions
#endif

	/* PLUGINS */
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
};
using namespace MQ2Prototypes;


enum PLUGIN_CAPABILITIES
{
	PLUGIN_WANTS_PULSE = 0x0001,
	PLUGIN_WANTS_DRAWHUD = 0x0002,
	PLUGIN_WANTS_SPAWNS = 0x0004,
	PLUGIN_WANTS_GROUNDITEMS = 0x0008,

	PLUGIN_CAPABILITIES_DEFAULTS = PLUGIN_WANTS_PULSE
	| PLUGIN_WANTS_DRAWHUD
	| PLUGIN_WANTS_SPAWNS
	| PLUGIN_WANTS_GROUNDITEMS
};

// basic interface for plugins
class IPlugin
{
public:
	virtual ~IPlugin() {}

	// needs to be a version quad or something of that sort
	virtual int GetVersion() const = 0;

	// returns bitfield representing plugin capabilities
	virtual int GetCapabilities() const { return PLUGIN_CAPABILITIES_DEFAULTS; };

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
};

struct MQPLUGIN
{
	unique_ptr<IPlugin> plugin;

	char szFilename[MAX_PATH];
	HMODULE hModule;
	BOOL bCustom;
	int capabilities;

	// In new style plugins, this is always 1.0. use IPlugin::GetVersion instead
	float fpVersion;

	MQPLUGIN* pLast;
	MQPLUGIN* pNext;
};
typedef MQPLUGIN* PMQPLUGIN;

typedef unique_ptr<IPlugin>(__cdecl *fMQPluginFactory)();
