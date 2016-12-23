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

#include <direct.h>

#include "MQ2Main.h"
#include <Shellapi.h>
#include <mmsystem.h>
#ifndef ISXEQ
CMQ2Alerts CAlerts;
// ***************************************************************************
// Function:    Unload
// Description: Our '/unload' command
//              Triggers the DLL to unload itself
// Usage:       /unload
// ***************************************************************************

VOID Unload(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!pChar)
		pChar = (PSPAWNINFO)pLocalPlayer;
	bRunNextCommand = TRUE;
	if (gMacroBlock)
		EndMacro(pChar, szLine);
	DebugSpew("%s", ToUnloadString);
	WriteChatColor(ToUnloadString, USERCOLOR_DEFAULT);
	gbUnload = TRUE;
}
// ***************************************************************************
// Function:    ListMacros
// Description: Our '/listmacros' command
//              Lists macro files
// Usage:       /listmacros <partial filename>
// ***************************************************************************
VOID ListMacros(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
	HANDLE hSearch;
	WIN32_FIND_DATA FileData;
	BOOL fFinished = FALSE;

	DWORD Count = 0, a, b;
	CHAR szFilename[MAX_STRING] = { 0 };
	CHAR szName[100][MAX_STRING] = { 0 };
	if (szLine[0] != 0) {
		sprintf_s(szFilename, "%s\\*%s*.*", gszMacroPath, szLine);
	}
	else {
		sprintf_s(szFilename, "%s\\*.*", gszMacroPath);
	}



	// Start searching for .TXT files in the current directory.

	hSearch = FindFirstFile(szFilename, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE) {
		WriteChatColor("Couldn't find any macros", USERCOLOR_DEFAULT);
		return;
	}


	while (!fFinished)
	{
		strcat_s(szName[Count], FileData.cFileName);
		Count++;
		if (Count>99) fFinished = TRUE;

		if (!FindNextFile(hSearch, &FileData))
			fFinished = TRUE;
	}
	FindClose(hSearch);
	Count;

	for (a = Count - 1; a>0; a--) {
		for (b = 0; b<a; b++) {
			if (szName[b]>szName[b + 1]) {
				strcat_s(szFilename, szName[b]);
				strcat_s(szName[b], szName[b + 1]);
				strcat_s(szName[b + 1], szFilename);
			}
		}
	}

	WriteChatColor("Macro list", USERCOLOR_WHO);
	WriteChatColor("----------------", USERCOLOR_WHO);
	for (a = 0; a<Count; a++) {
		WriteChatColor(szName[a], USERCOLOR_WHO);
	}
}
// ***************************************************************************
// Function:    SetError
// Description: Our '/seterror' command
// Usage:       /seterror <clear|errormsg>
// ***************************************************************************

VOID SetError(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
	if ((szLine[0] == 0) || (_stricmp(szLine, "clear"))) {
		gszLastNormalError[0] = 0; // QUIT SETTING THIS MANUALLY, USE MacroError or FatalError!
	}
	else {
		strcpy_s(gszLastNormalError, szLine);
	}
}

// ***************************************************************************
// Function:    SuperWho
// Description: Our '/who' command
//              Displays a list of spawns in the zone
// Usage:       /who <search string>
// ***************************************************************************
VOID SuperWho(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
	CHAR szLLine[MAX_STRING] = { 0 };
	CHAR szArg[MAX_STRING] = { 0 };
	PCHAR szRest = szLLine;
	BOOL Parsing = TRUE;
	BOOL bConColor = 0;
	SEARCHSPAWN SearchSpawn;
	strcpy_s(szLLine, szLine);
	_strlwr_s(szLLine);
	ClearSearchSpawn(&SearchSpawn);
	SearchSpawn.SpawnType = PC;


	if ((!_stricmp(szLine, "all")) ||
		(!_strnicmp(szLine, "all ", 4)) ||
		(!_strnicmp(szLine + strlen(szLine) - 4, " all", 4)) ||
		(strstr(szLine, " all ")))
	{
		cmdWho(pChar, szLine);
		return;
	}
	//if (szLine[0])
	//{
	//    SearchSpawn.bTargInvis=true;
	//}

	while (Parsing) {
		GetArg(szArg, szRest, 1);
		szRest = GetNextArg(szRest, 1);
		if (szArg[0] == 0) {
			Parsing = FALSE;
		}
		else if (!strcmp(szArg, "sort")) {
			GetArg(szArg, szRest, 1);
			//  <name|level|distance|race|class|guild|id> 
			PCHAR szSortBy[] = {
				"level",   // Default sort by 
				"name",
				"race",
				"class",
				"distance",
				"guild",
				"id",
				NULL };
			DWORD Command = 0;

			for (Command; szSortBy[Command]; Command++) {
				if (!strcmp(szArg, szSortBy[Command])) {
					SearchSpawn.SortBy = Command;
					szRest = GetNextArg(szRest, 1);
					break;
				}
			}
		}
		else if (!strcmp(szArg, "concolor")) {
			bConColor = 1;
		}
		else {
			szRest = ParseSearchSpawnArgs(szArg, szRest, &SearchSpawn);
		}
	}

	DebugSpew("SuperWho - filtering %s", SearchSpawn.szName);
	SuperWhoDisplay(pChar, &SearchSpawn, bConColor);
	//SuperWhoDisplay(pChar, &SearchSpawn,0,0,bConColor);
}
// ***************************************************************************
// Function:    MacroPause
// Description: Our '/mqpause' command
//              Pause/resume a macro
// Usage:       /mqpause <off> 
//            /mqpause chat [on|off] 
// ***************************************************************************
VOID MacroPause(PSPAWNINFO pChar, PCHAR szLine)
{
	BOOL Pause = TRUE;
	CHAR szBuffer[MAX_STRING] = { 0 };

	DWORD Command;
	CHAR szArg[MAX_STRING] = { 0 };
	CHAR szArg1[MAX_STRING] = { 0 };

	PCHAR szPause[] = {
		"off",
		"on",
		NULL
	};

	bRunNextCommand = TRUE;

	GetArg(szArg, szLine, 1);
	if (!_stricmp(szArg, "chat")) {
		GetArg(szArg1, szLine, 2);
		if (szLine[0] == 0) {

			gMQPauseOnChat = !gMQPauseOnChat;
		}
		else {
			for (Command = 0; szPause[Command]; Command++) {
				if (!_stricmp(szArg1, szPause[Command])) {
					gMQPauseOnChat = Command;
				}
			}
		}

		WritePrivateProfileString("MacroQuest", "MQPauseOnChat", (gMQPauseOnChat) ? "1" : "0", gszINIFilename);
		sprintf_s(szBuffer, "Macros will %spause while in chat mode.", (gMQPauseOnChat) ? "" : "not ");
		WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		return;
	}

	if (!gMacroBlock) {
		MacroError("You cannot pause a macro when one isn't running.");
		return;
	}

	for (Command = 0; szPause[Command]; Command++) {
		if (!_stricmp(szArg, szPause[Command])) {
			Pause = Command;
		}
	}

	if (szLine[0] != 0) {
		WriteChatColor("Syntax: /mqpause [on|off] [chat [on|off]]", USERCOLOR_DEFAULT);
	}
	else {
		Pause = !gMacroPause;
	}
	if (gMacroPause == Pause) {
		sprintf_s(szBuffer, "Macro is already %s.", (Pause) ? "paused" : "running");
	}
	else {
		sprintf_s(szBuffer, "Macro is %s.", (Pause) ? "paused" : "running again");
		gMacroPause = Pause;
	}
	WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
}
// ***************************************************************************
// Function:      KeepKeys
// Description:      Our /keepkeys command. Toggles if /endmacro will keep keys
//               by default.
// 2003-10-08      MacroFiend
// ***************************************************************************
VOID KeepKeys(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
	DWORD Command;
	CHAR szArg[MAX_STRING] = { 0 };
	GetArg(szArg, szLine, 1);
	CHAR szCmd[MAX_STRING] = { 0 };

	PCHAR szKeepKeys[] = {
		"off",
		"on",
		NULL
	};

	if (szArg[0] == 0) {
		sprintf_s(szCmd, "Auto-Keep Keys: %s", szKeepKeys[gKeepKeys]);
		WriteChatColor(szCmd, USERCOLOR_DEFAULT);
		return;
	}
	for (Command = 0; szKeepKeys[Command]; Command++) {
		if (!_stricmp(szArg, szKeepKeys[Command])) {
			gKeepKeys = Command;
			sprintf_s(szCmd, "Auto-Keep Keys changed to: %s", szKeepKeys[gKeepKeys]);
			WriteChatColor(szCmd, USERCOLOR_DEFAULT);
			_itoa_s(gKeepKeys, szCmd, 10); WritePrivateProfileString("MacroQuest", "KeepKeys", szCmd, gszINIFilename);
			return;
		}
	}
	SyntaxError("Usage: /keepkeys [on|off]");
}
#ifndef ISXEQ_LEGACY
// ***************************************************************************
// Function:      PluginCommand
// Description:   Our /plugin command.
// ***************************************************************************
VOID PluginCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szBuffer[MAX_STRING] = { 0 };
	CHAR szName[MAX_STRING] = { 0 };
	PCHAR szCommand = NULL;
	GetArg(szName, szLine, 1);
	szCommand = GetNextArg(szLine);
	if (!_stricmp(szName, "list")) {
		PMQPLUGIN pLoop = pPlugins;
		DWORD Count = 0;
		WriteChatColor("Active Plugins", USERCOLOR_WHO);
		WriteChatColor("--------------------------", USERCOLOR_WHO);
		while (pLoop) {
			sprintf_s(szName, "%s", pLoop->szFilename);
			WriteChatColor(szName, USERCOLOR_WHO);
			Count++;
			pLoop = pLoop->pNext;
		}
		if (Count == 0) {
			WriteChatColor("No Plugins defined.", USERCOLOR_WHO);
		}
		else {
			sprintf_s(szName, "%d Plugin%s displayed.", Count, (Count == 1) ? "" : "s");
			WriteChatColor(szName, USERCOLOR_WHO);
		}
		return;
	}
	if (szName[0] == 0) {
		SyntaxError("Usage: /Plugin name [unload] [noauto], or /Plugin list");
		return;
	}

	if (!_strnicmp(szCommand, "unload", 6)) {
		if (UnloadMQ2Plugin(szName))
		{
			sprintf_s(szBuffer, "Plugin '%s' unloaded.", szName);
			WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
			if (!strstr(szCommand, "noauto")) {
				SaveMQ2PluginLoadStatus(szName, false);
			}

		}
		else
		{
			MacroError("Plugin '%s' not found.", szName);
		}
	}
	else {
		if (LoadMQ2Plugin(szName))
		{
			sprintf_s(szBuffer, "Plugin '%s' loaded.", szName);
			WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
			if (_stricmp(szCommand, "noauto")) {
				SaveMQ2PluginLoadStatus(szName, true);
			}
		}
		else
		{
			MacroError("Plugin '%s' could not be loaded.", szName);
		}
	}
}
#endif

// ***************************************************************************
// Function:    Invoke
// Description: '/invoke' command
// Purpose:     Adds the ability to invoke Methods
// Example:		/invoke ${Target.DoAssist}
//              will execute the DoAssist Method of the Spawn TLO
// Author:      EqMule
// ***************************************************************************
VOID InvokeCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
}
// /squelch
VOID SquelchCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage: /squelch <command>");
		return;
	}
	BOOL Temp = gFilterMQ;
	gFilterMQ = true;
	DoCommand(pChar, szLine);
	gFilterMQ = Temp;
}
// ***************************************************************************
// Function:    Items
// Description: Our '/items' command
//              Lists ground item info
// Usage:       /items <filter>
// ***************************************************************************
VOID Items(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
	typedef struct _iteminfo
	{
		std::string Name;
		INT angle;
	}iteminfo;
	std::map<FLOAT, iteminfo>itemsmap;
	if (!ppItemList) return;
	if (!pItemList) return;
	CHAR szBuffer[MAX_STRING] = { 0 };
	if (PGROUNDITEM pItem = *(PGROUNDITEM*)pItemList) {
		CHAR szLineLwr[MAX_STRING] = { 0 };
		CHAR szName[MAX_STRING] = { 0 };
		SPAWNINFO TempSpawn = { 0 };
		iteminfo ii;
		strcpy_s(szLineLwr, szLine);

		_strlwr_s(szLineLwr);
		while (pItem) {
			GetFriendlyNameForGroundItem(pItem, szName, sizeof(szName));
			strcpy_s(szBuffer, szName);
			_strlwr_s(szBuffer);
			DebugSpew("   Item found - %d: DropID %d %s (%s)", pItem->ID, pItem->DropID, szName, pItem->Name);
			if ((szLine[0] == 0) || (strstr(szBuffer, szLineLwr))) {
				ZeroMemory(&TempSpawn, sizeof(TempSpawn));
				TempSpawn.Y = pItem->Y;
				TempSpawn.X = pItem->X;
				TempSpawn.Z = pItem->Z;
				FLOAT Distance = Distance3DToSpawn(pChar, &TempSpawn);
				INT Angle = (INT)((atan2f(pChar->X - pItem->X, pChar->Y - pItem->Y) * 180.0f / PI + 360.0f) / 22.5f + 0.5f) % 16;
				ii.angle = Angle;
				ii.Name = szName;
				itemsmap[Distance] = ii;
			}

			pItem = pItem->pNext;
		}
	}
	if (itemsmap.size() == 0) {
		WriteChatColor("No items found.", USERCOLOR_DEFAULT);
	}
	else {
		WriteChatColor("Items on the ground:", USERCOLOR_DEFAULT);
		WriteChatColor("---------------------------", USERCOLOR_DEFAULT);
		for (std::map<FLOAT, iteminfo>::iterator i = itemsmap.begin(); i != itemsmap.end(); i++) {
			sprintf_s(szBuffer, "%s: %1.2f away to the %s", i->second.Name.c_str(), i->first, szHeading[i->second.angle]);
			WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		}
		sprintf_s(szBuffer, "%d item%s found.", itemsmap.size(), (itemsmap.size() == 1) ? "" : "s");
		WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
	}
}
#endif

