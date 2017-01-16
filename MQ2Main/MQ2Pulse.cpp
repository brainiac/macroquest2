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
#if !defined(CINTERFACE)
#error /DCINTERFACE
#endif

#define DBG_SPEW


//#define DEBUG_TRY 1
#include "MQ2Main.h"
BOOL TurnNotDone = FALSE;
CRITICAL_SECTION gPulseCS;
#ifndef ISXEQ
BOOL DoNextCommand()
{
	if (!ppCharSpawn || !pCharSpawn) return FALSE;
	PSPAWNINFO pCharOrMount = NULL;
	PCHARINFO pCharInfo = GetCharInfo();
	PSPAWNINFO pChar = pCharOrMount = (PSPAWNINFO)pCharSpawn;
	if (pCharInfo && pCharInfo->pSpawn) pChar = pCharInfo->pSpawn;
	if ((!pChar) || (gZoning)/* || (gDelayZoning)*/) return FALSE;
	if (((gFaceAngle != 10000.0f) || (gLookAngle != 10000.0f)) && (TurnNotDone)) return FALSE;
	if (IsMouseWaiting()) return FALSE;
	if (gDelay && gDelayCondition[0])
	{
		CHAR szCond[MAX_STRING];
		strcpy_s(szCond, gDelayCondition);
		ParseMacroParameter(GetCharInfo()->pSpawn, szCond);
		DOUBLE Result;
		if (!Calculate(szCond, Result))
		{
			FatalError("Failed to parse /delay condition '%s', non-numeric encountered", szCond);
			return false;
		}
		if (Result != 0)
		{
			DebugSpewNoFile("/delay ending early, conditions met");
			gDelay = 0;
		}
	}
	if (!gDelay && !gMacroPause && (!gMQPauseOnChat || *EQADDR_NOTINCHATMODE) &&
		gMacroBlock && gMacroStack) {
		PMACROBLOCK tmpBlock = gMacroBlock;
		gMacroStack->Location = gMacroBlock;
#ifdef MQ2_PROFILING
		LARGE_INTEGER BeforeCommand;
		QueryPerformanceCounter(&BeforeCommand);
		PMACROBLOCK ThisMacroBlock = gMacroBlock;
#endif
		gMacroBlock->MacroCmd = 0;
		DoCommand(pChar, (PCHAR)gMacroBlock->Line.c_str());
		if (gMacroBlock) {
#ifdef MQ2_PROFILING
			LARGE_INTEGER AfterCommand;
			QueryPerformanceCounter(&AfterCommand);
			ThisMacroBlock->ExecutionCount++;
			ThisMacroBlock->ExecutionTime += AfterCommand.QuadPart - BeforeCommand.QuadPart;
#endif
			if (!gMacroBlock->pNext) {
				FatalError("Reached end of macro.");
			}
			else {
				// if the macro block changed and there was a /macro 
				// command don't bump the line 
				//if (gMacroBlock == tmpBlock || !gMacroBlock->MacroCmd) {
				gMacroBlock = gMacroBlock->pNext;
				//}
			}
		}
		return TRUE;
	}
	return FALSE;
}
#endif

