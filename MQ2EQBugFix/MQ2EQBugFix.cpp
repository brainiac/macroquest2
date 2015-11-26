// MQ2EQBugFix.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.

#include "../MQ2Plugin.h"

PreSetup("MQ2EQBugFix");

PLUGIN_API void ConfigurePlugin(PPLUGINCONFIG config)
{
	config->version = 1.1f;
	config->dependencies = {
		"MQ2ChatWnd",
		"MQ2IRC",
		"MQ2Labels"
	};
}

class CDisplay_Hook
{
public:
	int is_3dON_Trampoline();
	int is_3dON_Detour()
	{
		if (!this)
		{
			DebugSpew("MQ2EQBugFix::Crash avoided!");
			return 0;
		}
		return is_3dON_Trampoline();
	}
};
DETOUR_TRAMPOLINE_EMPTY(int CDisplay_Hook::is_3dON_Trampoline());

class MQ2EQBugFix : public IPlugin
{
public:
	virtual void Initialize() override
	{
		DebugSpewAlways("Initializing MQ2EQBugFix");
		EzDetour(CDisplay__is3dON, &CDisplay_Hook::is_3dON_Detour, &CDisplay_Hook::is_3dON_Trampoline);
	}

	virtual void Shutdown() override
	{
		DebugSpewAlways("Shutting down MQ2EQBugFix");
		RemoveDetour(CDisplay__is3dON);
	}
};
DECLARE_PLUGIN_CLASS(MQ2EQBugFix);