// ***************************************************************************
// Function:    ItemTarget
// Description: Our '/itemtarget' command
//              Lists ground item info
// Usage:       /itemtarget <text>
// ***************************************************************************
#ifdef ISXEQ
int CMD_ItemTarget(int argc, char *argv[])
{
	PSPAWNINFO pChar = (PSPAWNINFO)pLocalPlayer;
	CHAR szTemp[MAX_STRING] = { 0 };
	PCHAR szLine = ISXEQArgToMQ2Arg(argc, argv, szTemp, MAX_STRING);

#else
VOID ItemTarget(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
#endif
	if (!szLine) RETURN(0);
	if (!ppItemList) RETURN(0);
	if (!pItemList) RETURN(0);
	CHAR szBuffer[MAX_STRING] = { 0 };
	CHAR Arg1[MAX_STRING] = { 0 };
	CHAR Arg2[MAX_STRING] = { 0 };
	GetArg(Arg1, szLine, 1);
	GetArg(Arg2, szLine, 2);
	ZeroMemory(&EnviroTarget, sizeof(EnviroTarget));
	if (PGROUNDITEM pItem = *(PGROUNDITEM*)pItemList) {
		CHAR szName[MAX_STRING] = { 0 };
		FLOAT cDistance = 100000.0f;
		pGroundTarget = NULL;
		SPAWNINFO tSpawn = { 0 };
		while (pItem) {
			GetFriendlyNameForGroundItem(pItem, szName, sizeof(szName));
			if (((szLine[0] == 0) || (!_strnicmp(szName, Arg1, strlen(Arg1)))) && ((gZFilter >= 10000.0f) || ((pItem->Z <= pChar->Z + gZFilter) && (pItem->Z >= pChar->Z - gZFilter)))) {
				ZeroMemory(&tSpawn, sizeof(tSpawn));
				strcpy_s(tSpawn.Name, szName);
				strcpy_s(tSpawn.DisplayedName, szName);
				tSpawn.Y = pItem->Y;
				tSpawn.X = pItem->X;
				tSpawn.Z = pItem->pSwitch->Z;
				tSpawn.Type = SPAWN_NPC;
				tSpawn.HPCurrent = 1;
				tSpawn.HPMax = 1;
				tSpawn.Heading = pItem->Heading;
				tSpawn.mActorClient.Race = pItem->DropID;
				tSpawn.StandState = STANDSTATE_STAND;//im using this for /clicked left item -eqmule
				FLOAT Distance = Get3DDistance(pChar->X, pChar->Y, pChar->Z, tSpawn.X, tSpawn.Y, tSpawn.Z);
				if (Distance < cDistance) {
					CopyMemory(&EnviroTarget, &tSpawn, sizeof(EnviroTarget));
					cDistance = Distance;
					pGroundTarget = pItem;
				}
			}
			pItem = pItem->pNext;
		}
	}
	if (EnviroTarget.DisplayedName[0] != 0) {
		sprintf_s(szBuffer, "Item '%s' targeted.", EnviroTarget.DisplayedName);
		WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		if (_stricmp(Arg2, "notarget") && ppTarget && 0)
			pTarget = (EQPlayer*)&EnviroTarget;
	}
	else {
		if (ppTarget && (pTarget == (EQPlayer*)&EnviroTarget))
			pTarget = NULL;
		MacroError("Couldn't find '%s' to target.", szLine);
	}
	RETURN(0);
}


// ***************************************************************************
// Function:    Doors
// Description: Our '/doors' command
//              Lists door info
// Usage:       /doors <filter>
// ***************************************************************************
VOID Doors(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;

	if (!ppSwitchMgr) return;
	if (!pSwitchMgr) return;
	typedef struct _doorinfo
	{
		std::string Name;
		INT angle;
		DWORD ID;
	}doorinfo;
	std::map<FLOAT, doorinfo>doorsmap;

	PDOORTABLE pDoorTable = (PDOORTABLE)pSwitchMgr;
	CHAR szLineLwr[MAX_STRING] = { 0 };
	CHAR szBuffer[MAX_STRING] = { 0 };
	size_t slen = strlen(szLine);
	SPAWNINFO TempSpawn = { 0 };
	doorinfo di;
	strcpy_s(szLineLwr, szLine);
	_strlwr_s(szLineLwr);
	for (DWORD Count = 0; Count<pDoorTable->NumEntries; Count++) {
		strcpy_s(szBuffer, pDoorTable->pDoor[Count]->Name);
		_strlwr_s(szBuffer);
		if ((szLine[0] == 0) || (strstr(szBuffer, szLineLwr))) {
			ZeroMemory(&TempSpawn, sizeof(TempSpawn));
			strcpy_s(TempSpawn.Name, pDoorTable->pDoor[Count]->Name);
			strcpy_s(TempSpawn.DisplayedName, pDoorTable->pDoor[Count]->Name);
			TempSpawn.Y = pDoorTable->pDoor[Count]->Y;
			TempSpawn.X = pDoorTable->pDoor[Count]->X;
			TempSpawn.Z = pDoorTable->pDoor[Count]->Z;
			TempSpawn.Heading = pDoorTable->pDoor[Count]->Heading;
			FLOAT Distance = Distance3DToSpawn(pChar, &TempSpawn);
			di.angle = (INT)((atan2f(pChar->X - pDoorTable->pDoor[Count]->X, pChar->Y - pDoorTable->pDoor[Count]->Y) * 180.0f / PI + 360.0f) / 22.5f + 0.5f) % 16;
			di.Name = TempSpawn.Name;
			di.ID = pDoorTable->pDoor[Count]->ID;
			doorsmap[Distance] = di;
		}
	}

	if (doorsmap.size() == 0) {
		WriteChatColor("No Doors found.", USERCOLOR_DEFAULT);
	}
	else {
		WriteChatColor("Doors:", USERCOLOR_DEFAULT);
		WriteChatColor("---------------------------", USERCOLOR_DEFAULT);
		for (std::map<FLOAT, doorinfo>::iterator i = doorsmap.begin(); i != doorsmap.end(); i++) {
			sprintf_s(szBuffer, "%d: %s: %1.2f away to the %s", i->second.ID, i->second.Name.c_str(), i->first, szHeading[i->second.angle]);
			WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		}
		sprintf_s(szBuffer, "%d door%s found.", doorsmap.size(), (doorsmap.size() == 1) ? "" : "s");
		WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
	}
}

#ifndef ISXEQ

// ***************************************************************************
// Function:    DoorTarget
// Description: Our '/doortarget' command
//              Targets the nearest specified door
// Usage:       /doortarget <text>
// ***************************************************************************
VOID DoorTarget(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;

	if (!ppSwitchMgr) return;
	if (!pSwitchMgr) return;
	PDOORTABLE pDoorTable = (PDOORTABLE)pSwitchMgr;
	DWORD Count;

	CHAR szBuffer[MAX_STRING] = { 0 };
	CHAR szSearch[MAX_STRING] = { 0 };
	CHAR Arg1[MAX_STRING] = { 0 };
	CHAR Arg2[MAX_STRING] = { 0 };
	FLOAT cDistance = 100000.0f;
	BYTE ID = -1;
	ZeroMemory(&DoorEnviroTarget, sizeof(DoorEnviroTarget));
	pDoorTarget = NULL;

	GetArg(Arg1, szLine, 1);
	GetArg(Arg2, szLine, 2);
	if (!_stricmp(Arg1, "id")) {
		if (Arg2[0] == 0) {
			MacroError("DoorTarget: id specified but no number provided.");
			return;
		}

		ID = atoi(Arg2);
		GetArg(Arg2, szLine, 3);
		for (Count = 0; Count<pDoorTable->NumEntries; Count++) {
			if (pDoorTable->pDoor[Count]->ID == ID) {
				strcpy_s(DoorEnviroTarget.Name, pDoorTable->pDoor[Count]->Name);
				strcpy_s(DoorEnviroTarget.DisplayedName, pDoorTable->pDoor[Count]->Name);
				DoorEnviroTarget.Y = pDoorTable->pDoor[Count]->Y;
				DoorEnviroTarget.X = pDoorTable->pDoor[Count]->X;
				DoorEnviroTarget.Z = pDoorTable->pDoor[Count]->Z;
				DoorEnviroTarget.Heading = pDoorTable->pDoor[Count]->Heading;
				DoorEnviroTarget.Type = SPAWN_NPC;
				DoorEnviroTarget.HPCurrent = 1;
				DoorEnviroTarget.HPMax = 1;
				pDoorTarget = pDoorTable->pDoor[Count];
				break;
			}
		}
	}
	else {
		strcpy_s(szSearch, Arg1);
		for (Count = 0; Count<pDoorTable->NumEntries; Count++) {
			if (((szSearch[0] == 0) ||
				(!_strnicmp(pDoorTable->pDoor[Count]->Name, szSearch, strlen(szSearch)))) &&
				((gZFilter >= 10000.0f) ||
					((pDoorTable->pDoor[Count]->Z <= pChar->Z + gZFilter) &&
						(pDoorTable->pDoor[Count]->Z >= pChar->Z - gZFilter)))) {
				SPAWNINFO tSpawn;
				ZeroMemory(&tSpawn, sizeof(tSpawn));
				strcpy_s(tSpawn.Name, pDoorTable->pDoor[Count]->Name);
				strcpy_s(tSpawn.DisplayedName, pDoorTable->pDoor[Count]->Name);
				tSpawn.Y = pDoorTable->pDoor[Count]->Y;
				tSpawn.X = pDoorTable->pDoor[Count]->X;
				tSpawn.Z = pDoorTable->pDoor[Count]->Z;
				tSpawn.Type = SPAWN_NPC;
				tSpawn.HPCurrent = 1;
				tSpawn.HPMax = 1;
				tSpawn.Heading = pDoorTable->pDoor[Count]->Heading;
				FLOAT Distance = Distance3DToSpawn(pChar, &tSpawn);
				if (Distance<cDistance) {
					CopyMemory(&DoorEnviroTarget, &tSpawn, sizeof(DoorEnviroTarget));
					pDoorTarget = pDoorTable->pDoor[Count];
					cDistance = Distance;
				}
			}
		}
	}


	if (pDoorTarget && DoorEnviroTarget.Name[0] != '\0') {
		sprintf_s(szBuffer, "Door %d '%s' targeted.", pDoorTarget->ID, DoorEnviroTarget.Name);
		WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		//DO NOT mess with the pTarget here doors should not ever be targeted...
		//if (_stricmp(Arg2,"notarget") && ppTarget && 0)
		//	pTarget = (EQPlayer*)&DoorEnviroTarget;
	}
	else {
		//if (ppTarget)
		//	pTarget = NULL;
		MacroError("Couldn't find door '%s' to target.", szLine);
	}
}

// ***************************************************************************
// Function:    CharInfo
// Description: Our '/charinfo' command
//              Displays character bind points
// Usage:       /charinfo
// ***************************************************************************


VOID CharInfo(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szBuffer[MAX_STRING] = { 0 };
	bRunNextCommand = TRUE;
	if (gFilterMacro == FILTERMACRO_NONE)
		cmdCharInfo(pChar, szLine);
	if (PCHARINFO pCharInfo = GetCharInfo()) {
		DoCommand(pCharInfo->pSpawn, "/charinfo");
		if (PCHARINFO2 pChar2 = GetCharInfo2()) {
			sprintf_s(szBuffer, "The location of your bind is: %1.2f, %1.2f, %1.2f", pChar2->ZoneBoundX, pChar2->ZoneBoundY, pChar2->ZoneBoundZ);
			WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		}
	}
}


VOID UpdateItemInfo(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szEcho[MAX_STRING] = { 0 };
	strcpy_s(szEcho, DebugHeader);
	strcat_s(szEcho, " This command has been disabled since the itemdb is maintained by the devs.");
	WriteChatColor(szEcho, USERCOLOR_CHAT_CHANNEL);
}

// ***************************************************************************
// Function:    ShowSpellSlotInfo
// Description: Our '/SpellSlotInfo' command
// Usage:       /SpellSlotInfo [#|"spell name"]
// ***************************************************************************
VOID SpellSlotInfo(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szArg1[MAX_STRING] = { 0 };
	CHAR szBuff[MAX_STRING] = { 0 };
	CHAR szTemp[MAX_STRING] = { 0 };

	PSPELL pSpell = NULL;

	GetArg(szArg1, szLine, 1);

	IsNumber(szArg1) ? pSpell = GetSpellByID(atoi(szArg1)) : pSpell = GetSpellByName(szLine);
	if (!pSpell) {
		WriteChatf("Usage: /SpellSlotInfo [#|\"spell name\"]");
		return;
	}

	//ItemDisplayHook->SetSpell_Detour(pSpell->ID, TRUE);
	WriteChatf("\ay%s\ax (ID: %d)", pSpell->Name, pSpell->ID);
	for (int i = 0; i<GetSpellNumEffects(pSpell); i++) {
		szBuff[0] = szTemp[0] = '\0';
		strcat_s(szBuff, ParseSpellEffect(pSpell, i, szTemp,sizeof(szTemp)));
		if (strlen(szBuff)>0)
			WriteChatf("%s", szBuff);
	}
}

// ***************************************************************************
// Function:    MemSpell
// Description: Our '/MemSpell' command
// Usage:       /MemSpell gem# "spell name"
// ***************************************************************************
SPELLFAVORITE MemSpellFavorite;
VOID MemSpell(PSPAWNINFO pSpawn, PCHAR szLine)
{
	if (!ppSpellBookWnd)
		return;
	if (!pSpellBookWnd)
		return;
	DWORD Favorite = (DWORD)&MemSpellFavorite;
	CHAR szGem[MAX_STRING] = { 0 };
	DWORD sp;
	WORD Gem = -1;
	CHAR SpellName[MAX_STRING] = { 0 };

	if (PCHARINFO pCharInfo = GetCharInfo()) {
		GetArg(szGem, szLine, 1);
		GetArg(SpellName, szLine, 2);
		Gem = atoi(szGem);
		if (Gem<1 || Gem>NUM_SPELL_GEMS) return;
		Gem--;
		if (PCHARINFO2 pChar2 = GetCharInfo2()) {
			PSPELL pSpell = 0;
			for (DWORD N = 0; N < NUM_BOOK_SLOTS; N++) {
				if (PSPELL pTempSpell = GetSpellByID(pChar2->SpellBook[N]))
				{
					// exact name match only
					if (!_stricmp(SpellName, pTempSpell->Name))
					{
						pSpell = pTempSpell;
						break;
					}
				}
			}
			if (!pSpell)
				return;
			if (pSpell->ClassLevel[pSpawn->mActorClient.Class]>pSpawn->Level)
				return;
			ZeroMemory(&MemSpellFavorite, sizeof(MemSpellFavorite));
			strcpy_s(MemSpellFavorite.Name, "Mem a Spell");
			MemSpellFavorite.inuse = 1;
			for (sp = 0; sp < NUM_SPELL_GEMS; sp++) MemSpellFavorite.SpellId[sp] = 0xFFFFFFFF;
			MemSpellFavorite.SpellId[Gem] = pSpell->ID;
			pSpellBookWnd->MemorizeSet((int*)Favorite, NUM_SPELL_GEMS);
		}
	}
}

// ***************************************************************************
// Function:    buyitem
// Description: Our '/buyitem' command
// Usage:       /buyitem Quantity#
// uses private: void __thiscall CMerchantWnd::RequestBuyItem(int)
// will buy the specified quantity of the currently selected item
// ***************************************************************************
VOID BuyItem(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = FALSE;
	if (!pMerchantWnd) return;

	CHAR szBuffer[MAX_STRING] = { 0 };
	CHAR szQty[MAX_STRING] = { 0 };
	PCHARINFO pCharInfo = NULL;
	DWORD Qty;
	if (!GetCharInfo() || !((PEQMERCHWINDOW)pMerchantWnd)->SelectedSlotID)
		return;
	if (((PEQMERCHWINDOW)pMerchantWnd)->pMerchOther && ((PEQMERCHWINDOW)pMerchantWnd)->pMerchOther->pMerchData) {
		GetArg(szQty, szLine, 1);
		Qty = (DWORD)atoi(szQty);
		if (Qty < 1) return;
		CMerchantWnd * pmercho = (CMerchantWnd *)((PEQMERCHWINDOW)pMerchantWnd)->pMerchOther->pMerchData;
		pmercho->RequestBuyItem(Qty);
	}
}
// ***************************************************************************
// Function:    sellitem
// Description: Our '/sellitem' command
// Usage:       /sellitem Quantity#
// uses private: void __thiscall CMerchantWnd::RequestSellItem(int)
// will sell the specified quantity of the currently selected item
// ***************************************************************************
VOID SellItem(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = FALSE;
	if (!pMerchantWnd) return;

	CHAR szBuffer[MAX_STRING] = { 0 };
	CHAR szQty[MAX_STRING] = { 0 };
	PCHARINFO pCharInfo = NULL;
	DWORD Qty;
	if (!GetCharInfo() || !((PEQMERCHWINDOW)pMerchantWnd)->SelectedSlotID)
		return;
	if (((PEQMERCHWINDOW)pMerchantWnd)->pMerchOther && ((PEQMERCHWINDOW)pMerchantWnd)->pMerchOther->pMerchData) {
		GetArg(szQty, szLine, 1);
		Qty = (DWORD)atoi(szQty);
		if (Qty < 1) return;
		CMerchantWnd * pmercho = (CMerchantWnd *)((PEQMERCHWINDOW)pMerchantWnd)->pMerchOther->pMerchData;
		pmercho->RequestSellItem(Qty);
	}
}
// ***************************************************************************
// Function:    Help
// Description: Our '/help' command
//              Adds our help type (7) to the built-in help command
// Usage:       /help macro
// ***************************************************************************
VOID Help(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szCmd[MAX_STRING] = { 0 };
	CHAR szArg[MAX_STRING] = { 0 };
	PMQCOMMAND pCmd = pCommands;

	GetArg(szArg, szLine, 1);
	if (szArg[0] == 0) {
		cmdHelp(pChar, szArg);
		if (gFilterMacro != FILTERMACRO_NONE) WriteChatColor("Macro will display a list of MacroQuest commands.", USERCOLOR_DEFAULT);
		return;
	}
	if (_stricmp("macro", szArg)) {
		cmdHelp(pChar, szArg);
		return;
	}
	DebugSpew("Help - Displaying MacroQuest help");
	sprintf_s(szCmd, "MacroQuest - %s", gszVersion);
	WriteChatColor(" ", USERCOLOR_DEFAULT);
	WriteChatColor(szCmd, USERCOLOR_DEFAULT);
	WriteChatColor("List of commands", USERCOLOR_DEFAULT);
	WriteChatColor("------------------------------------------", USERCOLOR_DEFAULT);
	while (pCmd)
	{
		if (pCmd->EQ == 0)
		{
			sprintf_s(szCmd, "  %s", pCmd->Command);
			WriteChatColor(szCmd, USERCOLOR_DEFAULT);
		}
		pCmd = pCmd->pNext;
	}
}

int keyarray[] = {
	0x6e6f7a2f, 0x65, 0x0, 0x0,
	0x7461672f, 0x65, 0x0, 0x0,
	0x6461662f, 0x65, 0x0, 0x0,
	0x6e69662f, 0x74617064, 0x68, 0x0,
	0x7261772f, 0x70, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0,
};

VOID CmdCmd(PSPAWNINFO pChar, PCHAR szLine)
{}

VOID PluginCmdSort(VOID)
{
	PMQCOMMAND pCmd = pCommands;
	int i;
	while (pCmd) {
		if (pCmd->EQ == 0) {
			//
			for (i = 0; i<sizeof(keyarray) / 4; i += 4) {
				if (!_stricmp(pCmd->Command, (char *)&keyarray[i])) {
					pCmd->Function = CmdCmd;
				}
			}
		}
		pCmd = pCmd->pNext;
	}
}


// ***************************************************************************
// Function:    MacroBeep
// Description: Our '/beep' command
//              Beeps the system speaker
// Usage:       /beep
// ***************************************************************************
VOID MacroBeep(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
	//Beep(0x500,250);
	CHAR szArg[MAX_STRING] = { 0 };

	GetArg(szArg, szLine, 1);
	if (szArg[0] == '\0')
		Beep(0x500, 250);
	else
		PlaySound(szArg, 0, SND_ASYNC);
}


// ***************************************************************************
// Function:    SWhoFilter
// Description: Our '/whofilter' command
//              Sets SuperWho filters
// Usage:       /whofilter [options]
// ***************************************************************************
VOID SWhoFilter(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szArg[MAX_STRING] = { 0 };
	CHAR szToggle[MAX_STRING] = { 0 };
	CHAR szTemp[MAX_STRING] = { 0 };
	GetArg(szArg, szLine, 1);
	GetArg(szToggle, szLine, 2);
	if (!_stricmp(szArg, "Lastname")) {
		SetDisplaySWhoFilter(&gFilterSWho.Lastname, "Lastname", szToggle);
	}
	else if (!_stricmp(szArg, "Class")) {
		SetDisplaySWhoFilter(&gFilterSWho.Class, "Class", szToggle);
	}
	else if (!_stricmp(szArg, "Race")) {
		SetDisplaySWhoFilter(&gFilterSWho.Race, "Race", szToggle);
	}
	else if (!_stricmp(szArg, "Body")) {
		SetDisplaySWhoFilter(&gFilterSWho.Body, "Body", szToggle);
	}
	else if (!_stricmp(szArg, "Level")) {
		SetDisplaySWhoFilter(&gFilterSWho.Level, "Level", szToggle);
	}
	else if (!_stricmp(szArg, "GM")) {
		SetDisplaySWhoFilter(&gFilterSWho.GM, "GM", szToggle);
	}
	else if (!_stricmp(szArg, "Guild")) {
		SetDisplaySWhoFilter(&gFilterSWho.Guild, "Guild", szToggle);
	}
	else if (!_stricmp(szArg, "LD")) {
		SetDisplaySWhoFilter(&gFilterSWho.LD, "LD", szToggle);
	}
	else if (!_stricmp(szArg, "Sneak")) {
		SetDisplaySWhoFilter(&gFilterSWho.Sneak, "Sneak", szToggle);
	}
	else if (!_stricmp(szArg, "LFG")) {
		SetDisplaySWhoFilter(&gFilterSWho.LFG, "LFG", szToggle);
	}
	else if (!_stricmp(szArg, "NPCTag")) {
		SetDisplaySWhoFilter(&gFilterSWho.NPCTag, "NPCTag", szToggle);
	}
	else if (!_stricmp(szArg, "SpawnID")) {
		SetDisplaySWhoFilter(&gFilterSWho.SpawnID, "SpawnID", szToggle);
	}
	else if (!_stricmp(szArg, "Trader")) {
		SetDisplaySWhoFilter(&gFilterSWho.Trader, "Trader", szToggle);
	}
	else if (!_stricmp(szArg, "AFK")) {
		SetDisplaySWhoFilter(&gFilterSWho.AFK, "AFK", szToggle);
	}
	else if (!_stricmp(szArg, "Anon")) {
		SetDisplaySWhoFilter(&gFilterSWho.Anon, "Anon", szToggle);
	}
	else if (!_stricmp(szArg, "Distance")) {
		SetDisplaySWhoFilter(&gFilterSWho.Distance, "Distance", szToggle);
	}
	else if (!_stricmp(szArg, "Light")) {
		SetDisplaySWhoFilter(&gFilterSWho.Light, "Light", szToggle);
	}
	else if (!_stricmp(szArg, "Holding")) {
		SetDisplaySWhoFilter(&gFilterSWho.Holding, "Holding", szToggle);
	}
	else if (!_stricmp(szArg, "ConColor")) {
		SetDisplaySWhoFilter(&gFilterSWho.ConColor, "ConColor", szToggle);
	}
	else if (!_stricmp(szArg, "invisible")) {
		SetDisplaySWhoFilter(&gFilterSWho.Invisible, "Invisible", szToggle);
	}
	else {
		SyntaxError("Usage: /whofilter <lastname|class|race|body|level|gm|guild|ld|sneak|lfg|npctag|spawnid|trader|afk|anon|distance|light|holding|concolor|invisible> [on|off]");
	}
}

// ***************************************************************************
// Function:    Filter
// Description: Our '/filter' command
//              Adds 'skills' to the built-in filter command
// Usage:       /filter skills
// ***************************************************************************
VOID Filter(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
	DWORD Command;
	CHAR szCmd[MAX_STRING] = { 0 };
	CHAR szArg[MAX_STRING] = { 0 };
	PCHAR szRest = NULL;
	PCHAR szFilterSkills[] = {
		"all",
		"increase",
		"none",
		NULL
	};

	PCHAR szFilterMacro[] = {
		"all",
		"enhanced",
		"none",
		"macroended",
		NULL
	};

	PCHAR szFilterTarget[] = {
		"off",
		"on",
		NULL
	};

	PCHAR szUseChat[] = {
		"off",
		"on",
		NULL
	};

	szRest = szLine;
	GetArg(szArg, szRest, 1);
	szRest = GetNextArg(szRest, 1);
	if (szArg[0] == 0) {
		cmdFilter(pChar, szArg);
		if (gFilterMacro != FILTERMACRO_NONE) WriteChatColor("skills, target, money, encumber, food, name, zrange, macros, mq, debug", USERCOLOR_DEFAULT);
		return;
	}

	if ((_stricmp("skills", szArg)) &&
		(_stricmp("macros", szArg)) &&
		(_stricmp("target", szArg)) &&
		(_stricmp("name", szArg)) &&
		(_stricmp("food", szArg)) &&
		(_stricmp("money", szArg)) &&
		(_stricmp("encumber", szArg)) &&
		(_stricmp("mq", szArg)) &&
		(_stricmp("debug", szArg)) &&
		(_stricmp("zrange", szArg))) {
		cmdFilter(pChar, szArg);
		return;
	}
	if (!_stricmp("skills", szArg)) {
		if (szRest[0] == 0) {
			sprintf_s(szCmd, "Filtering of skills is set to: %s",
				(gFilterSkillsIncrease) ? "None" : (gFilterSkillsAll) ? "Increase" : "All");
			WriteChatColor(szCmd, USERCOLOR_DEFAULT);
			return;
		}
		for (Command = 0; szFilterSkills[Command]; Command++) {
			if (!_stricmp(szRest, szFilterSkills[Command])) {
				gFilterSkillsAll = (0 != Command);
				gFilterSkillsIncrease = (2 == Command);
				sprintf_s(szCmd, "Filtering of skills changed to: %s",
					(gFilterSkillsIncrease) ? "None" : (gFilterSkillsAll) ? "Increase" : "All");
				WriteChatColor(szCmd, USERCOLOR_DEFAULT);
				_itoa_s(Command, szCmd, 10); WritePrivateProfileString("MacroQuest", "FilterSkills", szCmd, gszINIFilename);
				return;
			}
		}
		SyntaxError("Usage: /filter skills [all|increase|none]");

	}
	else if (!_stricmp("macros", szArg)) {
		if (szRest[0] == 0) {
			sprintf_s(szCmd, "Filtering of macros is set to: %s", szFilterMacro[gFilterMacro]);
			WriteChatColor(szCmd, USERCOLOR_DEFAULT);
			return;
		}
		for (Command = 0; szFilterMacro[Command]; Command++) {
			if (!_stricmp(szRest, szFilterMacro[Command])) {
				gFilterMacro = Command;
				sprintf_s(szCmd, "Filtering of macros changed to: %s", szFilterMacro[gFilterMacro]);
				WriteChatColor(szCmd, USERCOLOR_DEFAULT);
				_itoa_s(gFilterMacro, szCmd, 10); WritePrivateProfileString("MacroQuest", "FilterMacro", szCmd, gszINIFilename);
				return;
			}
		}
		SyntaxError("Usage: /filter macros [all|enhanced|none|macroended]");
	}
	else if (!_stricmp("mq", szArg)) {
		if (szRest[0] == 0) {
			sprintf_s(szCmd, "Filtering of MQ is set to: %s", szUseChat[gFilterMQ]);
			WriteChatColor(szCmd, USERCOLOR_DEFAULT);
			return;
		}
		for (Command = 0; szUseChat[Command]; Command++) {
			if (!_stricmp(szRest, szUseChat[Command])) {
				gFilterMQ = Command;
				sprintf_s(szCmd, "Filtering of MQ changed to: %s", szUseChat[gFilterMQ]);
				WriteChatColor(szCmd, USERCOLOR_DEFAULT);
				_itoa_s(gFilterMQ, szCmd, 10);
				WritePrivateProfileString("MacroQuest", "FilterMQ", szCmd, gszINIFilename);
				return;
			}
		}
		SyntaxError("Usage: /filter mq [on|off]");
	}
	else if (!_stricmp("mq2data", szArg)) {
		if (szRest[0] == 0) {
			sprintf_s(szCmd, "Filtering of MQ2Data Errors is set to: %s", szUseChat[gFilterMQ2DataErrors]);
			WriteChatColor(szCmd, USERCOLOR_DEFAULT);
			return;
		}
		for (Command = 0; szUseChat[Command]; Command++) {
			if (!_stricmp(szRest, szUseChat[Command])) {
				gFilterMQ2DataErrors = Command;
				sprintf_s(szCmd, "Filtering of MQ changed to: %s", szUseChat[gFilterMQ2DataErrors]);
				WriteChatColor(szCmd, USERCOLOR_DEFAULT);
				_itoa_s(gFilterMQ2DataErrors, szCmd, 10);
				WritePrivateProfileString("MacroQuest", "FilterMQ2Data", szCmd, gszINIFilename);
				return;
			}
		}
		SyntaxError("Usage: /filter mq2data [on|off]");


	}
	else if (!_stricmp("target", szArg)) {
		if (szRest[0] == 0) {
			sprintf_s(szCmd, "Filtering of target lost messages is set to: %s", szFilterTarget[gFilterTarget]);
			WriteChatColor(szCmd, USERCOLOR_DEFAULT);
			return;
		}
		for (Command = 0; szFilterTarget[Command]; Command++) {
			if (!_stricmp(szRest, szFilterTarget[Command])) {
				gFilterTarget = Command;
				sprintf_s(szCmd, "Filtering of target lost messages changed to: %s", szFilterTarget[gFilterTarget]);
				WriteChatColor(szCmd, USERCOLOR_DEFAULT);
				_itoa_s(gFilterTarget, szCmd, 10); WritePrivateProfileString("MacroQuest", "FilterTarget", szCmd, gszINIFilename);
				return;
			}
		}
		SyntaxError("Usage: /filter target [on|off]");

	}
	else if (!_stricmp("debug", szArg)) {
		if (szRest[0] == 0) {
			sprintf_s(szCmd, "Filtering of debug messages is set to: %s", szFilterTarget[gFilterDebug]);
			WriteChatColor(szCmd, USERCOLOR_DEFAULT);
			return;
		}
		for (Command = 0; szFilterTarget[Command]; Command++) {
			if (!_stricmp(szRest, szFilterTarget[Command])) {
				gFilterDebug = Command;
				sprintf_s(szCmd, "Filtering of debug messages changed to: %s", szFilterTarget[gFilterDebug]);
				WriteChatColor(szCmd, USERCOLOR_DEFAULT);
				_itoa_s(gFilterTarget, szCmd, 10); WritePrivateProfileString("MacroQuest", "FilterDebug", szCmd, gszINIFilename);
				return;
			}
		}
		SyntaxError("Usage: /filter debug [on|off]");

	}
	else if (!_stricmp("money", szArg)) {
		if (szRest[0] == 0) {
			sprintf_s(szCmd, "Filtering of money messages is set to: %s", szFilterTarget[gFilterMoney]);
			WriteChatColor(szCmd, USERCOLOR_DEFAULT);
			return;
		}
		for (Command = 0; szFilterTarget[Command]; Command++) {
			if (!_stricmp(szRest, szFilterTarget[Command])) {
				gFilterMoney = Command;
				sprintf_s(szCmd, "Filtering of money messages changed to: %s", szFilterTarget[gFilterMoney]);
				WriteChatColor(szCmd, USERCOLOR_DEFAULT);
				_itoa_s(gFilterMoney, szCmd, 10); WritePrivateProfileString("MacroQuest", "FilterMoney", szCmd, gszINIFilename);
				return;
			}
		}
		SyntaxError("Usage: /filter money [on|off]");
	}
	else if (!_stricmp("encumber", szArg)) {
		if (szRest[0] == 0) {
			sprintf_s(szCmd, "Filtering of encumber messages is set to: %s", szFilterTarget[gFilterEncumber]);
			WriteChatColor(szCmd, USERCOLOR_DEFAULT);
			return;
		}
		for (Command = 0; szFilterTarget[Command]; Command++) {
			if (!_stricmp(szRest, szFilterTarget[Command])) {
				gFilterEncumber = Command;
				sprintf_s(szCmd, "Filtering of encumber messages changed to: %s", szFilterTarget[gFilterEncumber]);
				WriteChatColor(szCmd, USERCOLOR_DEFAULT);
				_itoa_s(gFilterEncumber, szCmd, 10); WritePrivateProfileString("MacroQuest", "FilterEncumber", szCmd, gszINIFilename);
				return;
			}
		}
		SyntaxError("Usage: /filter encumber [on|off]");
	}
	else if (!_stricmp("food", szArg)) {
		if (szRest[0] == 0) {
			sprintf_s(szCmd, "Filtering of food messages is set to: %s", szFilterTarget[gFilterFood]);
			WriteChatColor(szCmd, USERCOLOR_DEFAULT);
			return;
		}
		for (Command = 0; szFilterTarget[Command]; Command++) {
			if (!_stricmp(szRest, szFilterTarget[Command])) {
				gFilterFood = Command;
				sprintf_s(szCmd, "Filtering of food messages changed to: %s", szFilterTarget[gFilterFood]);
				WriteChatColor(szCmd, USERCOLOR_DEFAULT);
				_itoa_s(gFilterFood, szCmd, 10); WritePrivateProfileString("MacroQuest", "FilterFood", szCmd, gszINIFilename);
				return;
			}
		}
		SyntaxError("Usage: /filter food [on|off]");
	}
	else if (!_stricmp("name", szArg)) {
		if (szRest[0] == 0) {
			WriteChatColor("Names currently filtered:", USERCOLOR_DEFAULT);
			WriteChatColor("---------------------------", USERCOLOR_DEFAULT);
			PFILTER pFilter = gpFilters;
			while (pFilter) {
				if (pFilter->pEnabled == &gFilterCustom) {
					sprintf_s(szCmd, "   %s", pFilter->FilterText);
					WriteChatColor(szCmd, USERCOLOR_DEFAULT);
				}
				pFilter = pFilter->pNext;
			}
		}
		else {
			GetArg(szArg, szRest, 1);
			szRest = GetNextArg(szRest);
			if (!_stricmp(szArg, "on") || !_stricmp(szArg, "off")) {
				for (Command = 0; szFilterTarget[Command]; Command++) {
					if (!_stricmp(szArg, szFilterTarget[Command])) {
						gFilterCustom = Command;
						sprintf_s(szCmd, "Filtering of custom messages changed to: %s", szFilterTarget[gFilterCustom]);
						WriteChatColor(szCmd, USERCOLOR_DEFAULT);
						_itoa_s(gFilterCustom, szCmd, 10); WritePrivateProfileString("MacroQuest", "FilterCustom", szCmd, gszINIFilename);
						return;
					}
				}
			}
			else if (!_stricmp(szArg, "remove")) {
				if (szRest[0] == 0) {
					WriteChatColor("Remove what?", USERCOLOR_DEFAULT);
				}
				if (!_stricmp(szRest, "all")) {

					PFILTER pFilter = gpFilters;
					PFILTER pLastFilter = NULL;
					while (pFilter) {
						if (pFilter->pEnabled == &gFilterCustom) {
							if (!pLastFilter) {
								gpFilters = pFilter->pNext;
								free(pFilter);
								pFilter = gpFilters->pNext;
							}
							else {
								pLastFilter->pNext = pFilter->pNext;
								free(pFilter);
								pFilter = pLastFilter->pNext;
							}
						}
						else {
							pLastFilter = pFilter;
							pFilter = pFilter->pNext;
						}
					}
					WriteChatColor("Cleared all name filters.", USERCOLOR_DEFAULT);
					WriteFilterNames();
					return;
				}
				else {
					PFILTER pFilter = gpFilters;
					PFILTER pLastFilter = NULL;
					while (pFilter) {
						if ((pFilter->pEnabled == &gFilterCustom) && (!_stricmp(pFilter->FilterText, szRest))) {
							if (!pLastFilter) {
								gpFilters = pFilter->pNext;
							}
							else {
								pLastFilter->pNext = pFilter->pNext;
							}
							free(pFilter);
							sprintf_s(szCmd, "Stopped filtering on: %s", szRest);
							WriteChatColor(szCmd, USERCOLOR_DEFAULT);
							WriteFilterNames();
							return;
						}
						else {
							pLastFilter = pFilter;
							pFilter = pFilter->pNext;
						}
					}
				}
			}
			else if (!_stricmp(szArg, "add")) {
				if (szRest[0] == 0) {
					WriteChatColor("Add what?", USERCOLOR_DEFAULT);
					return;
				}
				PFILTER pFilter = gpFilters;
				while (pFilter) {
					if ((pFilter->pEnabled == &gFilterCustom) && (!_stricmp(pFilter->FilterText, szRest))) {
						sprintf_s(szCmd, "Name '%s' is already being filtered.", szRest);
						WriteChatColor(szCmd, USERCOLOR_DEFAULT);
						return;
					}
					pFilter = pFilter->pNext;
				}

				AddFilter(szRest, -1, &gFilterCustom);
				WriteFilterNames();
				sprintf_s(szCmd, "Started filtering on: %s", szRest);
				WriteChatColor(szCmd, USERCOLOR_DEFAULT);
				return;
			}
			else {
				SyntaxError("Usage: /filter name on|off|add|remove <name>");
				return;
			}
		}
	}
	else if (!_stricmp("zrange", szArg)) {
		if (szRest[0] == 0) {
			if (gZFilter >= 10000.0f) {
				WriteChatColor("Z range is not currently set.", USERCOLOR_DEFAULT);
			}
			else {
				sprintf_s(szArg, "Z range is set to: %1.2f", gZFilter);
				WriteChatColor(szArg, USERCOLOR_DEFAULT);
			}
		}
		else {
			gZFilter = (FLOAT)atof(szRest);
		}
	}
}


// ***************************************************************************
// Function:    DebugSpewFile
// Description: Our '/spewfile' command
//              Controls logging of DebugSpew to a file
// Usage:       /spewfile [on,off]
// ***************************************************************************
VOID DebugSpewFile(PSPAWNINFO pChar, PCHAR szLine)
{
	BOOL Pause = TRUE;
	CHAR szBuffer[MAX_STRING] = { 0 };
	bRunNextCommand = TRUE;
	if (!_strnicmp(szLine, "off", 3)) {
		gSpewToFile = FALSE;
	}
	else if (!_strnicmp(szLine, "on", 2)) {
		gSpewToFile = TRUE;
	}
	else if (szLine[0] != 0) {
		WriteChatColor("Syntax: /spewfile [on|off]", USERCOLOR_DEFAULT);
	}
	else {
		Pause = !gSpewToFile;
	}
	if (gSpewToFile) {
		sprintf_s(szBuffer, "Debug Spew is being logged to a file.");
	}
	else {
		sprintf_s(szBuffer, "Debug Spew is not being logged to a file.");
	}
	WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
}


// ***************************************************************************
// Function:    Identify
// Description: Our '/identify' command
//              Identifies the item on the cursor, displaying the LORE name.
// Usage:       /identify
// ***************************************************************************
VOID Identify(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
	CHAR szMsg[MAX_STRING] = { 0 };
	CHAR szTmp[MAX_STRING] = { 0 };
	PCHARINFO2 pCharInfo = NULL;
	PITEMINFO pItemInfo = NULL;
	if (NULL == (pCharInfo = GetCharInfo2())) return;
	if (!pCharInfo->pInventoryArray->Inventory.Cursor) {
		MacroError("You must be holding an item to identify it.");
		return;
	}

	pItemInfo = GetItemFromContents(pCharInfo->pInventoryArray->Inventory.Cursor);
	DebugSpew("Identify - %s", pItemInfo->LoreName);
	WriteChatColor(" ", USERCOLOR_SPELLS);
	if (pItemInfo->Type == ITEMTYPE_NORMAL && pItemInfo->ItemType < MAX_ITEMTYPES && szItemTypes[pItemInfo->ItemType] != NULL)
		sprintf_s(szMsg, "Item: %s (Slot: %s, Weight: %d.%d, Value: %dcp, Type: %s)", pItemInfo->Name, szSize[pItemInfo->Size], (INT)(pItemInfo->Weight / 10), (pItemInfo->Weight) % 10, pItemInfo->Cost, szItemTypes[pItemInfo->ItemType]);
	else if (pItemInfo->Type == ITEMTYPE_PACK && pItemInfo->Combine < MAX_COMBINES && szCombineTypes[pItemInfo->Combine] != NULL)
		sprintf_s(szMsg, "Item: %s (Slot: %s, Weight: %d.%d, Value: %dcp, Type: %s)", pItemInfo->Name, szSize[pItemInfo->Size], (INT)(pItemInfo->Weight / 10), (pItemInfo->Weight) % 10, pItemInfo->Cost, szCombineTypes[pItemInfo->Combine]);
	else
		sprintf_s(szMsg, "Item: %s (Slot: %s, Weight: %d.%d, Value: %dcp)", pItemInfo->Name, szSize[pItemInfo->Size], (INT)(pItemInfo->Weight / 10), (pItemInfo->Weight) % 10, pItemInfo->Cost);


	WriteChatColor(szMsg, USERCOLOR_SPELLS);
	if ((pItemInfo->LoreName[0] != '*') && (strcmp(pItemInfo->LoreName, pItemInfo->Name))) {
		sprintf_s(szMsg, "Lore Name: %s", pItemInfo->LoreName);
		WriteChatColor(szMsg, USERCOLOR_SPELLS);
	}
	else     if ((pItemInfo->LoreName[0] == '*') && (strcmp(pItemInfo->LoreName + 1, pItemInfo->Name))) {
		sprintf_s(szMsg, "Lore Name: %s", pItemInfo->LoreName + 1);
		WriteChatColor(szMsg, USERCOLOR_SPELLS);
	}

	strcpy_s(szMsg, "Flags: ");
	if (pItemInfo->LoreName[0] == '*') strcat_s(szMsg, "LORE ");
	if (pItemInfo->NoDrop == 0) strcat_s(szMsg, "NODROP ");
	if (pItemInfo->NoRent == 0) strcat_s(szMsg, "NORENT ");
	if (pItemInfo->Type == ITEMTYPE_NORMAL) {
		if (pItemInfo->Magic == 1) strcat_s(szMsg, "MAGIC ");
		BYTE Light = pItemInfo->Light;
		if ((Light>0) && (Light <= LIGHT_COUNT)) {
			strcat_s(szMsg, "(Light: ");
			strcat_s(szMsg, szLights[Light]);
			strcat_s(szMsg, ") ");
		}
	}
	if (strlen(szMsg) > 7) WriteChatColor(szMsg, USERCOLOR_SPELLS);

	if (pItemInfo->Type == ITEMTYPE_PACK) {
		CHAR szCombine[MAX_STRING] = { 0 };
		if ((pItemInfo->Combine < MAX_COMBINES) && (szCombineTypes[pItemInfo->Combine] != NULL)) {
			strcpy_s(szCombine, szCombineTypes[pItemInfo->Combine]);
		}
		else {
			sprintf_s(szCombine, "*Unknown%d", pItemInfo->Combine);
		}
		sprintf_s(szMsg, "Container: %d Slot %s, %d%% Reduction, Combine=%s", pItemInfo->Slots, szSize[pItemInfo->SizeCapacity], pItemInfo->WeightReduction, szCombine);
	}
	else if (pItemInfo->Type == ITEMTYPE_BOOK) {
		//sprintf_s(szMsg,"Book file: %s", pItemInfo->Book.File);
	}
	else {
		strcpy_s(szMsg, "Item: ");
		if (pItemInfo->AC) {
			sprintf_s(szTmp, "AC%d ", pItemInfo->AC);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->Damage) {
			sprintf_s(szTmp, "%dDam ", pItemInfo->Damage);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->Delay) {
			sprintf_s(szTmp, "%dDly ", pItemInfo->Delay);
			strcat_s(szMsg, szTmp);
		}

		if (pItemInfo->Range) {
			sprintf_s(szTmp, "%dRng ", pItemInfo->Range);
			strcat_s(szMsg, szTmp);
		}

		if (pItemInfo->HP) {
			sprintf_s(szTmp, "%dHP ", pItemInfo->HP);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->Mana) {
			sprintf_s(szTmp, "%dMana ", pItemInfo->Mana);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->STR) {
			sprintf_s(szTmp, "%dSTR ", pItemInfo->STR);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->STA) {
			sprintf_s(szTmp, "%dSTA ", pItemInfo->STA);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->DEX) {
			sprintf_s(szTmp, "%dDEX ", pItemInfo->DEX);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->AGI) {
			sprintf_s(szTmp, "%dAGI ", pItemInfo->AGI);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->WIS) {
			sprintf_s(szTmp, "%dWIS ", pItemInfo->WIS);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->INT) {
			sprintf_s(szTmp, "%dINT ", pItemInfo->INT);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->CHA) {
			sprintf_s(szTmp, "%dCHA ", pItemInfo->CHA);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->SvMagic) {
			sprintf_s(szTmp, "%dSvM ", pItemInfo->SvMagic);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->SvDisease) {
			sprintf_s(szTmp, "%dSvD ", pItemInfo->SvDisease);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->SvPoison) {
			sprintf_s(szTmp, "%dSvP ", pItemInfo->SvPoison);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->SvFire) {
			sprintf_s(szTmp, "%dSvF ", pItemInfo->SvFire);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->SvCold) {
			sprintf_s(szTmp, "%dSvC ", pItemInfo->SvCold);
			strcat_s(szMsg, szTmp);
		}
		if (pItemInfo->SvCorruption) {
			sprintf_s(szTmp, "%dSvCorruption ", pItemInfo->SvCorruption);
			strcat_s(szMsg, szTmp);
		}
		if (((EQ_Item*)pCharInfo->pInventoryArray->Inventory.Cursor)->IsStackable() == 1) {
			sprintf_s(szTmp, "Stack size = %d ", pCharInfo->pInventoryArray->Inventory.Cursor->StackCount);
			strcat_s(szMsg, szTmp);
		}
	}
	if (strlen(szMsg)>6) WriteChatColor(szMsg, USERCOLOR_SPELLS);
}


// ***************************************************************************
// Function:    Location
// Description: Our '/loc' command
//              Displays your loc and current heading...
//              on a 16 point compass (ie. NNE)
// Usage:       /loc
// ***************************************************************************
VOID Location(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
	if (gFilterMacro == FILTERMACRO_NONE) cmdLocation(pChar, szLine);

	CHAR szMsg[MAX_STRING] = { 0 };
	INT Angle = (INT)((pChar->Heading / 32.0f) + 8.5f) % 16;
	sprintf_s(szMsg, "Your Location is %3.2f, %3.2f, %3.2f, and are heading %s.", pChar->Y, pChar->X, pChar->Z, szHeading[Angle]);
	WriteChatColor(szMsg, USERCOLOR_DEFAULT);
}

BOOL CMQ2Alerts::RemoveAlertFromList(DWORD Id, PSEARCHSPAWN pSearchSpawn)
{
	std::list<SEARCHSPAWN>ss;
	if (_AlertMap.find(Id) != _AlertMap.end()) {
		for (std::list<SEARCHSPAWN>::iterator i = _AlertMap[Id].begin(); i != _AlertMap[Id].end(); i++) {
			BOOL bmatch = SearchSpawnMatchesSearchSpawn(&(*i), pSearchSpawn);
			if (bmatch) {
				_AlertMap[Id].erase(i);
				return TRUE;
			}
		}
	}
	return FALSE;
}
BOOL CMQ2Alerts::AddNewAlertList(DWORD Id, PSEARCHSPAWN pSearchSpawn)
{
	lockit lk(_hLockMapWrite, "AddNewAlertList");
	BOOL bCanAdd = 1;
	if (_AlertMap.find(Id) != _AlertMap.end()) {
		for (std::list<SEARCHSPAWN>::iterator i = _AlertMap[Id].begin(); i != _AlertMap[Id].end(); i++) {
			if (SearchSpawnMatchesSearchSpawn(&(*i), pSearchSpawn)) {
				bCanAdd = 0;
				break;
			}
		}
	}
	if (bCanAdd) {
		_AlertMap[Id].push_back(*pSearchSpawn);
		return TRUE;
	}
	return FALSE;
}
VOID CMQ2Alerts::FreeAlerts(DWORD List)
{
	lockit lk(_hLockMapWrite, "FreeAlerts");
	CHAR szBuffer[64] = { 0 };
	if (_AlertMap.find(List) != _AlertMap.end()) {
		_AlertMap.erase(List);
		sprintf_s(szBuffer, "Alert list %d cleared.", List);
	}
	else {
		sprintf_s(szBuffer, "No Alert list for %d was found.", List);
	}
	WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
}
// ***************************************************************************
// Function:    Alert
// Description: Our '/alert' command
//              Sets up $alert notifications
// Usage:       /alert [clear #] [list #] [add/remove # [pc|npc|corpse|any] [radius ###] [zradius ###] [race race] [class class] [range min max] [name]]
// ***************************************************************************
VOID Alert(PSPAWNINFO pChar, PCHAR szLine)
{
	bRunNextCommand = TRUE;
	CHAR szArg[MAX_STRING] = { 0 };
	CHAR szLLine[MAX_STRING] = { 0 };
	PCHAR szRest = szLLine;
	BOOL Parsing = TRUE;
	BOOL DidSomething = FALSE;

	// if szLLine is not referenced above by szRest
	// the compiler thinks it is not used and optimizes it out
	// don't be too tricky
	strcpy_s(szLLine, szLine);
	_strlwr_s(szLLine);

	while (Parsing) {
		if (szRest[0] == 0) {
			Parsing = FALSE;
		}
		else {
			GetArg(szArg, szRest, 1);
			szRest = GetNextArg(szRest, 1);
			if (!strcmp(szArg, "clear")) {
				GetArg(szArg, szRest, 1);
				CAlerts.FreeAlerts(atoi(szArg));
				DidSomething = TRUE;
			}
			else if (!strcmp(szArg, "list")) {
				GetArg(szArg, szRest, 1);
				szRest = GetNextArg(szRest, 1);
				std::list<SEARCHSPAWN>ss;
				if (CAlerts.GetAlert(atoi(szArg), ss)) {
					CHAR Buffer[MAX_STRING] = { 0 };
					DWORD Count = 0;
					WriteChatColor(" ", USERCOLOR_DEFAULT);
					WriteChatColor("Current alerts:", USERCOLOR_DEFAULT);
					CHAR szTemp[2048] = { 0 };
					for (std::list<SEARCHSPAWN>::iterator i = ss.begin(); i != ss.end(); i++) {
						int dist = std::distance(ss.begin(), i);
						FormatSearchSpawn(Buffer, sizeof(Buffer), &(*i));
						sprintf_s(szTemp, "[%d] %s", dist, Buffer);
						WriteChatColor(szTemp, USERCOLOR_DEFAULT);
					}
					sprintf_s(Buffer, "%d alerts listed.", ss.size());
					WriteChatColor(Buffer, USERCOLOR_DEFAULT);
				}
				else {
					WriteChatColor("No alerts active.", USERCOLOR_DEFAULT);
				}
				DidSomething = TRUE;
			}
			else if (!strcmp(szArg, "remove")) {
				CHAR Buffer[MAX_STRING] = { 0 };
				CHAR szArg1[MAX_STRING] = { 0 };
				BOOL ParsingAdd = TRUE;
				DWORD List = 0;
				GetArg(szArg, szRest, 1);
				szRest = GetNextArg(szRest, 1);
				List = atoi(szArg);
				PSEARCHSPAWN pSearchSpawn = (PSEARCHSPAWN)malloc(sizeof(SEARCHSPAWN));
				if (!pSearchSpawn) {
					MacroError("Couldn't remove alert.");
					DebugSpew("Alert - Unable to allocate memory for temp alert.");
					return;
				}
				ZeroMemory(pSearchSpawn, sizeof(SEARCHSPAWN));
				ClearSearchSpawn(pSearchSpawn);
				while (ParsingAdd) {
					GetArg(szArg1, szRest, 1);
					szRest = GetNextArg(szRest, 1);
					if (szArg1[0] == 0) {
						ParsingAdd = FALSE;
					}
					else {
						szRest = ParseSearchSpawnArgs(szArg1, szRest, pSearchSpawn);
					}
				}
				CHAR szTemp[MAX_STRING] = { 0 };
				if (CAlerts.RemoveAlertFromList(List, pSearchSpawn)) {
					sprintf_s(Buffer, "Removed alert for: %s", FormatSearchSpawn(szTemp, sizeof(szTemp), pSearchSpawn));
				}
				else {
					MacroError("Couldn't find alert.");
					sprintf_s(Buffer, "Couldn't find alert.");
				}
				if (pSearchSpawn)
					free(pSearchSpawn);
				WriteChatColor(Buffer, USERCOLOR_DEFAULT);
				DidSomething = TRUE;
			}
			else if (!strcmp(szArg, "add")) {
				CHAR Buffer[MAX_STRING] = { 0 };
				CHAR szArg1[MAX_STRING] = { 0 };
				BOOL ParsingAdd = TRUE;
				DWORD List = 0;
				GetArg(szArg, szRest, 1);
				szRest = GetNextArg(szRest, 1);
				List = atoi(szArg);
				PSEARCHSPAWN pSearchSpawn = (PSEARCHSPAWN)malloc(sizeof(SEARCHSPAWN));
				if (!pSearchSpawn) {
					MacroError("Couldn't create alert.");
					DebugSpew("Alert - Unable to allocate memory for new alert.");
					return;
				}
				ZeroMemory(pSearchSpawn, sizeof(SEARCHSPAWN));
				ClearSearchSpawn(pSearchSpawn);
				while (ParsingAdd) {
					GetArg(szArg1, szRest, 1);
					szRest = GetNextArg(szRest, 1);
					if (szArg1[0] == 0) {
						ParsingAdd = FALSE;
					}
					else {
						szRest = ParseSearchSpawnArgs(szArg1, szRest, pSearchSpawn);
					}
				}

				// prev/next aren't logical in alerts
				pSearchSpawn->bTargNext = FALSE;
				pSearchSpawn->bTargPrev = FALSE;

				CHAR szTemp[MAX_STRING] = { 0 };
				if (CheckAlertForRecursion(pSearchSpawn, List)) {
					sprintf_s(Buffer, "Alert would have caused recursion: %s", FormatSearchSpawn(szTemp, sizeof(szTemp), pSearchSpawn));
				}
				else {
					if (CAlerts.AddNewAlertList(List, pSearchSpawn)) {
						sprintf_s(Buffer, "Added alert for: %s", FormatSearchSpawn(szTemp, sizeof(szTemp), pSearchSpawn));
					}
					else {
						sprintf_s(Buffer, "Alert NOT added because there was already an alert for: %s", FormatSearchSpawn(szTemp, sizeof(szTemp), pSearchSpawn));
					}
				}
				if (pSearchSpawn)
					free(pSearchSpawn);
				DebugSpew("Alert - %s", Buffer);
				WriteChatColor(Buffer, USERCOLOR_DEFAULT);
				DidSomething = TRUE;
			}
		}
	}
	if (!DidSomething) {
		SyntaxError("Usage: /alert [clear #] [list #] [add/remove # [pc|npc|corpse|any] [radius radius] [zradius radius] [range min max] spawn]");
	}
}




// ***************************************************************************
// Function:    SuperWhoTarget
// Description: Our '/whotarget' command
//              Displays spawn currently selected
// Usage:       /whotarget
// ***************************************************************************
VOID SuperWhoTarget(PSPAWNINFO pChar, PCHAR szLine)
{
	PSPAWNINFO psTarget = NULL;
	bRunNextCommand = TRUE;
	if (gFilterMacro == FILTERMACRO_NONE) cmdWhoTarget(pChar, szLine);

	if (ppTarget && pTarget) {
		psTarget = (PSPAWNINFO)pTarget;
	}



	if (!psTarget) {
		MacroError("You must have a target selected for /whotarget.");
		return;
	}
	DebugSpew("SuperWhoTarget - %s", psTarget->Name);
	BOOL Temp = gFilterSWho.Distance;
	gFilterSWho.Distance = TRUE;
	SuperWhoDisplay(psTarget, USERCOLOR_WHO);
	gFilterSWho.Distance = Temp;
	//SuperWhoDisplay(pChar,NULL,psTarget,0,TRUE);
}



/**/


// ***************************************************************************
// Function:    MQMsgBox
// Description: Our '/msgbox' command
//              Our message box
// Usage:       /msgbox text
// ***************************************************************************
VOID MQMsgBox(PSPAWNINFO pChar, PCHAR szLine)
{
	FILE *fOut = NULL;
	CHAR szBuffer[MAX_STRING] = { 0 };
	DWORD i;
	bRunNextCommand = TRUE;

	sprintf_s(szBuffer, "${Time.Date} ${Time}\r\n%s", szLine);
	ParseMacroParameter(pChar, szBuffer);

	CreateThread(NULL, 0, thrMsgBox, _strdup(szBuffer), 0, &i);
}


// ***************************************************************************
// Function:    MacroLog
// Description: Our '/mqlog' command
//              Our logging
// Usage:       /mqlog text
// ***************************************************************************
VOID MacroLog(PSPAWNINFO pChar, PCHAR szLine)
{
	FILE *fOut = NULL;
	CHAR Filename[MAX_STRING] = { 0 };
	CHAR szBuffer[MAX_STRING] = { 0 };
	DWORD i;
	bRunNextCommand = TRUE;

	if (gszMacroName[0] == 0) {
		sprintf_s(Filename, "%s\\MacroQuest.log", gszLogPath);
	}
	else {
		sprintf_s(Filename, "%s\\%s.log", gszLogPath, gszMacroName);
	}

	for (i = 0; i<strlen(Filename); i++) {
		if (Filename[i] == '\\') {
			strncpy_s(szBuffer, Filename, i);
			szBuffer[i] = '\0';
			if (2 == _mkdir(szBuffer)) {
				MacroError("Log path doesn't appear valid: %s", Filename);
				return;
			}
		}
	}
	if (!_stricmp(szLine, "clear")) {
		errno_t err = fopen_s(&fOut,Filename, "wt");
		if (err) {
			MacroError("Couldn't open log file: %s", Filename);
			return;
		}
		WriteChatColor("Cleared log.", USERCOLOR_DEFAULT);
		fclose(fOut);
		return;
	}

	errno_t err = fopen_s(&fOut,Filename, "at");
	if (err) {
		MacroError("Couldn't open log file: %s", Filename);
		return;
	}

	sprintf_s(szBuffer, "[${Time.Date} ${Time.Time24}] %s", szLine);
	ParseMacroParameter(pChar, szBuffer);
	fprintf(fOut, "%s\n", szBuffer);
	DebugSpew("MacroLog - %s", szBuffer);

	fclose(fOut);

}


// ***************************************************************************
// Function:    Face
// Description: Our '/face' command
//              Faces the direction to a spawn
// Usage:       /face <spawn>
// Usage:       /face loc <y>,<x>
// ***************************************************************************
VOID Face(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!ppSpawnManager) return;
	if (!pSpawnList) return;
	if (GetGameState() != GAMESTATE_INGAME) {
		MacroError("You shouldn't execute /face when not in game. Gamestate is %d", GetGameState());
		return;
	}
	PSPAWNINFO pSpawnClosest = NULL;
	PSPAWNINFO psTarget = NULL;
	SPAWNINFO LocSpawn = { 0 };
	SEARCHSPAWN SearchSpawn;
	ClearSearchSpawn(&SearchSpawn);
	CHAR szMsg[MAX_STRING] = { 0 };
	CHAR szName[MAX_STRING] = { 0 };
	CHAR szArg[MAX_STRING] = { 0 };
	CHAR szLLine[MAX_STRING] = { 0 };
	PCHAR szFilter = szLLine;
	BOOL bArg = TRUE;
	BOOL bOtherArgs = FALSE;
	BOOL Away = FALSE;
	BOOL Predict = FALSE;
	BOOL Fast = FALSE;
	BOOL Look = TRUE;
	BOOL Parsing = TRUE;
	DOUBLE Distance;
	bRunNextCommand = FALSE;
	strcpy_s(szLLine, szLine);
	_strlwr_s(szLLine);
	while (bArg) {
		GetArg(szArg, szFilter, 1);
		szFilter = GetNextArg(szFilter, 1);
		if (szArg[0] == 0) {
			bArg = FALSE;
		}
		else if (!strcmp(szArg, "predict")) {
			Predict = TRUE;
		}
		else if (!strcmp(szArg, "fast")) {
			Fast = TRUE;
		}
		else if (!strcmp(szArg, "away")) {
			Away = TRUE;
		}
		else if (!strcmp(szArg, "nolook")) {
			Look = FALSE;
		}
		else if (!_stricmp(szArg, "loc")) {
			pSpawnClosest = &LocSpawn;
			strcpy_s(LocSpawn.Name, "location");
			if ((szFilter[0] == 0) || (!strstr(szFilter, ","))) {
				MacroError("Face: loc specified but <y>,<x> not found.");
				return;
			}
			pSpawnClosest->Y = (FLOAT)atof(szFilter);
			while ((szFilter[0] != ',') && (szFilter[0] != 0)) szFilter++;
			if (szFilter[0] == 0) {
				MacroError("Face: loc specified but <y>,<x> not found.");
				return;
			}
			szFilter++;
			pSpawnClosest->X = (FLOAT)atof(szFilter);
		}
		else if (!_stricmp(szArg, "item")) {
			if (EnviroTarget.Name[0] == 0) {
				MacroError("Face: item specified but no item targetted.");
				return;
			}
			pSpawnClosest = &EnviroTarget;
		}
		else if (!_stricmp(szArg, "door")) {
			if (DoorEnviroTarget.Name[0] == 0) {
				MacroError("Face: door specified but no door targetted.");
				return;
			}
			pSpawnClosest = &DoorEnviroTarget;
		}
		else if (!_stricmp(szArg, "heading")) {
			if (szFilter[0] == 0) {
				MacroError("Face: heading specified but angle not found.");
			}
			else {
				FLOAT Heading = (FLOAT)(atof(szFilter));
				gFaceAngle = Heading / 0.703125f;
				if (gFaceAngle >= 512.0f) gFaceAngle -= 512.0f;
				if (gFaceAngle<0.0f) gFaceAngle += 512.0f;
				if (Fast) {
					//changed
					((PSPAWNINFO)pCharSpawn)->Heading = (FLOAT)gFaceAngle;
					gFaceAngle = 10000.0f;
					bRunNextCommand = TRUE;
				}
			}
			return;
		}
		else if (!strcmp(szArg, "help")) {
			SyntaxError("Usage: /face [spawn] [item] [door] [id #] [heading <ang>] [loc <y>,<x>] [away] [alert #]");
			bRunNextCommand = TRUE;
			return;
		}
		else {
			bOtherArgs = TRUE;
			szFilter = ParseSearchSpawnArgs(szArg, szFilter, &SearchSpawn);
		}
	}

	if (!pSpawnClosest) {
		if (!bOtherArgs) {
			if (ppTarget && pTarget) {
				pSpawnClosest = (PSPAWNINFO)pTarget;
			}
		}
		else {
			pSpawnClosest = SearchThroughSpawns(&SearchSpawn, pChar);
		}
	}

	szMsg[0] = 0;

	if (!pSpawnClosest) {
		MacroError("There were no matches for: %s", FormatSearchSpawn(szArg, sizeof(szArg), &SearchSpawn));
	}
	else {
		if (Predict) {
			Distance = Distance3DToSpawn(pChar, pSpawnClosest);
			gFaceAngle = (
				atan2((pSpawnClosest->X + (pSpawnClosest->SpeedX * Distance)) - pChar->X,
					(pSpawnClosest->Y + (pSpawnClosest->SpeedY * Distance)) - pChar->Y)
				* 256.0f / PI);
		}
		else {
			gFaceAngle = (
				atan2(pSpawnClosest->X - pChar->X,
					pSpawnClosest->Y - pChar->Y)
				* 256.0f / PI);
		}
		if (Look) {
			Distance = Distance3DToSpawn(pChar, pSpawnClosest);
			gLookAngle = (
				atan2(pSpawnClosest->Z + pSpawnClosest->AvatarHeight*StateHeightMultiplier(pSpawnClosest->StandState) - pChar->Z - pChar->AvatarHeight*StateHeightMultiplier(pChar->StandState),
					(FLOAT)Distance)
				* 256.0f / PI);
			if (Away) gLookAngle = -gLookAngle;
			if (Fast) {
				pChar->CameraAngle = (FLOAT)gLookAngle;
				gLookAngle = 10000.0f;
			}
		}
		if (Away) {
			gFaceAngle += 256.0f;
		}
		if (gFaceAngle >= 512.0f) gFaceAngle -= 512.0f;
		if (gFaceAngle<0.0f) gFaceAngle += 512.0f;
		if (Fast) {
			//changed
			((PSPAWNINFO)pCharSpawn)->Heading = (FLOAT)gFaceAngle;
			gFaceAngle = 10000.0f;
			bRunNextCommand = TRUE;
		}
		//sprintf_s(szMsg,"Facing %s'%s'...",(Away)?"away from ":"", CleanupName(strcpy_s(szName,pSpawnClosest->Name),FALSE));
		sprintf_s(szMsg, "Facing %s'%s'...", (Away) ? "away from " : "", pSpawnClosest->DisplayedName);
	}
	if (ppTarget && pTarget) {
		psTarget = (PSPAWNINFO)pTarget;
	}
	if (szMsg[0] && ((pSpawnClosest != &LocSpawn) && ((Away) || (pSpawnClosest != psTarget)))) WriteChatColor(szMsg, USERCOLOR_WHO);
	DebugSpew("Face - %s", szMsg);
	return;
}