void Pulse()
{
	if (!ppCharSpawn || !pCharSpawn) return;
	PSPAWNINFO pCharOrMount = NULL;
	PCHARINFO pCharInfo = GetCharInfo();
	PSPAWNINFO pChar = pCharOrMount = (PSPAWNINFO)pCharSpawn;
	if (pCharInfo && pCharInfo->pSpawn) pChar = pCharInfo->pSpawn;

	static WORD LastZone = -1;

	static PSPAWNINFO pCharOld = NULL;
	static FLOAT LastX = 0.0f;
	static FLOAT LastY = 0.0f;
	static ULONGLONG LastMoveTick = 0;
	static DWORD MapDelay = 0;

	// Drop out here if we're waiting for something.
	if (!pChar || gZoning /* || gDelayZoning*/) return;
	if (!pCharInfo) {
		//DebugSpew("Pulse: no charinfo returning\n");
		return;
	}

	if ((unsigned int)GetCharInfo()->charinfo_info & 0x80000000) return;

	if (pChar != pCharOld && WereWeZoning)
	{
		WereWeZoning = FALSE;
		pCharOld = pChar;
		gFaceAngle = 10000.0f;
		gLookAngle = 10000.0f;
		gbMoving = FALSE;
		LastX = pChar->X;
		LastY = pChar->Y;
		LastMoveTick = MQGetTickCount64();
		EnviroTarget.Name[0] = 0;
		pGroundTarget = 0;
		DoorEnviroTarget.Name[0] = 0;
		DoorEnviroTarget.DisplayedName[0] = 0;
		pDoorTarget = 0;

		// see if we're on a pvp server
		if (!_strnicmp(EQADDR_SERVERNAME, "tallon", 6) || !_strnicmp(EQADDR_SERVERNAME, "vallon", 6))
		{
			PVPServer = PVP_TEAM;
		}
		else if (!_strnicmp(EQADDR_SERVERNAME, "sullon", 6))
		{
			PVPServer = PVP_SULLON;
		}
		else if (!_strnicmp(EQADDR_SERVERNAME, "rallos", 6))
		{
			PVPServer = PVP_RALLOS;
		}
		else
			PVPServer = PVP_NONE;
		srand((unsigned int)time(NULL) + (unsigned int)GetCurrentProcessId()); // reseed
		Benchmark(bmPluginsOnZoned, PluginsZoned());
		
	}
	else if ((LastX != pChar->X) || (LastY != pChar->Y) || LastMoveTick>MQGetTickCount64() - 100) {
		if ((LastX != pChar->X) || (LastY != pChar->Y)) LastMoveTick = MQGetTickCount64();
		gbMoving = TRUE;
		LastX = pChar->X;
		LastY = pChar->Y;
	}
	else {
		gbMoving = FALSE;
	}
	if (!pTarget)
		gTargetbuffs = FALSE;

	if (gbDoAutoRun && pChar && pCharInfo) {
		gbDoAutoRun = FALSE;
#ifndef EMU
		InitKeyRings();
#endif
		CHAR szServerAndName[MAX_STRING] = { 0 };
		CHAR szAutoRun[MAX_STRING] = { 0 };
		PCHAR pAutoRun = szAutoRun;
		/* autorun for everyone */
		GetPrivateProfileString("AutoRun", "ALL", "", szAutoRun, MAX_STRING, gszINIFilename);
		while (pAutoRun[0] == ' ' || pAutoRun[0] == '\t') pAutoRun++;
		if (szAutoRun[0] != 0) DoCommand(pChar, pAutoRun);
		/* autorun for toon */
		ZeroMemory(szAutoRun, MAX_STRING); pAutoRun = szAutoRun;
		sprintf_s(szServerAndName, "%s.%s", EQADDR_SERVERNAME, pCharInfo->Name);
		GetPrivateProfileString("AutoRun", szServerAndName, "", szAutoRun, MAX_STRING, gszINIFilename);
		while (pAutoRun[0] == ' ' || pAutoRun[0] == '\t') pAutoRun++;
		if (szAutoRun[0] != 0) DoCommand(pChar, pAutoRun);
	}

	if ((gFaceAngle != 10000.0f) || (gLookAngle != 10000.0f)) {
		TurnNotDone = FALSE;
		if (gFaceAngle != 10000.0f) {
			if (abs((INT)(pCharOrMount->Heading - gFaceAngle)) < 10.0f) {
				pCharOrMount->Heading = (FLOAT)gFaceAngle;
				pCharOrMount->SpeedHeading = 0.0f;
				gFaceAngle = 10000.0f;
			}
			else {
				TurnNotDone = TRUE;
				DOUBLE c1 = pCharOrMount->Heading + 256.0f;
				DOUBLE c2 = gFaceAngle;
				if (c2<pChar->Heading) c2 += 512.0f;
				DOUBLE turn = (DOUBLE)(rand() % 200) / 10;
				if (c2<c1) {
					pCharOrMount->Heading += (FLOAT)turn;
					pCharOrMount->SpeedHeading = 12.0f;
					if (pCharOrMount->Heading >= 512.0f) pCharOrMount->Heading -= 512.0f;
				}
				else {
					pCharOrMount->Heading -= (FLOAT)turn;
					pCharOrMount->SpeedHeading = -12.0f;
					if (pCharOrMount->Heading<0.0f) pCharOrMount->Heading += 512.0f;
				}
			}
		}

		if (gLookAngle != 10000.0f) {
			if (abs((INT)(pChar->CameraAngle - gLookAngle)) < 5.0f) {
				pChar->CameraAngle = (FLOAT)gLookAngle;
				gLookAngle = 10000.0f;
				TurnNotDone = FALSE;
			}
			else {
				TurnNotDone = TRUE;
				FLOAT c1 = pChar->CameraAngle;
				FLOAT c2 = (FLOAT)gLookAngle;

				DOUBLE turn = (DOUBLE)(rand() % 200) / 20;
				if (c1<c2) {
					pChar->CameraAngle += (FLOAT)turn;
					if (pChar->CameraAngle >= 128.0f) pChar->CameraAngle -= 128.0f;
				}
				else {
					pChar->CameraAngle -= (FLOAT)turn;
					if (pChar->CameraAngle <= -128.0f) pChar->CameraAngle += 128.0f;
				}
			}
		}

		if (TurnNotDone) {
			bRunNextCommand = FALSE;
			IsMouseWaiting();
			return;
		}
	}
}


