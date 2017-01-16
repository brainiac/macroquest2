// MQ2Test.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.


#include "../MQ2Plugin.h"

PreSetup("MQ2Test");

//----------------------------------------------------------------------------
// test the mq2 datatype extension code

class MQ2CharacterExtensionType* pCharExtType = nullptr;

class MQ2CharacterExtensionType : public MQ2Type
{
public:
	enum ExtensionMembers {
		Test1 = 1,
		Test2 = 2
	};

	MQ2CharacterExtensionType() : MQ2Type("MQ2TestCharacterExtension")
	{
		TypeMember(Test1);
		TypeMember(Test2);
	}

	bool GETMEMBER()
	{
		PMQ2TYPEMEMBER pMember = FindMember(Member);
		if (!pMember)
			return false;
		switch (pMember->ID) {
		case Test1:
			Dest.DWord = 1;
			Dest.Type = pIntType;
			return true;
		case Test2:
			Dest.DWord = 2;
			Dest.Type = pIntType;
			return true;
		}

		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination)
	{
		return false;
	}

	bool FromData(MQ2VARPTR& VarPtr, MQ2TYPEVAR& Source)
	{
		if (Source.Type != pCharExtType)
			return false;
		VarPtr.Ptr = Source.Ptr;
		return true;
	}

	bool FromString(MQ2VARPTR& VarPtr, PCHAR Source)
	{
		return false;
	}
};

bool DoArrayClassTests();

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	DebugSpewAlways("Initializing MQ2Test");

	pCharExtType = new MQ2CharacterExtensionType;
	AddMQ2TypeExtension("character", pCharExtType);

	if (!DoArrayClassTests())
		WriteChatf("Array Tests Failed");
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	DebugSpewAlways("Shutting down MQ2Test");

	RemoveMQ2TypeExtension("character", pCharExtType);
	delete pCharExtType;
}