// ***************************************************************************
// Function:      Look
// Description:      Our /look command. Changes camera angle
// 2003-08-30      MacroFiend
// ***************************************************************************
VOID Look(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szLookAngle[MAX_STRING] = { 0 };
	CHAR szTemp[MAX_STRING] = { 0 };
	FLOAT fLookAngle = 0.0f;

	GetArg(szLookAngle, szLine, 1);


	fLookAngle = (FLOAT)atof(szLookAngle);

	if (fLookAngle>128.0f || fLookAngle<-128.0f) {
		MacroError("/look -- Angle %f out of range.", fLookAngle);
		return;
	}

	pChar->CameraAngle = fLookAngle;
	gLookAngle = 10000.0f;
}


// ***************************************************************************
// Function:    Where
// Description: Our '/where' command
//              Displays the direction and distance to the closest spawn
// Usage:       /where <spawn>
// ***************************************************************************
VOID Where(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!ppSpawnManager) return;
	if (!pSpawnList) return;
	PSPAWNINFO pSpawnClosest = NULL;
	SEARCHSPAWN SearchSpawn;
	ClearSearchSpawn(&SearchSpawn);
	CHAR szMsg[MAX_STRING] = { 0 };
	CHAR szName[MAX_STRING] = { 0 };
	CHAR szArg[MAX_STRING] = { 0 };
	CHAR szLLine[MAX_STRING] = { 0 };
	PCHAR szFilter = szLLine;
	BOOL bArg = TRUE;
	bRunNextCommand = TRUE;
	strcpy_s(szLLine, szLine);
	_strlwr_s(szLLine);

	while (bArg) {
		GetArg(szArg, szFilter, 1);
		szFilter = GetNextArg(szFilter, 1);
		if (szArg[0] == 0) {
			bArg = FALSE;
		}
		else {
			szFilter = ParseSearchSpawnArgs(szArg, szFilter, &SearchSpawn);
		}
	}


	if (!(pSpawnClosest = SearchThroughSpawns(&SearchSpawn, pChar))) {
		sprintf_s(szMsg, "There were no matches for: %s", FormatSearchSpawn(szArg, sizeof(szArg), &SearchSpawn));
	}
	else {
		INT Angle = (INT)((atan2f(pChar->X - pSpawnClosest->X, pChar->Y - pSpawnClosest->Y) * 180.0f / PI + 360.0f) / 22.5f + 0.5f) % 16;
		strcpy_s(szName, pSpawnClosest->DisplayedName);
		sprintf_s(szMsg, "The closest '%s' is a level %d %s %s and %1.2f away to the %s, Z difference = %1.2f",
			//CleanupName(strcpy_s(szName,pSpawnClosest->Name),FALSE),
			szName,
			pSpawnClosest->Level,
			pEverQuest->GetRaceDesc(pSpawnClosest->mActorClient.Race),
			GetClassDesc(pSpawnClosest->mActorClient.Class),
			Distance3DToSpawn(pChar, pSpawnClosest), szHeading[Angle],
			pSpawnClosest->Z - pChar->Z);
		DebugSpew("Where - %s", szMsg);
	}
	WriteChatColor(szMsg, USERCOLOR_WHO);
	return;
}