int Heartbeat()
{
	if (gbUnload) {
		return 1;
	}
	if (gbLoad) {
		gbLoad = FALSE;
		return 2;
	}

	static ULONGLONG LastGetTick = 0;
	static bool bFirstHeartBeat = true;
	static ULONGLONG TickDiff = 0;
	static fMQPulse pEQPlayNicePulse = NULL;
	static DWORD BeatCount = 0;

	ULONGLONG Tick = MQGetTickCount64();

	BeatCount++;

	if (bFirstHeartBeat)
	{
		LastGetTick = Tick;
		bFirstHeartBeat = false;
	}
	// This accounts for rollover
	TickDiff += (Tick - LastGetTick);
	LastGetTick = Tick;
#ifndef ISXEQ
	while (TickDiff >= 100) {
		TickDiff -= 100;
		if (gDelay>0) gDelay--;
		DropTimers();
	}
#endif
	if (!gStringTableFixed && pStringTable) // Please dont remove the second condition
	{
		FixStringTable();
		gStringTableFixed = TRUE;
	}

	DebugTry(int GameState = GetGameState());
	if (GameState != -1)
	{
		if ((DWORD)GameState != gGameState)
		{
			DebugSpew("GetGameState()=%d vs %d", GameState, gGameState);
			gGameState = GameState;
			DebugTry(Benchmark(bmPluginsSetGameState, PluginsSetGameState(GameState)));
		}
	}
	else
		return 0;
	DebugTry(UpdateMQ2SpawnSort());
#ifndef ISXEQ_LEGACY
#ifndef ISXEQ
	DebugTry(DrawHUD());
	//if (gGameState==GAMESTATE_INGAME && !bMouseLook && ScreenMode==3)
	//{
	//    DebugTry(pWndMgr->DrawCursor());
	//}
#endif
#endif

	bRunNextCommand = TRUE;
	DebugTry(Pulse());
#ifndef ISXEQ_LEGACY
#ifndef ISXEQ
	DebugTry(Benchmark(bmPluginsPulse, DebugTry(PulsePlugins())));
#endif
	if (pEQPlayNicePulse) {
		pEQPlayNicePulse();
	}
	else {
		HMODULE hmEQPlayNice;
		if (((BeatCount % 63) == 0) && (hmEQPlayNice = GetModuleHandle("EQPlayNice.dll"))) {
			if (pEQPlayNicePulse = (fMQPulse)GetProcAddress(hmEQPlayNice, "Compat_ProcessFrame"))
				pEQPlayNicePulse();
		}
	}
#endif
	DebugTry(ProcessPendingGroundItems());


	static bool ShownNews = false;
	if (gGameState == GAMESTATE_CHARSELECT && !ShownNews)
	{
		ShownNews = true;
		if (gCreateMQ2NewsWindow)
			CreateMQ2NewsWindow();
	}

#ifndef ISXEQ
	DWORD CurTurbo = 0;

	if (gDelayedCommands) {// delayed commands
		lockit lk(ghLockDelayCommand,"HeartBeat");
		DoCommand((PSPAWNINFO)pLocalPlayer, gDelayedCommands->szText);
		PCHATBUF pNext = gDelayedCommands->pNext;
		//HLOCAL hlret = LocalFree(gDelayedCommands);
		//if (hlret != 0) {
		//	MessageBox(NULL,"LocalFree Failed we have a memory leak in HeartBeat","tell eqmule",MB_SYSTEMMODAL|MB_OK);
		//}
		delete gDelayedCommands;
		gDelayedCommands = pNext;
	}
	while (bRunNextCommand) {
		if (!DoNextCommand())
			break;
		if (gbUnload)
			return 1;
		if (!gTurbo)
			break;//bRunNextCommand = FALSE;
		if (++CurTurbo>gMaxTurbo)
			break;//bRunNextCommand =   FALSE;
	}
	DoTimedCommands();
#endif
	return 0;
}