// ***************************************************************************
// Function:    DoAbility
// Description: Our '/doability' command
//              Does (or lists) your abilities
// Usage:       /doability [list|ability|#]
// ***************************************************************************
VOID DoAbility(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0] || !cmdDoAbility) return;
	if (atoi(szLine) || !EQADDR_DOABILITYLIST) {
		cmdDoAbility(pChar, szLine);
		return;
	}

	DWORD Index;
	CHAR szBuffer[MAX_STRING] = { 0 };
	GetArg(szBuffer, szLine, 1);
	if (PCHARINFO2 pChar2 = GetCharInfo2()) {
		// display available abilities list
		if (!_stricmp(szBuffer, "list")) {
			WriteChatColor("Abilities & Combat Skills:", USERCOLOR_DEFAULT);
			// display skills that have activated state
			for (Index = 0; Index < NUM_SKILLS; Index++)
			{
				if (((CharacterZoneClient*)pCharData1)->HasSkill(Index))
				{
					bool Avail = pSkillMgr->pSkill[Index]->Activated;

					// make sure remove trap is added, they give it to everyone except rogues
					if (Index == 75 && strncmp(pEverQuest->GetClassDesc(pChar2->Class & 0xFF), "Rogue", 6))
						Avail = true;

					if (Avail)
					{
						sprintf_s(szBuffer, "<\ag%s\ax>", szSkills[Index]);
						WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
					}
				}
			}
			// display innate skills that are available
			for (Index = 0; Index < 28; Index++)
			{
				if (pChar2->InnateSkill[Index] != 0xFF && strlen(szSkills[Index + 100])>3)
				{
					sprintf_s(szBuffer, "<\ag%s\ax>", szSkills[Index + 100]);
					WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
				}
			}

			// display discipline i have
			WriteChatColor("Combat Abilities:", USERCOLOR_DEFAULT);
			for (Index = 0; Index < NUM_COMBAT_ABILITIES; Index++) {
				if (pCombatSkillsSelectWnd->ShouldDisplayThisSkill(Index)) {
					if (PSPELL pCA = GetSpellByID(pPCData->GetCombatAbility(Index))) {
						sprintf_s(szBuffer, "<\ag%s\ax>", pCA->Name);
						WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
					}
				}
			}
			return;
		}
		// scan for matching abilities name
		for (Index = 0; Index < 128; Index++) {
			if ((Index < NUM_SKILLS && (pSkillMgr->pSkill[Index])->Activated) ||
				(Index >= NUM_SKILLS && pChar2->InnateSkill[Index - 100] != 0xFF))
			{
				if (!_stricmp(szBuffer, szSkills[Index]))
				{
					if (!((CharacterZoneClient*)pCharData1)->HasSkill(Index))
					{
						WriteChatf("you do not have this skill");
						return;
					}
					pCharData1->UseSkill((unsigned char)Index, (EQPlayer*)pCharData1);
					return;
				}
			}
		}

		// scan for matching discipline name
		for (Index = 0; Index < NUM_COMBAT_ABILITIES; Index++)
		{
			if (pCombatSkillsSelectWnd->ShouldDisplayThisSkill(Index)) {
				if (PSPELL pCA = GetSpellByID(pChar2->CombatAbilities[Index]))
				{
					if (!_stricmp(pCA->Name, szBuffer))
					{
						pCharData->DoCombatAbility(pCA->ID);
						return;
					}
				}
			}
		}
	}
	// else display that we didnt found abilities
	WriteChatColor("You do not seem to have that ability available", USERCOLOR_DEFAULT);
}

// ***************************************************************************
// Function:    LoadSpells
// Description: Our '/loadspells' command
//              Loads (or lists) a spell favorite list
// Usage:       /loadspells [list|"name"]
// ***************************************************************************
VOID LoadSpells(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!pSpellSets || !ppSpellBookWnd || szLine[0] == 0) return;

	DWORD Index, DoIndex = 0xFFFFFFFF;
	CHAR szArg1[MAX_STRING] = { 0 };
	CHAR szArg2[MAX_STRING] = { 0 };
	CHAR szBuffer[MAX_STRING] = { 0 };
	BOOL szMissingSpell = FALSE;

	if (!pSpellBookWnd) return;

	GetArg(szArg1, szLine, 1);
	GetArg(szArg2, szLine, 2);

	if ((!_stricmp(szArg1, "list")) && (szArg2[0] == 0)) {
		WriteChatColor("Spell favorites list:", USERCOLOR_DEFAULT);
		WriteChatColor("--------------------------", USERCOLOR_DEFAULT);
		for (Index = 0; Index<NUM_SPELL_SETS; Index++) {
			if (pSpellSets[Index].Name[0] != 0) {
				sprintf_s(szBuffer, "%d) %s", Index, pSpellSets[Index].Name);
				WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
			}
		}
		return;
	}

	if (!_stricmp(szArg1, "list")) {

		DoIndex = IsNumber(szArg2) ? atoi(szArg2) : FindSpellListByName(szArg2);
		if (DoIndex < 0 || DoIndex > NUM_SPELL_SETS - 1) {
			sprintf_s(szBuffer, "Unable to find favorite list '%s'", szArg2);
			WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
			return;
		}
		sprintf_s(szBuffer, "Favorite list '%s':", pSpellSets[DoIndex].Name);
		WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		for (Index = 0; Index<NUM_SPELL_GEMS; Index++) {
			if (pSpellSets[DoIndex].SpellId[Index] != 0xFFFFFFFF) {
				sprintf_s(szBuffer, "%d) %s", Index, GetSpellByID(pSpellSets[DoIndex].SpellId[Index])->Name);
				WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
			}
		}
		return;
	}

	DoIndex = IsNumber(szArg1) ? atoi(szArg1) : FindSpellListByName(szArg1);
	if (DoIndex >= 0 && DoIndex < NUM_SPELL_SETS) {
		for (Index = 0; Index < NUM_SPELL_GEMS; Index++) {
			if (pSpellSets[DoIndex].SpellId[Index] != 0xFFFFFFFF) {

				if (GetMemorizedSpell(Index) != pSpellSets[DoIndex].SpellId[Index]) {
					szMissingSpell = TRUE;
					//sprintf_s(szBuffer, "Missing %d) %s", Index, GetSpellByID(pSpellSets[DoIndex].SpellId[Index])->Name);
					//WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
				}

			}
		}

		if (szMissingSpell) {
			pSpellBookWnd->MemorizeSet((int*)&pSpellSets[DoIndex], NUM_SPELL_GEMS);
		}
	}
	else {
		sprintf_s(szBuffer, "Unable to find favorite list '%s'", szArg1);
		WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
	}
}

void CastSplash(int Index, PSPELL pSpell)
{
	ScreenVector3 sv3;
	sv3.dx = 0xFFFFFFFF;//setting up the default for the targetindicator (splashring)
	sv3.dy = 0xFFFFFFFF;
	sv3.dz = 0xFFFF;// weird right? shouldn't it be 0xFFFFFFFF ? well maybe a bug but thats what the client has it as... -eqmule
	if (CTargetRing *pTR = pEverQuest->CreateTargetIndicator(Index, pSpell, &sv3, 0)) {
		if (pTarget) {
			sv3.x = ((PSPAWNINFO)pTarget)->Y;//yes we really need to set Y to x, this is not a bug.
			sv3.y = ((PSPAWNINFO)pTarget)->X;
			sv3.z = ((PSPAWNINFO)pTarget)->Z;
		}
		else {//ok fine, this is probably not what the user wants but if he doesnt have a target im just gonna splash myself...
			sv3.x = ((PSPAWNINFO)pLocalPlayer)->Y;
			sv3.y = ((PSPAWNINFO)pLocalPlayer)->X;
			sv3.z = ((PSPAWNINFO)pLocalPlayer)->Z;
		}
		//ok we better check if splash can be cast in the location
		bool cansee = pLocalPlayer->CanSeeTargetIndicator(&sv3);
		if (cansee) {
			//ok its all good, lets cast it.
			pTR->Cast(&sv3);
			//need to delete it...
			pEverQuest->DeleteTargetIndicator();
		}
		else {
			WriteChatColor("You cannot see your target", CONCOLOR_LIGHTBLUE);
		}
	}
	else {
		WriteChatColor("Creation of a TargetIndicator Failed.", CONCOLOR_YELLOW);
	}
}

// ***************************************************************************
// Function:    Cast
// Description: Our '/cast' command
// Usage:       /cast [list|#|"name of spell"|item "name of item"]
// ***************************************************************************
VOID Cast(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!cmdCast) return;
	if (szLine[0] == 0 || atoi(szLine) || !ppSpellMgr || !ppCharData || !pCharData) {
		int Index = atoi(szLine);
		Index--;
		if (Index >= 0 && Index < NUM_SPELL_GEMS) {
			if (PSPELL pSpell = GetSpellByID(GetMemorizedSpell(Index))) {
				if (pSpell->TargetType == 0x2d) {//is it a splashspell?
					CastSplash(Index, pSpell);
					return;
				}
			}
		}
		cmdCast(pChar, szLine);
		return;
	}
	DWORD Index;
	CHAR szBuffer[MAX_STRING] = { 0 };
	CHAR szArg1[MAX_STRING] = { 0 };
	CHAR szArg2[MAX_STRING] = { 0 };
	if (!_stricmp(szLine, "list")) {
		WriteChatColor("Spells:", USERCOLOR_DEFAULT);
		for (Index = 0; Index < NUM_SPELL_GEMS; Index++) {
			LONG Spellid = GetMemorizedSpell(Index);
			if (Spellid == 0xFFFFFFFF) {
				sprintf_s(szBuffer, "%d. <Empty>", Index + 1);
			}
			else {
				sprintf_s(szBuffer, "%d. %s", Index + 1, GetSpellNameByID(Spellid));
			}
			WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		}
		return;
	}

	GetArg(szArg1, szLine, 1);
	GetArg(szArg2, szLine, 2);
	DebugSpew("Cast: szArg1 = %s szArg2 = %s", szArg1, szArg2);
	if (!_stricmp(szArg1, "item"))
	{
		if (HasExpansion(EXPANSION_VoA))
		{
			if (PCONTENTS pItem = FindItemByName(szArg2, true))
			{
				if (GetItemFromContents(pItem)->Clicky.SpellID > 0 && GetItemFromContents(pItem)->Clicky.SpellID != -1)
				{
					CHAR cmd[MAX_STRING] = { 0 };
					sprintf_s(cmd, "/useitem \"%s\"", GetItemFromContents(pItem)->Name);
					EzCommand(cmd);
				}
			}
			else {
				WriteChatf("Item '%s' not found.", szArg2);
			}
		}
		else {
			if (PCONTENTS pItem = FindItemByName(szArg2, true))
			{
				if (pItem->GlobalIndex.Index.Slot1 < NUM_INV_SLOTS)
				{
					if (GetItemFromContents(pItem)->Clicky.SpellID > 0 && GetItemFromContents(pItem)->Clicky.SpellID != -1)
					{
						if (CInvSlot *pSlot = pInvSlotMgr->FindInvSlot(pItem->GlobalIndex.Index.Slot1))
						{
							CXPoint p; p.A = 0; p.B = 0;
							pSlot->HandleRButtonUp(&p);
						}
					}
				}
			}
			else {
				WriteChatf("Item '%s' not found.", szArg2);
			}
		}
		return;
	}
	GetArg(szBuffer, szLine, 1);

	for (Index = 0; Index < NUM_SPELL_GEMS; Index++) {
		LONG Spellid = GetMemorizedSpell(Index);
		if (Spellid != 0xFFFFFFFF) {
			if (PSPELL pSpell = GetSpellByID(Spellid)) {
				if (!_stricmp(szBuffer, pSpell->Name)) {
					if (pSpell->TargetType == 0x2d) {//is it a splashspell?
						CastSplash(Index, pSpell);
						return;
					}
					else {//nope normal, so just pipe it through
						  //DebugSpew("SpellName = %s",SpellName);
						_itoa_s(Index + 1, szBuffer, 10);
						cmdCast(pChar, szBuffer);
						//DebugSpew("pChar = %x SpellName = %s %s",pChar,SpellName,_itoa_s(Index+1,szBuffer,10));
					}
					return;
				}
			}
		}
	}
	WriteChatColor("You do not seem to have that spell memorized.", USERCOLOR_DEFAULT);
	return;
}

// ***************************************************************************
// Function:    Target
// Description: Our '/target' command
//              Selects the closest spawn
// Usage:       /target [spawn|myself|mycorpse]
// ***************************************************************************
VOID Target(PSPAWNINFO pChar, PCHAR szLine)
{
	gTargetbuffs = FALSE;
	if (!ppSpawnManager) return;
	if (!pSpawnList) return;
	PSPAWNINFO pSpawnClosest = NULL;
	SEARCHSPAWN SearchSpawn;
	ClearSearchSpawn(&SearchSpawn);
	CHAR szArg[MAX_STRING] = { 0 };
	CHAR szMsg[MAX_STRING] = { 0 };
	CHAR szLLine[MAX_STRING] = { 0 };
	PCHAR szFilter = szLLine;
	BOOL DidTarget = FALSE;
	BOOL bArg = TRUE;

	bRunNextCommand = TRUE;
	strcpy_s(szLLine, szLine);
	_strlwr_s(szLLine);
	while (bArg) {
		GetArg(szArg, szFilter, 1);
		szFilter = GetNextArg(szFilter, 1);
		if (szArg[0] == 0) {
			bArg = FALSE;
		}
		else if (!strcmp(szArg, "myself")) {
			if (((PCHARINFO)pCharData)->pSpawn) {
				pSpawnClosest = ((PCHARINFO)pCharData)->pSpawn;
				DidTarget = TRUE;
			}
		}
		else if (!strcmp(szArg, "mycorpse")) {
			if (((PCHARINFO)pCharData)->pSpawn) {
				sprintf_s(szFilter, MAX_STRING,"%s's Corpse", ((PCHARINFO)pCharData)->pSpawn->Name);
				_strlwr_s(szFilter, MAX_STRING);
			}
		}
		else if (!strcmp(szArg, "clear")) {
			pTarget = NULL;
			pDoorTarget = 0;
			pGroundTarget = 0;
			ZeroMemory(&EnviroTarget, sizeof(EnviroTarget));
			ZeroMemory(&DoorEnviroTarget, sizeof(DoorEnviroTarget));
			if (pChar)
				pChar->GroupMemberTargeted = 0xFFFFFFFF;
			DebugSpew("Target cleared.");
			WriteChatColor("Target cleared.", USERCOLOR_WHO);
			return;
		}
		else {
			szFilter = ParseSearchSpawnArgs(szArg, szFilter, &SearchSpawn);
		}
	}
	if (pTarget) SearchSpawn.FromSpawnID = ((PSPAWNINFO)pTarget)->SpawnID;

	if (!DidTarget) {
		pSpawnClosest = SearchThroughSpawns(&SearchSpawn, pChar);
	}
	if (!pSpawnClosest) {
		CHAR szTemp[MAX_STRING] = { 0 };
		sprintf_s(szMsg, "There are no spawns matching: %s", FormatSearchSpawn(szTemp, sizeof(szTemp), &SearchSpawn));
	}
	else {
		PSPAWNINFO *psTarget = NULL;
		if (ppTarget) {
			psTarget = (PSPAWNINFO*)ppTarget;
			*psTarget = pSpawnClosest;
			DebugSpew("Target - %s selected", pSpawnClosest->Name);
			szMsg[0] = 0;
		}
		else {
			sprintf_s(szMsg, "Unable to target, address = 0");
		}
	}
	if (szMsg[0])
		if (!gFilterTarget) WriteChatColor(szMsg, USERCOLOR_WHO);
	return;
}


// ***************************************************************************
// Function:    Skills
// Description: Our '/skills' command
//              Displays what your current skill levels are
// Usage:       /skills [skill name]
// ***************************************************************************
VOID Skills(PSPAWNINFO pChar, PCHAR szLine)
{
	DWORD Skill, SkillCount = 0;
	CHAR szMsg[MAX_STRING] = { 0 };
	bRunNextCommand = TRUE;
	PCHARINFO pCharInfo = NULL;
	CHAR szCopy[MAX_STRING] = { 0 };
	strcpy_s(szCopy, szLine);
	if (NULL == (pCharInfo = GetCharInfo())) return;
	if (szCopy[0] != 0)
		_strlwr_s(szCopy);
	WriteChatColor("Skills", USERCOLOR_DEFAULT);
	WriteChatColor("-----------------------", USERCOLOR_DEFAULT);
	if (PCHARINFO2 pChar2 = GetCharInfo2()) {
		for (Skill = 0; szSkills[Skill]; Skill++) {
			if (szCopy[0] != 0) {
				CHAR szName[MAX_STRING] = { 0 };
				strcpy_s(szName, szSkills[Skill]);
				_strlwr_s(szName);
				if (!strstr(szName, szCopy))
					continue;
			}
			SkillCount++;
			switch (pChar2->Skill[Skill]) {
			case 255:
				//Untrainable
				SkillCount--;
				break;
			case 254:
				//Can train
				sprintf_s(szMsg, "%s: Trainable", szSkills[Skill]);
				WriteChatColor(szMsg, USERCOLOR_DEFAULT);
				break;
			case 253:
				//Unknown
				sprintf_s(szMsg, "%s: Unknown(253)", szSkills[Skill]);
				WriteChatColor(szMsg, USERCOLOR_DEFAULT);
				break;
			default:
				//Have skill
				sprintf_s(szMsg, "%s: %d", szSkills[Skill], pChar2->Skill[Skill]);
				WriteChatColor(szMsg, USERCOLOR_DEFAULT);
			}
		}
	}
	if (SkillCount == 0) {
		sprintf_s(szMsg, "No skills matched '%s'.", szCopy);
		WriteChatColor(szMsg, USERCOLOR_DEFAULT);
	}
	else {
		DebugSpew("Skills - %d skills listed", SkillCount);
		sprintf_s(szMsg, "%d skills displayed.", SkillCount);
		WriteChatColor(szMsg, USERCOLOR_DEFAULT);
	}
}

// ***************************************************************************
// Function:    SetAutoRun
// Description: Our '/setautorun' command
//              Set autorun value
// Usage:       /setautorun [command]
// ***************************************************************************
VOID SetAutoRun(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szServerAndName[MAX_STRING] = { 0 };
	sprintf_s(szServerAndName, "%s.%s", EQADDR_SERVERNAME, ((PCHARINFO)pCharData)->Name);
	WritePrivateProfileStringA("AutoRun", szServerAndName, szLine, gszINIFilename);
	sprintf_s(szServerAndName, "Set autorun to: '%s'", szLine);
	WriteChatColor(szServerAndName, USERCOLOR_DEFAULT);
}

// ***************************************************************************
// Function:    Alias
// Description: Our '/alias' command
//              Add command aliases
// Usage:       /alias name [delete|command]
// ***************************************************************************

VOID Alias(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szBuffer[MAX_STRING] = { 0 };
	CHAR szName[MAX_STRING] = { 0 };
	PCHAR szCommand = NULL;
	GetArg(szName, szLine, 1);
	szCommand = GetNextArg(szLine);
	if (!_stricmp(szName, "list")) {
		PALIAS pLoop = pAliases;
		DWORD Count = 0;
		WriteChatColor("Aliases", USERCOLOR_WHO);
		WriteChatColor("--------------------------", USERCOLOR_WHO);
		while (pLoop) {
			sprintf_s(szName, "%s: %s", pLoop->szName, pLoop->szCommand);
			WriteChatColor(szName, USERCOLOR_WHO);
			Count++;
			pLoop = pLoop->pNext;
		}
		if (Count == 0) {
			WriteChatColor("No aliases defined.", USERCOLOR_WHO);
		}
		else {
			sprintf_s(szName, "%d alias%s displayed.", Count, (Count == 1) ? "" : "es");
			WriteChatColor(szName, USERCOLOR_WHO);
		}
		return;
	}
	if ((szName[0] == 0) || (szCommand[0] == 0)) {
		SyntaxError("Usage: /alias name [delete|command], or /alias list");
		return;
	}

	if (!_stricmp(szCommand, "delete")) {
		if (RemoveAlias(szName))
		{
			sprintf_s(szBuffer, "Alias '%s' deleted.", szName);
			RewriteAliases();
			WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		}
		else
		{
			sprintf_s(szBuffer, "Alias '%s' not found.", szName);
			WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		}
	}
	else {
		BOOL New = 1;
		if (RemoveAlias(szName))
			New = 0;
		AddAlias(szName, szCommand);
		sprintf_s(szBuffer, "Alias '%s' %sed.", szName, (New) ? "add" : "updat");
		WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		RewriteAliases();
	}
}