#ifndef ISXEQ_LEGACY
// *************************************************************************** 
// Function:    ProcessGameEvents 
// Description: Our ProcessGameEvents Hook
// *************************************************************************** 
BOOL Trampoline_ProcessGameEvents(VOID);
BOOL Detour_ProcessGameEvents(VOID)
{
	CAutoLock Lock(&gPulseCS);
	int ret = Heartbeat();
#ifdef ISXEQ
	if (!pISInterface->ScriptEngineActive())
		pISInterface->LavishScriptPulse();
#endif
	int ret2 =  Trampoline_ProcessGameEvents();
#ifndef ISXEQ
	if(ret==2 && bPluginCS==0) {
		OutputDebugString("I am loading in ProcessGameEvents");
		//we are loading stuff
		DWORD oldscreenmode = ScreenMode;
		ScreenMode = 3;
		InitializeMQ2Commands();
		InitializeMQ2Windows();
		MQ2MouseHooks(1);
		Sleep(100);
		InitializeMQ2KeyBinds();
		#ifndef ISXEQ
		InitializeMQ2Plugins();
		#endif
		ScreenMode = oldscreenmode;
		SetEvent(hLoadComplete);
	} else if(ret==1) {
		OutputDebugString("I am unloading in ProcessGameEvents");
		//we are unloading stuff
		DWORD oldscreenmode = ScreenMode;
		ScreenMode = 3;
		WriteChatColor(UnloadedString,USERCOLOR_DEFAULT);
		DebugSpewAlways("%s", UnloadedString);
		UnloadMQ2Plugins();
		MQ2Shutdown();
		DebugSpew("Shutdown completed");
		g_Loaded = FALSE;
		ScreenMode = oldscreenmode;
		SetEvent(hUnloadComplete);

	}
#endif
	return ret2;
}

DETOUR_TRAMPOLINE_EMPTY(BOOL Trampoline_ProcessGameEvents(VOID));
class CEverQuestHook {
public:
	VOID EnterZone_Trampoline(PVOID pVoid);
	VOID EnterZone_Detour(PVOID pVoid)
	{
		EnterZone_Trampoline(pVoid);
		gZoning = TRUE;
		WereWeZoning = TRUE;
	}

	VOID SetGameState_Trampoline(DWORD GameState);
	VOID SetGameState_Detour(DWORD GameState)
	{
		//        DebugSpew("SetGameState_Detour(%d)",GameState);
		SetGameState_Trampoline(GameState);
		Benchmark(bmPluginsSetGameState, PluginsSetGameState(GameState));
	}
	VOID CTargetWnd__RefreshTargetBuffs_Trampoline(PBYTE);
	VOID CTargetWnd__RefreshTargetBuffs_Detour(PBYTE buffer)
	{
		gTargetbuffs = FALSE;
		CTargetWnd__RefreshTargetBuffs_Trampoline(buffer);
		gTargetbuffs = TRUE;
		if (gbAssistComplete == 1) {
			gbAssistComplete = 2;
		}
	}
};

DETOUR_TRAMPOLINE_EMPTY(VOID CEverQuestHook::EnterZone_Trampoline(PVOID));
DETOUR_TRAMPOLINE_EMPTY(VOID CEverQuestHook::SetGameState_Trampoline(DWORD));
DETOUR_TRAMPOLINE_EMPTY(VOID CEverQuestHook::CTargetWnd__RefreshTargetBuffs_Trampoline(PBYTE));

void InitializeMQ2Pulse()
{
	DebugSpew("Initializing Pulse");
	if (!ghLockDelayCommand)
		ghLockDelayCommand = CreateMutex(NULL, FALSE, NULL);
	InitializeCriticalSection(&gPulseCS);
	EzDetourwName(ProcessGameEvents, Detour_ProcessGameEvents, Trampoline_ProcessGameEvents,"ProcessGameEvents");
	EzDetourwName(CEverQuest__EnterZone, &CEverQuestHook::EnterZone_Detour, &CEverQuestHook::EnterZone_Trampoline,"CEverQuest__EnterZone");
	EzDetourwName(CEverQuest__SetGameState, &CEverQuestHook::SetGameState_Detour, &CEverQuestHook::SetGameState_Trampoline,"CEverQuest__SetGameState");
	EzDetourwName(CTargetWnd__RefreshTargetBuffs, &CEverQuestHook::CTargetWnd__RefreshTargetBuffs_Detour, &CEverQuestHook::CTargetWnd__RefreshTargetBuffs_Trampoline,"CTargetWnd__RefreshTargetBuffs");
}
void ShutdownMQ2Pulse()
{
	EnterCriticalSection(&gPulseCS);
	RemoveDetour((DWORD)CTargetWnd__RefreshTargetBuffs);
	RemoveDetour((DWORD)ProcessGameEvents);
	RemoveDetour(CEverQuest__EnterZone);
	RemoveDetour(CEverQuest__SetGameState);
	LeaveCriticalSection(&gPulseCS);
	DeleteCriticalSection(&gPulseCS);
}
#endif