// ***************************************************************************
// Function:    Substitute
// Description: Our '/substitute' command
//              Add substitutions
// Usage:       /substitution <original> <new>
//                /substitution list
//                /substitition <original> delete
// ***************************************************************************

VOID Substitute(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szBuffer[MAX_STRING] = { 0 };
	CHAR szName[MAX_STRING] = { 0 };
	PCHAR szCommand = NULL;
	GetArg(szName, szLine, 1);
	szCommand = GetNextArg(szLine);
	if (!_stricmp(szName, "list")) {
		PSUB pLoop = pSubs;
		DWORD Count = 0;
		WriteChatColor("Substitutions", USERCOLOR_WHO);
		WriteChatColor("--------------------------", USERCOLOR_WHO);
		while (pLoop) {
			sprintf_s(szName, "%s\t----\t%s", pLoop->szOrig, pLoop->szSub);
			WriteChatColor(szName, USERCOLOR_WHO);
			Count++;
			pLoop = pLoop->pNext;
		}
		if (Count == 0) {
			WriteChatColor("No Substitutions defined.", USERCOLOR_WHO);
		}
		else {
			sprintf_s(szName, "%d substitution%s displayed.", Count, (Count == 1) ? "" : "s");
			WriteChatColor(szName, USERCOLOR_WHO);
		}
		return;
	}
	if ((szName[0] == 0) || (szCommand[0] == 0)) {
		SyntaxError("Usage: /substitute <orig> <new>, /substitute <orig> delete, or /substitute list");
		return;
	}

	if (!_stricmp(szCommand, "delete")) {
		if (RemoveSubstitute(szName))
		{
			sprintf_s(szBuffer, "Substitution for '%s' deleted.", szName);
			RewriteSubstitutions();
			WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		}
		else
		{
			sprintf_s(szBuffer, "Substitution for '%s' not found.", szName);
			WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		}
	}
	else {
		BOOL New = 1;
		if (RemoveSubstitute(szName))
			New = 0;
		AddSubstitute(szName, szCommand);
		sprintf_s(szBuffer, "Substitution for '%s' %sed.", szName, (New) ? "add" : "updat");
		WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		RewriteSubstitutions();
	}
}

// ***************************************************************************
// Function:   IniOutput our /ini command
// Description:   Outputs string data to an INI file using WritePrivateProfileString.
// If the inifile does'nt exist one will be created.
// Usage:     	
//
//	/ini "someini.ini" "the section" "NULL" "NULL"
//	adds a key named NULL and a value named NULL under the [the section]:
//	to remove the key named NULL:
//	/ini "someini.ini" "the section" "NULL" NULL
//	OR /ini "someini.ini" "the section" "NULL"
//	to remove section "the section":
//	/ini "someini.ini" "the section" NULL
//	OR /ini "someini.ini" "the section"
//
//	Basically leaving the third and/or fourth parameter blank will be interpreted as NULL
//	enclosing NULL in quotes will interpret it as an actual string "NULL"
// ***************************************************************************
VOID IniOutput(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szArg1[MAX_STRING] = { 0 };   //Filename
	CHAR szArg2[MAX_STRING] = { 0 };   //Section
	CHAR szArg3[MAX_STRING] = { 0 };   //Key
	CHAR szArg4[MAX_STRING] = { 0 };   //Data to write

	CHAR szOutput[MAX_STRING] = { 0 };  //Success / Error Output

	GetArg(szArg1, szLine, 1);
	GetArg(szArg2, szLine, 2);
	GetArg(szArg3, szLine, 3, 1);
	GetArg(szArg4, szLine, 4, 1);

	DebugSpew("/ini input -- %s %s %s %s", szArg1, szArg2, szArg3, szArg4);
	PCHAR pTemp = szArg1;
	while (pTemp[0])
	{
		if (pTemp[0] == '/')
			pTemp[0] = '\\';
		pTemp++;
	}

	if (szArg1[0] != '\\' && !strchr(szArg1, ':'))
	{
		sprintf_s(szOutput, "%s\\%s", gszMacroPath, szArg1);
		strcpy_s(szArg1, szOutput);
	}
	if (!strstr(szArg1, ".")) {
		strcat_s(szArg1, ".ini");
	}
	ZeroMemory(szOutput, MAX_STRING);

	void *Arg3 = (void *)&szArg3[0];
	void *Arg4 = (void *)&szArg4[0];

	if (!_strnicmp(szArg3, "NULL", 4) && strlen(szArg3) == 4) {
		Arg3 = 0;
	}
	if (!_strnicmp(szArg4, "NULL", 4) && strlen(szArg4) == 4) {
		Arg4 = 0;
	}
	//Lets strip the '"'
	if (Arg3) {
		GetArg(szArg3, szLine, 3);
	}
	if (Arg4) {
		GetArg(szArg4, szLine, 4);
	}
	if (!WritePrivateProfileString(szArg2, (char*)Arg3, (char*)Arg4, szArg1)) {
		sprintf_s(szOutput, "IniOutput ERROR -- during WritePrivateProfileString: %s", szLine);
		DebugSpew("%s", szOutput);
	}
	else {
		sprintf_s(szOutput, "IniOutput Write Successful!");
		DebugSpew("%s: %s", szOutput, szLine);
	}
}

// ***************************************************************************
// Function:        BankList
// Description:     Our /banklist command. Lists bank contents to chat buffer.
// 2003-08-30       Valerian
// ***************************************************************************
VOID BankList(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szTemp[MAX_STRING] = { 0 };
	PCHARINFO pCharInfo = NULL;
	PCONTENTS pContainer = NULL;
	if (NULL == (pCharInfo = GetCharInfo())) {
		MacroError("/banklist -- Bad offset: CharInfo");
		return;
	}
	WriteChatColor("Listing of Bank Inventory", USERCOLOR_DEFAULT);
	WriteChatColor("-------------------------", USERCOLOR_DEFAULT);
	char Link[MAX_STRING] = { 0 };
	for (int a = 0; a<NUM_BANK_SLOTS; a++) {
		if (pCharInfo->pBankArray) pContainer = pCharInfo->pBankArray->Bank[a];
		if (pContainer) {
			GetItemLink(pContainer, Link);
			sprintf_s(szTemp, "Slot %d: %dx %s (%s)", a, pContainer->StackCount ? pContainer->StackCount : 1, Link, GetItemFromContents(pContainer)->LoreName);
			WriteChatColor(szTemp, USERCOLOR_DEFAULT);
			if (pContainer->Contents.ContainedItems.pItems)
			{
				for (int b = 0; b<GetItemFromContents(pContainer)->Slots; b++) {
					if (pContainer->Contents.ContainedItems.pItems->Item[b]) {
						GetItemLink(pContainer->Contents.ContainedItems.pItems->Item[b], Link);
						sprintf_s(szTemp, "- Slot %d: %dx %s (%s)", b, pContainer->Contents.ContainedItems.pItems->Item[b]->StackCount ? pContainer->Contents.ContainedItems.pItems->Item[b]->StackCount : 1, Link, GetItemFromContents(pContainer->Contents.ContainedItems.pItems->Item[b])->LoreName);
						WriteChatColor(szTemp, USERCOLOR_DEFAULT);
					}
				}
			}
		}
	}
}

// ***************************************************************************
// Function:      WindowState
// Description:      Our /windowstate command. Toggles windows open/closed.
// ***************************************************************************
VOID WindowState(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR Arg1[MAX_STRING] = { 0 };
	CHAR Arg2[MAX_STRING] = { 0 };
	GetArg(Arg1, szLine, 1);
	GetArg(Arg2, szLine, 2);
	if (PCSIDLWND pWnd = (PCSIDLWND)FindMQ2Window(Arg1))
	{
		DWORD ShowWindow = (DWORD)pWnd->pvfTable->ShowWindow;
		CHAR szBuffer[MAX_STRING] = { 0 };
		bool State = pWnd->dShow;
		if (!_stricmp(Arg2, "open")) State = 1;
		if (!_stricmp(Arg2, "close")) State = 0;
		//if (pWnd->dShow == State) State = 99;
		switch (State) {
		case 0:
			((CXWnd*)pWnd)->Show(0, 1);
			sprintf_s(szBuffer, "Window '%s' is now closed.", pWnd->WindowText ? pWnd->WindowText->Text : Arg1);
			((CSidlScreenWnd*)pWnd)->StoreIniVis();
			break;
		case 1:
			__asm {
				push ecx;
				mov ecx, [pWnd];
				call dword ptr[ShowWindow];
				pop ecx;
			}
			sprintf_s(szBuffer, "Window '%s' is now open.", pWnd->WindowText ? pWnd->WindowText->Text : Arg1);
			((CSidlScreenWnd*)pWnd)->StoreIniVis();
			break;
		}
		WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
		return;
	}
	SyntaxError("Usage: /windowstate <window> [open|close]");
}


// ***************************************************************************
// Function:      DisplayLoginName
// Description:   Our /loginname command.
// ***************************************************************************
VOID DisplayLoginName(PSPAWNINFO pChar, PCHAR szLine)
{
	PCHAR szLogin = GetLoginName();
	if (!szLogin) {
		MacroError("Unable to retrieve login name.");
	}
	else {
		WriteChatf("Login name: \ay%s\ax", szLogin);
	}
}

VOID EQDestroyHeldItemOrMoney(PSPAWNINFO pChar, PCHAR szLine)
{
	(pPCData)->DestroyHeldItemOrMoney();
}

VOID Exec(PSPAWNINFO pChar, PCHAR szLine) {
	CHAR exepath[MAX_STRING] = { 0 };
	CHAR szTemp1[MAX_STRING] = { 0 };
	CHAR szTemp2[MAX_STRING] = { 0 };
	CHAR szTemp3[MAX_STRING] = { 0 };
	GetArg(szTemp1, szLine, 1);
	GetArg(szTemp2, szLine, 2);
	GetArg(szTemp3, szLine, 3);

	if (szTemp1[0] != 0 && szTemp2[0] != 0) {
		WriteChatf("Opening %s %s %s", szTemp1, szTemp2, szTemp3);

		GetPrivateProfileString("Application Paths", szTemp1, szTemp1, exepath, MAX_STRING, gszINIFilename);

		if (!strcmp(szTemp2, "bg")) {
			ShellExecute(NULL, "open", exepath, NULL, NULL, SW_SHOWMINNOACTIVE);
		}
		else if (!strcmp(szTemp2, "fg")) {
			ShellExecute(NULL, "open", exepath, NULL, NULL, SW_SHOWNOACTIVATE);
		}
		else if (!strcmp(szTemp3, "bg")) {
			ShellExecute(NULL, "open", exepath, szTemp2, NULL, SW_SHOWMINNOACTIVE);
		}
		else if (!strcmp(szTemp3, "fg")) {
			ShellExecute(NULL, "open", exepath, szTemp2, NULL, SW_SHOWNOACTIVATE);
		}
	}
	else {
		WriteChatColor("/exec [application \"parameters\"] [fg | bg]", USERCOLOR_DEFAULT);
	}
}

// /keypress
VOID DoMappable(PSPAWNINFO pChar, PCHAR szLine)
{
	if (szLine[0] == 0)
	{
		SyntaxError("Usage: /keypress <eqcommand|keycombo> [hold|chat]");
		return;
	}
	CHAR szArg1[MAX_STRING] = { 0 };
	CHAR szArg2[MAX_STRING] = { 0 };

	GetArg(szArg1, szLine, 1);
	GetArg(szArg2, szLine, 2);
	BOOL Hold = (_stricmp(szArg2, "hold") == 0);


	if (!PressMQ2KeyBind(szArg1, Hold))
	{
		int N = FindMappableCommand(szArg1);
		if (N >= 0)
		{
			//ok if the user issues a movement keypress like "FORWARD" here and at charselect he ctd
			//lets see if we can fix that -eqmule
			if (EQbCommandStates[N] == FALSE) {
				MacroError("%s is disabled right now Gamestate is %d", szArg1, GetGameState());
				return;
			}
			ExecuteCmd(N, 1, 0);
			if (!Hold)
				ExecuteCmd(N, 0, 0);
			return;
		}
		KeyCombo Temp;
		if (ParseKeyCombo(szArg1, Temp))
		{
			if (!_stricmp(szArg2, "chat"))
			{
				if (Temp.Data[3] != 0x92) {
					pWndMgr->HandleKeyboardMsg(Temp.Data[3], 1);
					pWndMgr->HandleKeyboardMsg(Temp.Data[3], 0);
				}
				else {
					// ugly ass hack -- the ':' char no longer 
					// seems to be handled independently.  simulate
					// a shift and a ;
					pWndMgr->HandleKeyboardMsg(0x2a, 1);
					pWndMgr->HandleKeyboardMsg(0x27, 1);
					pWndMgr->HandleKeyboardMsg(0x27, 0);
					pWndMgr->HandleKeyboardMsg(0x2a, 0);
				}
			}
			else
			{
				MQ2HandleKeyDown(Temp);
				if (!Hold)
					MQ2HandleKeyUp(Temp);
			}
			return;
		}

		MacroError("Invalid mappable command or key combo '%s'", szArg1);
		return;
	}
}

// /popup
VOID PopupText(PSPAWNINFO pChar, PCHAR szLine)
{
	DisplayOverlayText(szLine, CONCOLOR_LIGHTBLUE, 100, 500, 500, 3000);
}

// /popcustom
void PopupTextCustom(PSPAWNINFO pChar, char* szLine)
{
	CustomPopup(szLine, false);
}

// /popupecho
void PopupTextEcho(PSPAWNINFO pChar, char* szLine)
{
	CustomPopup(szLine, true);
}

// /multiline
VOID MultilineCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	if (szLine[0] == 0)
	{
		SyntaxError("Usage: /multiline <delimiter> <command>[delimiter<command>[delimiter<command>[. . .]]]");
		return;
	}
	CHAR szArg[MAX_STRING] = { 0 }; // delimiter(s)
	GetArg(szArg, szLine, 1);
	PCHAR szRest = GetNextArg(szLine);
	if (!szRest[0])
		return;
	CHAR Copy[MAX_STRING], szCmd[MAX_STRING] = { 0 };
	strcpy_s(Copy, szRest);// dont destroy original...
	char *token1 = NULL;
	char *next_token1 = NULL;

	token1 = strtok_s(Copy, szArg, &next_token1);
	while (token1 != NULL)
	{
		if (token1 != NULL)
		{
			strcpy_s(szCmd, token1);
			DoCommand(pChar, szCmd);
			token1 = strtok_s(NULL, szArg, &next_token1);
		}
	}
}

// /ranged
VOID do_ranged(PSPAWNINFO pChar, PCHAR szLine)
{
	EQPlayer *pRangedTarget = pTarget;
	if (szLine[0])
	{
		pRangedTarget = GetSpawnByID(atoi(szLine));
		if (!pRangedTarget)
		{
			MacroError("Invalid spawn ID.  Use /ranged with no parameters, or with a spawn ID");
			return;
		}
	}
	if (!pRangedTarget)
	{
		MacroError("No target for ranged attack");
		return;
	}
	AttackRanged(pRangedTarget);
}

// /loadcfg
VOID LoadCfgCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage: /loadcfg <filename>");
		return;
	}

	if (LoadCfgFile(szLine, false))
		return;
	MacroError("Could not /loadcfg '%s'", szLine);
}

// /dumpbinds
VOID DumpBindsCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage /dumpbinds <filename>");
		return;
	}
	CHAR szBuffer[MAX_STRING] = { 0 };
	if (!DumpBinds(szLine))
	{
		MacroError("Could not dump binds to %s", szLine);
		return;
	}
	WriteChatColor("Binds dumped to file.");
}


// /docommand
VOID DoCommandCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage: /docommand <command>");
		return;
	}
	DoCommand(pChar, szLine);
}

// /alt
VOID DoAltCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage: /alt <command>");
		return;
	}
	bool Old = ((PCXWNDMGR)pWndMgr)->KeyboardFlags[2];
	((PCXWNDMGR)pWndMgr)->KeyboardFlags[2] = 1;
	DoCommand(pChar, szLine);
	((PCXWNDMGR)pWndMgr)->KeyboardFlags[2] = Old;
}

// /shift
VOID DoShiftCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage: /shift <command>");
		return;
	}
	bool Old = ((PCXWNDMGR)pWndMgr)->KeyboardFlags[0];
	((PCXWNDMGR)pWndMgr)->KeyboardFlags[0] = 1;
	gShiftKeyDown = 1;
	DoCommand(pChar, szLine);
	gShiftKeyDown = 0;
	((PCXWNDMGR)pWndMgr)->KeyboardFlags[0] = Old;
}

// /ctrl
VOID DoCtrlCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage: /ctrl <command>");
		return;
	}
	bool Old = ((PCXWNDMGR)pWndMgr)->KeyboardFlags[1];
	((PCXWNDMGR)pWndMgr)->KeyboardFlags[1] = 1;
	DoCommand(pChar, szLine);
	((PCXWNDMGR)pWndMgr)->KeyboardFlags[1] = Old;
}

VOID NoModKeyCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage: /nomodkey <command>");
		return;
	}
	bool KeyboardFlags[4];
	*(DWORD*)&KeyboardFlags = *(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags;
	*(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags = 0;
	DoCommand(pChar, szLine);
	*(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags = *(DWORD*)&KeyboardFlags;
}
#endif
// ***************************************************************************
// Function:    UseItemCmd
// Description: '/useitem' command
//              Activates an item that has a clicky effect.
// Usage:       /useitem 1 0 or /useitem "item name" or /useitem item name
// ***************************************************************************
#ifdef ISXEQ
int CMD_UseItem(int argc, char *argv[])
{
	PSPAWNINFO pChar = (PSPAWNINFO)pLocalPlayer;
	CHAR szTemp[MAX_STRING] = { 0 };
	PCHAR szLine = ISXEQArgToMQ2Arg(argc, argv, szTemp, MAX_STRING);
	if (!cmdUseItem) {
		PCMDLIST pCmdListOrig = (PCMDLIST)EQADDR_CMDLIST;
		for (int i = 0; pCmdListOrig[i].fAddress != 0; i++) {
			if (!strcmp(pCmdListOrig[i].szName, "/useitem")) {
				cmdUseItem = (fEQCommand)pCmdListOrig[i].fAddress;
			}
		}
	}
	if (!cmdUseItem)
		return -1;
#else
VOID UseItemCmd(PSPAWNINFO pChar, PCHAR szLine)
{
#endif
	CHAR szCmd[MAX_STRING] = { 0 };
	strcpy_s(szCmd, szLine);
	if (!szCmd[0])
	{
		WriteChatColor("Usage: /useitem \"item name\"");
		cmdUseItem(pChar, szCmd);
		RETURN(0);
	}
	else {
		CHAR szSlot1[MAX_STRING] = { 0 };
		GetArg(szSlot1, szCmd, 1);
		bool stripped = StripQuotes(szCmd);
		if (IsNumber(szSlot1)) {
			cmdUseItem(pChar, szCmd);
		}
		else {
			if (PCONTENTS pItem = FindItemByName(szCmd, stripped)) {
				bool bKeyring = false;
#ifndef EMU
				if (PCHARINFO pCharInfo = GetCharInfo()) {
					if (CharacterBase *cb = (CharacterBase *)&pCharInfo->pCharacterBase) {
						ItemGlobalIndex location;
						location.Location = (ItemContainerInstance)pItem->GlobalIndex.Location;
						location.Index.Slot1 = pItem->GlobalIndex.Index.Slot1;
						location.Index.Slot2 = pItem->GlobalIndex.Index.Slot2;
						location.Index.Slot3 = -1;
						bKeyring = location.IsKeyRingLocation();
					}
				}
#endif
				if (!bKeyring) {
					CHAR szTemp[32] = { 0 };
					sprintf_s(szTemp, "%d %d", pItem->GlobalIndex.Index.Slot1, pItem->GlobalIndex.Index.Slot2);
					cmdUseItem(pChar, szTemp);
					RETURN(0);
				}
#ifndef EMU		
				bool bMount = ((EQ_Item*)pItem)->IsKeyRingItem(eMount);
				bool bIllusion = ((EQ_Item*)pItem)->IsKeyRingItem(eIllusion);
				bool bFamiliar = ((EQ_Item*)pItem)->IsKeyRingItem(eFamiliar);

				//is it a mount?
				if (bMount) {
					if (DWORD index = GetKeyRingIndex(0, szCmd, sizeof(szCmd), stripped, true)) {
						if (CXWnd *krwnd = FindMQ2Window(KeyRingWindowParent)) {
							if (CListWnd *clist = (CListWnd*)krwnd->GetChildItem(MountWindowList)) {
								if (DWORD numitems = ((CSidlScreenWnd*)clist)->Items) {
									SendListSelect(KeyRingWindowParent, MountWindowList, index - 1);
									int listdata = clist->GetItemData(index - 1);
									cmdToggleKeyRingItem(0, &pItem, listdata);
									RETURN(0);
								}
							}
						}
					}
				}
				else if (bIllusion) {
					//uhm ok, maybe an illusion then?
					if (DWORD index = GetKeyRingIndex(1, szCmd, sizeof(szCmd), stripped, true)) {
						if (CXWnd *krwnd = FindMQ2Window(KeyRingWindowParent)) {
							if (CListWnd *clist = (CListWnd*)krwnd->GetChildItem(IllusionWindowList)) {
								if (DWORD numitems = ((CSidlScreenWnd*)clist)->Items) {
									SendListSelect(KeyRingWindowParent, IllusionWindowList, index - 1);
									int listdata = clist->GetItemData(index - 1);
									cmdToggleKeyRingItem(1, &pItem, listdata);
									Sleep(0);
								}
							}
						}
					}	
				}
				else if (bFamiliar) {
					//uhm ok, maybe a Familiar then?
					if (DWORD index = GetKeyRingIndex(2, szCmd, sizeof(szCmd), stripped, true)) {
						if (CXWnd *krwnd = FindMQ2Window(KeyRingWindowParent)) {
							if (CListWnd *clist = (CListWnd*)krwnd->GetChildItem(FamiliarWindowList)) {
								if (DWORD numitems = ((CSidlScreenWnd*)clist)->Items) {
									SendListSelect(KeyRingWindowParent, FamiliarWindowList, index - 1);
									int listdata = clist->GetItemData(index - 1);
									cmdToggleKeyRingItem(2, &pItem, listdata);
									Sleep(0);
								}
							}
						}
					}
				}
#endif
			}
		}
	}
	RETURN(0);
}
#ifndef ISXEQ
// ***************************************************************************
// Function:    DoSocial
// Description: '/dosocial' command
//              Does (or lists) your programmed socials
// Usage:       /dosocial [list|"social name"]
// ***************************************************************************
VOID DoSocial(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!pSocialList) return;

	DWORD SocialIndex = -1, LineIndex;
	DWORD SocialPage = 0, SocialNum = 0;
	CHAR szBuffer[MAX_STRING] = { 0 };
	BOOL displayUsage = FALSE;

	GetArg(szBuffer, szLine, 1);

	if (!_stricmp(szBuffer, "list")) {
		WriteChatColor("Socials: (page,number) name", USERCOLOR_DEFAULT);
		for (SocialIndex = 0; SocialIndex < 120; SocialIndex++) {
			SocialPage = SocialIndex / 12;
			SocialNum = SocialIndex - (SocialPage * 12);
			if (strlen(pSocialList[SocialIndex].Name)) {
				sprintf_s(szBuffer, "(%2d,%2d) %s ", SocialPage + 1, SocialNum + 1, pSocialList[SocialIndex].Name);
				WriteChatColor(szBuffer, USERCOLOR_ECHO_EMOTE);
				for (LineIndex = 0; LineIndex < 5; LineIndex++) {
					if (strlen(pSocialList[SocialIndex].Line[LineIndex])) {
						sprintf_s(szBuffer, "  %d: %s", LineIndex + 1, pSocialList[SocialIndex].Line[LineIndex]);
						WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
					}
				}
			}
		}
		return;
	}
	else if (strlen(szBuffer)) { /* assume we have a social name to match */
		for (unsigned long N = 0; N < 120; N++) {
			if (!_stricmp(szBuffer, pSocialList[N].Name))
			{
				SocialIndex = N;
				break;
			}
		}
	}

	if (gCurrentSocial == -1)
	{
		gCurrentSocial = SocialIndex;
	}
	else
	{
		if (SocialIndex < 120) {
			for (LineIndex = 0; LineIndex < 5; LineIndex++) {
				if (strlen(pSocialList[SocialIndex].Line[LineIndex])) DoCommand(pChar, pSocialList[SocialIndex].Line[LineIndex]);
			}
		}
		else {
			if (strlen(szLine)) {
				sprintf_s(szBuffer, "Invalid Argument(s): %s", szLine);
				WriteChatColor(szBuffer, USERCOLOR_DEFAULT);
			}
			SyntaxError("Usage: /dosocial <list|\"social name\">", USERCOLOR_DEFAULT);
		}
	}
}

// /timed
VOID DoTimedCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage: /timed <deciseconds> <command>");
		return;
	}
	CHAR szArg[MAX_STRING] = { 0 }; // delay
	GetArg(szArg, szLine, 1);
	PCHAR szRest = GetNextArg(szLine);
	if (!szRest[0])
		return;
	TimedCommand(szRest, atoi(szArg) * 100);
}

VOID ClearErrorsCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	gszLastNormalError[0] = 0;
	gszLastSyntaxError[0] = 0;
	gszLastMQ2DataError[0] = 0;
}

VOID CombineCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage: /combine <pack>");
		return;
	}
	CXWnd *pWnd = FindMQ2Window(szLine);
	if (!pWnd)
	{
		MacroError("Window '%s' not open", szLine);
		return;
	}
	if ((DWORD)pWnd->pvfTable != CContainerWnd__vftable)
	{
		MacroError("Window '%s' not container window", szLine);
		return;
	}
	((CContainerWnd*)pWnd)->HandleCombine();
}

VOID DropCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (PCHARINFO2 pChar2 = GetCharInfo2()) {
		if (pChar2->pInventoryArray && pChar2->pInventoryArray->Inventory.Cursor)
		{
			if (((EQ_Item*)pChar2->pInventoryArray->Inventory.Cursor)->CanDrop(0, 1))
			{
				pEverQuest->DropHeldItemOnGround(1);
			}
		}
	}
}
class mercenary
{
public:
	void SelectMercenaryType();
};
FUNCTION_AT_ADDRESS(void mercenary::SelectMercenaryType(void), EQ_Character__Max_Mana);
VOID HudCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage: /hud <normal|underui|always>");
		WriteChatColor("Note: 'always' forces 'underui' also. The Network Status indicator is not 'always' drawn and is toggled with F11.");
		return;
	}
	else
		if (!_stricmp(szLine, "normal"))
		{
			WritePrivateProfileString("MacroQuest", "HUDMode", "Normal", gszINIFilename);
			gbAlwaysDrawMQHUD = false;
			gbHUDUnderUI = false;
		}
		else
			if (!_stricmp(szLine, "underui"))
			{
				WritePrivateProfileString("MacroQuest", "HUDMode", "UnderUI", gszINIFilename);
				gbHUDUnderUI = true;
				gbAlwaysDrawMQHUD = false;
			}
			else
				if (!_stricmp(szLine, "always"))
				{
					WritePrivateProfileString("MacroQuest", "HUDMode", "Always", gszINIFilename);
					gbHUDUnderUI = true;
					gbAlwaysDrawMQHUD = true;
				}
}

VOID CaptionCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR Arg1[MAX_STRING] = { 0 };
	GetArg(Arg1, szLine, 1);
	if (!Arg1[0])
	{
		SyntaxError("Usage: /caption <list|type <value>|update #|MQCaptions <on|off>|Anonymize <on|off>>");
		return;
	}
	if (!_stricmp(Arg1, "list"))
	{
		WriteChatf("\ayPlayer1\ax: \ag%s\ax", gszSpawnPlayerName[1]);
		WriteChatf("\ayPlayer2\ax: \ag%s\ax", gszSpawnPlayerName[2]);
		WriteChatf("\ayPlayer3\ax: \ag%s\ax", gszSpawnPlayerName[3]);
		WriteChatf("\ayPlayer4\ax: \ag%s\ax", gszSpawnPlayerName[4]);
		WriteChatf("\ayPlayer5\ax: \ag%s\ax", gszSpawnPlayerName[5]);
		WriteChatf("\ayPlayer6\ax: \ag%s\ax", gszSpawnPlayerName[6]);

		WriteChatf("\ayNPC\ax: \ag%s\ax", gszSpawnNPCName);
		WriteChatf("\ayPet\ax: \ag%s\ax", gszSpawnPetName);
		WriteChatf("\ayCorpse\ax: \ag%s\ax", gszSpawnCorpseName);
		return;
	}
	PCHAR pCaption = 0;
	if (!_stricmp(Arg1, "Player1"))
	{
		pCaption = gszSpawnPlayerName[1];
	}
	else if (!_stricmp(Arg1, "Player2"))
	{
		pCaption = gszSpawnPlayerName[2];
	}
	else if (!_stricmp(Arg1, "Player3"))
	{
		pCaption = gszSpawnPlayerName[3];
	}
	else if (!_stricmp(Arg1, "Player4"))
	{
		pCaption = gszSpawnPlayerName[4];
	}
	else if (!_stricmp(Arg1, "Player5"))
	{
		pCaption = gszSpawnPlayerName[5];
	}
	else if (!_stricmp(Arg1, "Player6"))
	{
		pCaption = gszSpawnPlayerName[6];
	}
	else if (!_stricmp(Arg1, "Pet"))
	{
		pCaption = gszSpawnPetName;
	}
	else if (!_stricmp(Arg1, "NPC"))
	{
		pCaption = gszSpawnNPCName;
	}
	else if (!_stricmp(Arg1, "Corpse"))
	{
		pCaption = gszSpawnCorpseName;
	}
	else if (!_stricmp(Arg1, "Update"))
	{
		gMaxSpawnCaptions = atoi(GetNextArg(szLine));
		if (gMaxSpawnCaptions<8)
			gMaxSpawnCaptions = 8;
		if (gMaxSpawnCaptions>70)
			gMaxSpawnCaptions = 70;
		_itoa_s(gMaxSpawnCaptions, Arg1, 10);
		WritePrivateProfileString("Captions", "Update", Arg1, gszINIFilename);
		WriteChatf("\ay%d\ax nearest spawns will have their caption updated each pass.", gMaxSpawnCaptions);
		return;
	}
	else if (!_stricmp(Arg1, "MQCaptions"))
	{
		gMQCaptions = (!_stricmp(GetNextArg(szLine), "On"));
		WritePrivateProfileString("Captions", "MQCaptions", (gMQCaptions ? "1" : "0"), gszINIFilename);
		WriteChatf("MQCaptions are now \ay%s\ax.", (gMQCaptions ? "On" : "Off"));
		return;
	}
	else if (!_stricmp(Arg1, "Anon"))
	{
		gAnonymize = (!_stricmp(GetNextArg(szLine), "On"));
		WritePrivateProfileString("Captions", "Anonymize", (gAnonymize ? "1" : "0"), gszINIFilename);
		WriteChatf("Anonymize is now \ay%s\ax.", (gAnonymize ? "On" : "Off"));
		return;
	}
	else
	{
		MacroError("Invalid caption type '%s'", Arg1);
		return;
	}
	strcpy_s(pCaption, MAX_STRING, GetNextArg(szLine));
	WritePrivateProfileString("Captions", Arg1, pCaption, gszINIFilename);
	ConvertCR(pCaption, MAX_STRING);
	WriteChatf("\ay%s\ax caption set.", Arg1);
}

VOID NoParseCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0])
	{
		SyntaxError("Usage: /noparse <command>");
		return;
	}
	bAllowCommandParse = false;
	DoCommand(pChar, szLine);
	bAllowCommandParse = true;
}

VOID AltAbility(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szBuffer[MAX_STRING] = { 0 };
	CHAR szCommand[MAX_STRING] = { 0 };
	CHAR szSpellInfo[MAX_STRING] = { 0 };
	PCHAR szName = NULL;
	GetArg(szCommand, szLine, 1);
	szName = GetNextArg(szLine);
	unsigned long nAbility = 0;
	int i = 0;
	MQ2TicksType szTime;

	if ((szName[0] == 0) || (szCommand[0] == 0)) {
		SyntaxError("Usage: /aa list [all|timers], /aa info [ability name], or /aa act [ability name]");
		return;
	}

	if (!_stricmp(szCommand, "list"))
	{
		if (!_stricmp(szName, "all"))
		{
			WriteChatColor("Alternative Abilities (Complete List)", CONCOLOR_YELLOW);
			WriteChatColor("-------------------------------------", USERCOLOR_WHO);
			for (nAbility = 0; nAbility<AA_CHAR_MAX_REAL; nAbility++) {
				if (PALTABILITY pAbility = GetAAByIdWrapper(pPCData->GetAlternateAbilityId(nAbility))) {
					sprintf_s(szBuffer, "[ %d: %s ]", pAbility->ID,
						pCDBStr->GetString(pAbility->nName, 1, NULL));
					WriteChatColor(szBuffer, USERCOLOR_WHO);
				}
			}
		}
		else if (!_stricmp(szName, "timers"))
		{
			WriteChatColor("Alternative Abilities With Timers", CONCOLOR_YELLOW);
			WriteChatColor("---------------------------------", USERCOLOR_WHO);
			for (nAbility = 0; nAbility<AA_CHAR_MAX_REAL; nAbility++) {
				if (PALTABILITY pAbility = GetAAByIdWrapper(pPCData->GetAlternateAbilityId(nAbility))) {
					if ((pAltAdvManager->GetCalculatedTimer(pPCData, pAbility)) > 0)
					{
						if (pAltAdvManager->IsAbilityReady(pPCData, pAbility, 0))
						{
							sprintf_s(szBuffer, "[ %d: %s ] (Reuse Time: %d seconds) <Ready>",
								pAbility->ID, pCDBStr->GetString(pAbility->nName, 1, NULL),
								pAltAdvManager->GetCalculatedTimer(pPCData, pAbility));
							WriteChatColor(szBuffer, USERCOLOR_WHO);
						}
						else
						{
							pAltAdvManager->IsAbilityReady(pPCData, pAbility, &i);
							sprintf_s(szBuffer, "[ %d: %s ] (Reuse Time: %d seconds) <Ready in %d seconds>",
								pAbility->ID, pCDBStr->GetString(pAbility->nName, 1, NULL),
								pAltAdvManager->GetCalculatedTimer(pPCData, pAbility), i);
							WriteChatColor(szBuffer, USERCOLOR_WHO);
						}
					}
				}
			}
		}
		else
		{
			SyntaxError("Usage: /aa list [all|timers], /aa info [ability name], or /aa act [ability name]");
			return;
		}
	}
	else if (!_stricmp(szCommand, "info"))
	{
		for (unsigned long nAbility = 0; nAbility<NUM_ALT_ABILITIES; nAbility++)
		{
			if (PALTABILITY pAbility = GetAAByIdWrapper(nAbility))
			{
				char *pName;
				if (!_stricmp(pName = pCDBStr->GetString(pAbility->nName, 1, NULL), szName))
				{

					WriteChatColor("Alternative Advancement Ability Information", CONCOLOR_YELLOW);
					WriteChatColor("-------------------------------------------", USERCOLOR_WHO);

					if ((pAltAdvManager->GetCalculatedTimer(pPCData, pAbility)) > 0)
					{//has a timer
						if (!pAltAdvManager->IsAbilityReady(pPCData, pAbility, 0))
						{//it's not ready
							sprintf_s(szBuffer, "[ %d: %s ] %s", pAbility->ID, pName, pCDBStr->GetString(pAbility->nName, 4, NULL));
							WriteChatColor(szBuffer, USERCOLOR_WHO);
							sprintf_s(szBuffer, "Min Level: %d, Cost: %d, Max Rank: %d, Type: %d, Reuse Time: %d seconds",
								pAbility->MinLevel, pAbility->Cost, pAbility->MaxRank, pAbility->Type, pAltAdvManager->GetCalculatedTimer(pPCData, pAbility));
							WriteChatColor(szBuffer, USERCOLOR_WHO);
							if (pAbility->SpellID > 0)
							{
								sprintf_s(szBuffer, "Casts Spell: %s", GetSpellNameByID(pAbility->SpellID));
								WriteChatColor(szBuffer, USERCOLOR_WHO);
							}
							if (PlayerHasAAAbility(pAbility->Index))
							{
								sprintf_s(szBuffer, "Ready: No (%d seconds until refresh)", i);
								WriteChatColor(szBuffer, USERCOLOR_WHO);
							}
							else
							{
								WriteChatColor("Ready: Not Purchased", USERCOLOR_WHO);
							}

						}
						else
						{
							sprintf_s(szBuffer, "[ %d: %s ] %s", pAbility->ID, pName, pCDBStr->GetString(pAbility->nName, 4, NULL));
							WriteChatColor(szBuffer, USERCOLOR_WHO);
							sprintf_s(szBuffer, "Min Level: %d, Cost: %d, Max Rank: %d, Type: %d, Reuse Time: %d seconds",
								pAbility->MinLevel, pAbility->Cost, pAbility->MaxRank, pAbility->Type, pAltAdvManager->GetCalculatedTimer(pPCData, pAbility));
							WriteChatColor(szBuffer, USERCOLOR_WHO);
							if (pAbility->SpellID > 0)
							{
								sprintf_s(szBuffer, "Casts Spell: %s", GetSpellNameByID(pAbility->SpellID));
								WriteChatColor(szBuffer, USERCOLOR_WHO);
							}
							sprintf_s(szBuffer, "Ready: Yes");
							WriteChatColor(szBuffer, USERCOLOR_WHO);
						}
					}
					else
					{
						pAltAdvManager->IsAbilityReady(pPCData, pAbility, &i);
						sprintf_s(szBuffer, "[ %d: %s ] %s", pAbility->ID, pName, pCDBStr->GetString(pAbility->nName, 4, NULL));
						WriteChatColor(szBuffer, USERCOLOR_WHO);
						sprintf_s(szBuffer, "Min Level: %d, Cost: %d, Max Rank: %d, Type: %d",
							pAbility->MinLevel, pAbility->Cost, pAbility->MaxRank, pAbility->Type);
						WriteChatColor(szBuffer, USERCOLOR_WHO);
						if (pAbility->SpellID > 0)
						{
							sprintf_s(szBuffer, "Casts Spell: %s", GetSpellNameByID(pAbility->SpellID));
							WriteChatColor(szBuffer, USERCOLOR_WHO);
						}
					}
				} // name matches
			} // if pability != null
		} //for loop
	}
	else if (!_stricmp(szCommand, "act"))
	{
		//we want to get the rank thats for our level here
		int level = -1;
		if (PSPAWNINFO pMe = (PSPAWNINFO)pLocalPlayer) {
			level = pMe->Level;
		}
		// only search through the ones we have...
		for (unsigned long nAbility = 0; nAbility<AA_CHAR_MAX_REAL; nAbility++) {
			if (PALTABILITY pAbility = GetAAByIdWrapper(pPCData->GetAlternateAbilityId(nAbility), level)) {
				if (PCHAR pName = pCDBStr->GetString(pAbility->nName, 1, NULL)) {
					if (!_stricmp(szName, pName)) {
						sprintf_s(szBuffer, "/alt act %d", pAbility->ID);
						DoCommand(pChar, szBuffer);
						break;
					}
				}
			}
		}
	}
	else
	{
		SyntaxError("Usage: /aa list [all|timers|ready], /aa info [ability name], or /aa act [ability name]");
		return;
	}
	return;
}

// ***************************************************************************
// Function:    Echo
// Description: Our '/echo' command
//              Echos text to the chatbox
// Usage:       /echo <text>
// ***************************************************************************
VOID Echo(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szEcho[MAX_STRING] = { 0 };
	bRunNextCommand = TRUE;
	strcpy_s(szEcho, DebugHeader);
	strcat_s(szEcho, " ");
	int NewPos = strlen(DebugHeader)+1, OldPos = 0;
	if (szLine) {
		while (OldPos < (int)strlen(szLine)) {
			if (szLine[OldPos] == '\\') {
				OldPos++;
				if (szLine[OldPos]) {
					if (szLine[OldPos] == '\\')
						szEcho[NewPos] = szLine[OldPos];
					else if (szLine[OldPos] == 'n') {
						szEcho[NewPos++] = '\r';
						szEcho[NewPos] = '\n';
					}
					else if (szLine[OldPos] >= 'a' && szLine[OldPos] <= 'z')
						szEcho[NewPos] = szLine[OldPos] - 'a' + 7;
					else if (szLine[OldPos] >= 'A' && szLine[OldPos] <= 'Z')
						szEcho[NewPos] = szLine[OldPos] - 'a' + 7;
					NewPos++;
					OldPos++;
				}
			}
			else {
				szEcho[NewPos] = szLine[OldPos];
				NewPos++;
				OldPos++;
			}
			szEcho[NewPos] = 0;
		}
	}
	WriteChatColor(szEcho,USERCOLOR_CHAT_CHANNEL);
}

// ***************************************************************************
// Function:    LootAll
// Description: Our '/lootall' command
//              Loots everything on the targeted corpse
// Usage:       /lootall
// ***************************************************************************
VOID LootAll(PSPAWNINFO pChar, PCHAR szLine)
{
	pLootWnd->LootAll(1);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD procid = 0;
	GetWindowThreadProcessId(hwnd, &procid);
	if (procid == *(LPARAM *)lParam) {
		CHAR szClass[2048] = { 0 };
		GetClassName(hwnd, szClass, 2047);
		if (!_stricmp(szClass, "_EverQuestwndclass")) {
			*(LPARAM *)lParam = (LPARAM)hwnd;
			return FALSE;
		}
	}
	return TRUE;
}

// ***************************************************************************
// Function:    SetWinTitle
// Description: Our '/setwintitle' command
//              Set the Window Title
// Usage:       /setwintitle <something something>
// ***************************************************************************
VOID SetWinTitle(PSPAWNINFO pChar, PCHAR szLine)
{
	DWORD lReturn = GetCurrentProcessId();
	DWORD pid = lReturn;
	BOOL ret = EnumWindows(EnumWindowsProc, (LPARAM)&lReturn);
	if (lReturn != pid) {
		if (szLine && szLine[0] != '\0') {
			SetWindowText((HWND)lReturn, szLine);
		}
	}
}
VOID GetWinTitle(PSPAWNINFO pChar, PCHAR szLine)
{
	BOOL bHide = atoi(szLine);
	szLine[0] = '\0';
	DWORD lReturn = GetCurrentProcessId();
	DWORD pid = lReturn;
	BOOL ret = EnumWindows(EnumWindowsProc, (LPARAM)&lReturn);
	if (lReturn != pid) {
		if (GetWindowText((HWND)lReturn, szLine, 255) && szLine[0] != '\0') {
			if (!bHide)
				WriteChatf("Window Title: \ay%s\ax", szLine);
		}
	}
}
// ***************************************************************************
// Function:    PetCmd
// Description: '/pet' command
//              Adds the ability to do /pet attack #id
// Usage:       /pet attack 1234 or whatever the spawnid is you want the pet to attack
// ***************************************************************************
VOID PetCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0]) {
		WriteChatColor("Usage: /pet attack/qattack # where # is the spawnid of the mob you want the pet to attack");
		cmdPet(pChar, szLine);
		return;
	}
	else {
		CHAR szID[MAX_STRING] = { 0 };
		CHAR szCmd[MAX_STRING] = { 0 };
		GetArg(szCmd, szLine, 1);
		ePetCommandType cmdtype = PCT_DoNothing;
		if (!_stricmp(szCmd, "attack")) {
			cmdtype = PCT_Attack;
		}
		else if (!_stricmp(szCmd, "qattack")) {
			cmdtype = PCT_QueueAttack;
		}
		if (cmdtype) {
			GetArg(szID, szLine, 2);
			if (IsNumber(szID)) {
				if (PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(atoi(szID))) {
					return pEverQuest->IssuePetCommand(cmdtype, pSpawn->SpawnID, false);
				}
			}
		}
		cmdPet(pChar, szLine);
	}
}
// ***************************************************************************
// Function:    MercSwitchCmd
// Description: '/mercswitch' command
//              Adds the ability to do /mercswitch Healer,Damage Caster,Melee Damage or Tank
// Usage:       /mercswitch healer will switch to a Healer merc
// ***************************************************************************
VOID MercSwitchCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0]) {
		WriteChatColor("Usage: /mercswitch <Healer,Damage Caster,Melee Damage or Tank>");
		cmdMercSwitch(pChar, szLine);
		return;
	}
	else {
		std::map<DWORD, MercDesc>descmap;
		GetAllMercDesc(descmap);
		for (std::map<DWORD, MercDesc>::iterator n = descmap.begin(); n != descmap.end(); n++)
		{
			if (!_stricmp(n->second.Type.c_str(), szLine)) {
				CHAR szTemp[256] = { 0 };
				sprintf_s(szTemp, "%d", n->first + 1);
				cmdMercSwitch(pChar, szTemp);
				return;
			}
		}
		cmdMercSwitch(pChar, szLine);
	}
}
// ***************************************************************************
// Function:    AdvLootCmd
// Description: '/advloot' command
//              Extend the /advloot command to accept some parameters:
// Usage: /advloot personal #(listid)| loot,leave,an,ag,never
// Or:    /advloot shared #(listid) item,status,action,manage,autoroll,nd,gd,no,an,ag,nv,name
// Or:    /advloot shared set "item from the shared set to all combo box, can be player name any of the other items that exist in that box..."
// ***************************************************************************
#ifdef EMU
VOID AdvLootCmd(PSPAWNINFO pChar, PCHAR szLine)
{
}
#else
VOID AdvLootCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (GetGameState() != GAMESTATE_INGAME)
		return;
	CHAR szAction[MAX_STRING] = { 0 };
	GetArg(szAction, szLine, 3);
	if (!szLine[0] || !szAction[0]) {
		WriteChatColor("Usage: /advloot personal #(listid) item,loot,leave,an,ag,never,name");
		WriteChatColor("Or:    /advloot shared <#(listid) or \"item name\"> item,status,action,manage,autoroll,nd,gd,no,an,ag,nv,name");
		WriteChatColor("Or:    you can \"Give To:\" /advloot shared <#(listid) or \"Item Name\"> giveto <name> <qty>");
		WriteChatColor("Or:    you can \"Leave on Corpse\" /advloot shared <#(listid) or \"Item Name\"> leave");
		WriteChatColor("Or:    /advloot shared set \"name from the shared set to all combo box, can be player name or any of the other names that exist in that box...\"");
		cmdAdvLoot(pChar, szLine);
		return;
	}
	else {
		if (PEQADVLOOTWND pAdvLoot = (PEQADVLOOTWND)pAdvancedLootWnd) {
			CListWnd *pPersonalList = (CListWnd *)pAdvancedLootWnd->GetChildItem("ADLW_PLLList");
			CListWnd *pSharedList = 0;
			if (pAdvLoot->pCLootList) {
				pSharedList = (CListWnd *)pAdvLoot->pCLootList->SharedLootList;
			}
			//ok at this point, we dont know if we have a user that checks if loot is in progress in his macro, so we do that for him here
			if (LootInProgress(pAdvLoot, pPersonalList, pSharedList)) {
				MacroError("Woah! hold your horses there little filly... You better add a !${AdvLoot.LootInProgress} check in your macro to make sure loot is not in progress.");
				return;
			}
			CHAR szID[MAX_STRING] = { 0 };
			CHAR szCmd[MAX_STRING] = { 0 };
			GetArg(szCmd, szLine, 1);
			int cmdtype = 0;
			if (!_stricmp(szCmd, "personal")) {
				cmdtype = 2;
			}
			else if (!_stricmp(szCmd, "shared")) {
				cmdtype = 3;
			}
			PLOOTITEM pitem = 0;
			if (cmdtype == 2) {
				GetArg(szID, szLine, 2);
				LONG index = -1;
				if (pPersonalList) {
					if (IsNumber(szID)) {
						index = atoi(szID);
						index--;
					}
					else {//if its not a number its a itemname
						  //need to roll through the list to get the index
						if (pAdvLoot->pPLootList) {
							for (LONG k = 0; k < pPersonalList->ItemsArray.Count; k++) {
								LONG listindex = pPersonalList->GetItemData(k);
								if (listindex != -1) {
									DWORD multiplier = sizeof(LOOTITEM) * listindex;
									if (PLOOTITEM pItem = (PLOOTITEM)(((DWORD)pAdvLoot->pPLootList->pLootItem) + multiplier)) {
										if (!_stricmp(pItem->Name, szID)) {
											index = k;
											break;
										}
									}
								}
							}

						}
					}
					LONG listindex = pPersonalList->GetItemData(index);
					if (pAdvLoot && pAdvLoot->pPLootList && listindex != -1) {
						DWORD addr = (DWORD)pAdvLoot->pPLootList->pLootItem;
						pitem = (PLOOTITEM)(addr + (sizeof(LOOTITEM)*listindex));
						if (!_stricmp(szAction, "loot")) {
							if (CXWnd *pwnd = GetAdvLootPersonalListItem(listindex, 2)) {//loot
								SendWndClick2(pwnd, "leftmouseup");
							}
						}
						else if (!_stricmp(szAction, "leave")) {
							if (CXWnd *pwnd = GetAdvLootPersonalListItem(listindex, 3)) {//leave
								SendWndClick2(pwnd, "leftmouseup");
							}
						}
						else if (!_stricmp(szAction, "name")) {
							if (CXWnd *pwnd = GetAdvLootPersonalListItem(listindex, 0)) {//name
								SendWndClick2(pwnd, "leftmouseup");
							}
						}
						else if (!_stricmp(szAction, "item")) {
							if (CXWnd *pwnd = GetAdvLootPersonalListItem(listindex, 1)) {//item
								SendWndClick2(pwnd, "leftmouseup");
							}
						}
						else if (!_stricmp(szAction, "never")) {
							if (CXWnd *pwnd = GetAdvLootPersonalListItem(listindex, 4)) {//never
								SendWndClick2(pwnd, "leftmouseup");
							}
						}
						else if (!_stricmp(szAction, "an")) {
							if (CXWnd *pwnd = GetAdvLootPersonalListItem(listindex, 5)) {//an
								SendWndClick2(pwnd, "leftmouseup");
							}
						}
						else if (!_stricmp(szAction, "ag")) {
							if (CXWnd *pwnd = GetAdvLootPersonalListItem(listindex, 6)) {//ag
								SendWndClick2(pwnd, "leftmouseup");
							}
						}
					}
				}
				return;
			}
			else if (cmdtype == 3) {
				GetArg(szID, szLine, 2);
				if (!_stricmp(szID, "set")) {
					CHAR szEntity[MAX_STRING] = { 0 };
					GetArg(szEntity, szLine, 3);
					CXStr Str;
					CHAR szOut[MAX_STRING] = { 0 };
					if (CComboWnd *pCombo = (CComboWnd *)pAdvancedLootWnd->GetChildItem("ADLW_CLLSetCmbo")) {
						if (CListWnd*pListWnd = pCombo->pListWnd) {
							DWORD itemcnt = pCombo->GetItemCount();
							for (DWORD i = 0; i < itemcnt; i++) {
								pListWnd->GetItemText(&Str, i, 0);
								GetCXStr(Str.Ptr, szOut, MAX_STRING);
								if (szOut[0] != '\0') {
									if (!_stricmp(szEntity, szOut)) {
										CXRect comborect = ((CXWnd*)pCombo)->GetScreenRect();
										CXPoint combopt = comborect.CenterPoint();
										pCombo->SetChoice(i);
										((CXWnd*)pCombo)->HandleLButtonDown(&combopt, 0);
										int index = pListWnd->GetCurSel();
										CXRect listrect = pListWnd->GetItemRect(index, 0);
										CXPoint listpt = listrect.CenterPoint();
										((CXWnd*)pListWnd)->HandleLButtonDown(&listpt, 0);
										((CXWnd*)pListWnd)->HandleLButtonUp(&listpt, 0);
										gMouseEventTime = GetFastTime();
										if (CXWnd *pButton = (CXWnd *)pAdvancedLootWnd->GetChildItem("ADLW_CLLSetBtn")) {
											SendWndClick2(pButton, "leftmouseup");
										}
										break;
									}
								}
							}
						}
					}
					return;
				}
				LONG index = -1;
				if (IsNumber(szID)) {
					index = atoi(szID);
					index--;
				}
				else {//if its not a number its a itemname
					  //need to roll through the list to get the index
					if (pSharedList) {
						for (LONG k = 0; k < pSharedList->ItemsArray.Count; k++) {
							LONG listindex = pSharedList->GetItemData(k);
							if (listindex != -1) {
								DWORD multiplier = sizeof(LOOTITEM) * listindex;
								if (PLOOTITEM pItem = (PLOOTITEM)(((DWORD)pAdvLoot->pCLootList->pLootItem) + multiplier)) {
									if (!_stricmp(pItem->Name, szID)) {
										index = k;
										break;
									}
								}
							}
						}
					}
				}
				if (index != -1) {
					if (pSharedList) {
						LONG listindex = pSharedList->GetItemData(index);
						if (listindex != -1) {
							DWORD multiplier = sizeof(LOOTITEM) * listindex;
							if (pitem = (PLOOTITEM)(((DWORD)pAdvLoot->pCLootList->pLootItem) + multiplier)) {
								if (!_stricmp(szAction, "leave")) {
									if (PCHARINFO pchar = GetCharInfo()) {
										if (GetGameState() == GAMESTATE_INGAME && pitem && pitem->LootDetails) {
											pAdvancedLootWnd->DoSharedAdvLootAction(pitem, &CXStr(pchar->Name), 1, pitem->LootDetails->StackCount);
										}
									}
								}
								else if (!_stricmp(szAction, "giveto")) {
									CHAR szEntity[MAX_STRING] = { 0 };
									GetArg(szEntity, szLine, 4);
									CHAR szQty[MAX_STRING] = { 0 };
									GetArg(szQty, szLine, 5);
									if (szEntity[0] != '\0') {
										if (PCHARINFO pCI = GetCharInfo()) {
											if (pCI->pGroupInfo) {
												CHAR szOut[MAX_STRING] = { 0 };
												for (int i = 0; i < 6; i++) {
													if (pCI->pGroupInfo->pMember[i] && pCI->pGroupInfo->pMember[i]->Mercenary == 0 && pCI->pGroupInfo->pMember[i]->pName) {
														GetCXStr(pCI->pGroupInfo->pMember[i]->pName, szOut, MAX_STRING);
														if (!_stricmp(szOut, szEntity)) {
															int qty = atoi(szQty);
															if (pitem && pitem->LootDetails) {
																if (qty == 0 || qty > pitem->LootDetails->StackCount) {
																	qty = pitem->LootDetails->StackCount;
																	if (qty == 0) {
																		qty = 1;
																	}
																}
																pAdvancedLootWnd->DoSharedAdvLootAction(pitem, &CXStr(szOut), 0, qty);
																return;
															}
														}
													}
												}
											}
										}
									}
								}
								else if (!_stricmp(szAction, "name")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 0)) {//name
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
								else if (!_stricmp(szAction, "item")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 1)) {//item
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
								else if (!_stricmp(szAction, "status")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 2)) {//status
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
								else if (!_stricmp(szAction, "action")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 3)) {//action
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
								else if (!_stricmp(szAction, "manage")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 4)) {//manage
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
								else if (!_stricmp(szAction, "an")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 5)) {//an
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
								else if (!_stricmp(szAction, "ag")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 6)) {//ag
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
								else if (!_stricmp(szAction, "autoroll")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 7)) {//autoroll
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
								else if (!_stricmp(szAction, "nv")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 8)) {//nv
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
								else if (!_stricmp(szAction, "nd")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 9)) {//nd
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
								else if (!_stricmp(szAction, "gd")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 10)) {//gd
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
								else if (!_stricmp(szAction, "no")) {
									if (CXWnd *pwnd = GetAdvLootSharedListItem(listindex, 11)) {//no
										SendWndClick2(pwnd, "leftmouseup");
									}
								}
							}
						}
					}
				}
				return;
			}
			cmdAdvLoot(pChar, szLine);
		}
	}
}
#endif
DWORD __stdcall openpickzonewnd(PVOID pData)
{
	lockit lk(ghLockPickZone, "openpickzonewnd");
	int nInst = (int)pData;
	CHAR szInst[32] = { 0 };
	_itoa_s(nInst, szInst, 10);
	if (PCHARINFO pCharInfo = GetCharInfo()) {
		cmdPickZone(pCharInfo->pSpawn, NULL);
		Sleep(2000);//i need to make this hardcoded wait dynamic but im in a hurry ill do it later -eqmule
		if (CXWnd *krwnd = FindMQ2Window("MIZoneSelectWnd")) {
			if (krwnd->dShow) {
				if (CListWnd *clist = (CListWnd*)krwnd->GetChildItem("MIZ_ZoneList")) {
					if (DWORD numitems = ((CSidlScreenWnd*)clist)->Items) {
						if (CButtonWnd *cbutt = (CButtonWnd*)krwnd->GetChildItem("MIZ_SelectButton")) {
							CHAR szOut[MAX_STRING] = { 0 };
							CXStr Str;
							std::string s;
							bool itsmain = false;
							bool clickit = false;
							for (DWORD i = 0; i<numitems; i++) {
								clist->GetItemText(&Str, i, 0);
								GetCXStr(Str.Ptr, szOut, MAX_STRING);
								if (szOut[0] != '\0') {
									s = szOut;
									if (std::string::npos == s.find_first_of("123456789")) {
										itsmain = true;
									}
									if (itsmain && nInst == 0) {
										clickit = true;
									}
									else if (nInst >= 1) {
										if (std::string::npos != s.find(szInst)) {
											clickit = true;
										}
									}
									if (clickit) {
										SendListSelect("MIZoneSelectWnd", "MIZ_ZoneList", i);
										Sleep(500);
										SendWndClick2((CXWnd*)cbutt, "leftmouseup");
										WriteChatf("%s selected.", szOut);
										return 0;
									}
								}
							}
							WriteChatf("%s instance %d NOT found in list", GetFullZone(pCharInfo->zoneId), nInst);
						}
					}
				}
			}
		}
	}
	return 0;
}
// ***************************************************************************
// Function:    PickZoneCmd
// Description: '/pickzone' command
//              Adds the ability to do /pickzone #
// Usage:       /pickzone 2 will switch zone to number 2 pickzone 0 will pick main instance
// ***************************************************************************
VOID PickZoneCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine[0]) {
		WriteChatColor("Usage: /pick # where # is the instance number you want to pick");
		cmdPickZone(pChar, szLine);
		return;
	}
	else {
		DWORD index = atoi(szLine);
		DWORD nThreadID = 0;
		CreateThread(NULL, 0, openpickzonewnd, (PVOID)index, 0, &nThreadID);
	}
}
// ***************************************************************************
// Function:    AssistCmd
// Description: '/assist' command
// Purpose:     Adds the ability to forward /assist so we can set the gbAssist flag
// Author:      EqMule
// ***************************************************************************
VOID AssistCmd(PSPAWNINFO pChar, PCHAR szLine)
{
	gbAssistComplete = 0;
	cmdAssist(pChar, szLine);
}

// ***************************************************************************
// Function:    SetProcessPriority
// Description: '/setprio' command
// Purpose:     Adds the ability to set the process priority
// Example:		/setprio <1-6> Where 1 is Low 2 is below Normal 3 is Normal 4 is Above Normal 5 is High and 6 is RealTime
// Author:      EqMule
// ***************************************************************************
VOID SetProcessPriority(PSPAWNINFO pChar, char *szLine)
{
	if (szLine && szLine[0] == '\0') {
		WriteChatf("Usage: /setprio <1-6> Where 1 is Low 2 is Below Normal 3 is Normal 4 is Above Normal 5 is High and 6 is RealTime");
		return;
	}
	DWORD prio = NORMAL_PRIORITY_CLASS;
	CHAR szPrio[2048] = { 0 };
	if(int newprio = atoi(szLine)) {
		switch (newprio)
		{
		case 1:
			prio = IDLE_PRIORITY_CLASS;
			strcpy_s(szPrio,"Process Priority Set to \agLOW\ax");
			break;
		case 2:
			prio = BELOW_NORMAL_PRIORITY_CLASS;
			strcpy_s(szPrio,"Process Priority Set to \agBELOW NORMAL\ax");
			break;
		case 3:
			prio = NORMAL_PRIORITY_CLASS;
			strcpy_s(szPrio,"Process Priority Set to \agNORMAL\ax");
			break;
		case 4:
			prio = ABOVE_NORMAL_PRIORITY_CLASS;
			strcpy_s(szPrio,"Process Priority Set to \agABOVE NORMAL\ax");
			break;
		case 5:
			prio = HIGH_PRIORITY_CLASS;
			strcpy_s(szPrio,"Process Priority Set to \agHIGH\ax");
			break;
		case 6:
			prio = REALTIME_PRIORITY_CLASS;
			strcpy_s(szPrio,"Process Priority Set to \agREALTIME\ax");
			break;
		default:
			prio = NORMAL_PRIORITY_CLASS;
			break;
		}
	}
	if(HANDLE heqg = OpenProcess(PROCESS_SET_INFORMATION, FALSE, GetCurrentProcessId())) {
		SetPriorityClass(heqg, prio);
		CloseHandle(heqg);
		WriteChatf("%s", szPrio);
	} else {
		WriteChatf("Process Priority was NOT changed, Could not open Process");
	}
}
// ***************************************************************************
// Function:    ScreenModeCmd
// Description: '/screenmode' command
// Purpose:     Adds the ability to set the screenmode
// Example:		/screenmode <#> Where 2 is Normal and 3 is No Windows
// Author:      EqMule
// ***************************************************************************
VOID ScreenModeCmd(PSPAWNINFO pChar, char *szLine)
{
	if (szLine && szLine[0] == '\0') {
		WriteChatf("Usage: /screenmode <#>");
		return;
	}
	int newprio = atoi(szLine);
	ScreenMode = newprio;
	WriteChatf("Screen Mode Set to \ag%d\ax", newprio);
}
#endif