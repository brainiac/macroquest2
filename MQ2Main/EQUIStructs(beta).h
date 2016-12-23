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
#pragma pack(push)
#pragma pack(8)
namespace EQUIStructs
{


enum UIType
{
   UI_Unknown=-1,
   UI_Class=0,
   UI_RGB=1,
   UI_Point=2,
   UI_Size=3,
   UI_TextureInfo=4,
   UI_Frame=5,
   UI_Ui2DAnimation=6,
   UI_ButtonDrawTemplate=7,
   UI_GaugeDrawTemplate=8,
   UI_SpellGemDrawTemplate=9,
   UI_FrameTemplate=10,
   UI_ScrollbarDrawTemplate=11,
   UI_WindowDrawTemplate=12,
   UI_SliderDrawTemplate=13,
   UI_ScreenPiece=14,
   UI_StaticScreenPiece=15,
   UI_StaticAnimation=16,
   UI_StaticTintedBlendAnimation=17,
   UI_StaticText=18,
   UI_StaticFrame=19,
   UI_StaticHeader=20,
   UI_LayoutStrategy=21,
   UI_LayoutVertical=22,
   UI_LayoutHorizontal=23,
   UI_Control=24,
   UI_TemplateAssoc=25,
   UI_TemplateScreen=26,
   UI_ListboxColumn=27,
   UI_Listbox=28,
   UI_Button=29,
   UI_Gauge=30,
   UI_SpellGem=31,
   UI_HtmlComponent=32,
   UI_InvSlot=33,
   UI_EditBox=34,
   UI_Slider=35,
   UI_Label=36,
   UI_STMLBox=37,
   UI_TreeView=38,
   UI_Combobox=39,
   UI_Page=40,
   UI_TabBox=41,
   UI_LayoutBox=42,
   UI_HorizontalLayoutBox=43,
   UI_VerticalLayoutBox=44,
   UI_FinderBox=45,
   UI_TileLayoutBox=46,
   UI_NamedTemplatePiece=47,
   UI_TemplateContainer=48,
   UI_Screen=49,
   UI_SuiteDefaults=50,
   UI_Screens=51,
   UI_TopLevelWindowList=52,
   UI_HotButton=53,
};

// ***************************************************************************
// Structures
// ***************************************************************************

// 10-24-2003 Lax - Massive overhaul.
// See http://macroquest.sourceforge.net/phpBB2/viewtopic.php?t=3569 for
// standards on structures to make things easier on everyone!
// -- ONLY applies to EQ structs, not MQ structs like alerts, etc.

typedef struct _EQCURRENTSELECTION {
/*0x00*/   DWORD   Unknown;
/*0x04*/   DWORD   SelectedWnd;     //address to selection in tree
/*0x08*/   DWORD   Unknownlodncrap;
/*0x0c*/   DWORD   NumberOfSlots;   //merchantslots == 79
/*0x10*/   DWORD   TextureAnim;
/*0x14*/
} EQCURRENTSELECTION, *PEQCURRENTSELECTION;

//CXWnd see B1058C in Nov  7 2016 eqgame.exe (Test) -eqmule
#define VFTABLE \
/*0x000*/ LPVOID  IsValid; \
/*0x004*/ LPVOID  vector_deleting_destructor; \
/*0x008*/ LPVOID  DrawNC; \
/*0x00c*/ LPVOID  PostDraw; \
/*0x010*/ LPVOID  PostDraw2; \
/*0x014*/ LPVOID  DrawCursor; \
/*0x018*/ LPVOID  CXWnd__OnResize; \
/*0x01c*/ LPVOID  PostDraw3; \
/*0x020*/ LPVOID  DrawBackground; \
/*0x024*/ LPVOID  DrawTooltip; \
/*0x028*/ LPVOID  DrawTooltipAtPoint; \
/*0x02c*/ LPVOID  GetMinimizedRect; \
/*0x030*/ LPVOID  DrawTitleBar; \
/*0x034*/ LPVOID  GetCursorToDisplay; \
/*0x038*/ LPVOID  HandleLButtonDown; \
/*0x03c*/ LPVOID  HandleLButtonUp; \
/*0x040*/ LPVOID  HandleLButtonHeld; \
/*0x044*/ LPVOID  HandleLButtonUpAfterHeld; \
/*0x048*/ LPVOID  HandleRButtonDown; \
/*0x04c*/ LPVOID  HandleRButtonUp; \
/*0x050*/ LPVOID  HandleRButtonHeld; \
/*0x054*/ LPVOID  HandleRButtonUpAfterHeld; \
/*0x058*/ LPVOID  HandleWheelButtonDown; \
/*0x05c*/ LPVOID  HandleWheelButtonUp; \
/*0x060*/ LPVOID  HandleMouseMove; \
/*0x064*/ LPVOID  HandleWheelMove; \
/*0x068*/ LPVOID  GetDragDropCursor; \
/*0x06c*/ LPVOID  PostDraw4; \
/*0x070*/ LPVOID  OnDragDrop; \
/*0x074*/ LPVOID  GetClickStickCursor; \
/*0x078*/ LPVOID  QueryDropOK; \
/*0x07c*/ LPVOID  OnClickStick; \
/*0x080*/ LPVOID  GetClickStickCursor2; \
/*0x084*/ LPVOID  QueryClickStickDropOK; \
/*0x088*/ LPVOID  WndNotification; \
/*0x08c*/ LPVOID  Unknown8c; \
/*0x090*/ LPVOID  ShowWindow; /* Activate */ \
/*0x094*/ LPVOID  Deactivate; \
/*0x098*/ LPVOID  PostDraw5; \
/*0x09c*/ LPVOID  OnResize; \
/*0x0a0*/ LPVOID  OnResize2; \
/*0x0a4*/ LPVOID  Unknown0xa4; \
/*0x0a8*/ LPVOID  Unknown0xa8; \
/*0x0ac*/ LPVOID  OnMinimizeBox; \
/*0x0b0*/ LPVOID  Unknown0xb0; \
/*0x0b4*/ LPVOID  OnTileBox; \
/*0x0b8*/ LPVOID  PostDraw6; \
/*0x0bc*/ LPVOID  OnMove2; \
/*0x0c0*/ LPVOID  OnMove3; \
/*0x0c4*/ LPVOID  OnProcessFrame; /*Dofade*/ \
/*0x0c8*/ LPVOID  OnVScroll; \
/*0x0cc*/ LPVOID  OnHScroll; \
/*0x0d0*/ LPVOID  PostDraw7; \
/*0x0d4*/ LPVOID  OnMove4; \
/*0x0d8*/ LPVOID  Show; \
/*0x0dc*/ LPVOID  AboutToShow; \
/*0x0e0*/ LPVOID  AboutToHide; \
/*0x0e4*/ LPVOID  RequestDockInfo; \
/*0x0e8*/ LPVOID  GetTooltip; \
/*0x0ec*/ LPVOID  Unknown0x0ec; /*this is new and added on oct 11 2016 its just empty*/ \
/*0x0f0*/ LPVOID  HitTest; \
/*0x0f4*/ LPVOID  GetHitTestRect; \
/*0x0f8*/ LPVOID  GetInnerRect; \
/*0x0fc*/ LPVOID  GetClientRect; \
/*0x100*/ LPVOID  GetClientClipRect; \
/*0x104*/ LPVOID  GetMinSize; \
/*0x108*/ LPVOID  GetMaxSize; \
/*0x10c*/ LPVOID  GetUntileSize; \
/*0x110*/ LPVOID  IsPointTransparent; \
/*0x114*/ LPVOID  Unknown10c; \
/*0x118*/ LPVOID  Unknown110; \
/*0x11c*/ LPVOID  Unknown114; \
/*0x120*/ LPVOID  SetDrawTemplate; \
/*0x124*/ LPVOID  Move_Rect; /*CXWnd__Move1_x*/ \
/*0x128*/ LPVOID  Move_Point; /*CXWnd__Move_x*/ \
/*0x12c*/ LPVOID  SetWindowTextA; \
/*0x130*/ LPVOID  GetChildWndAt; /*CXWnd__GetChildWndAt_x*/ \
/*0x134*/ LPVOID  GetSidlPiece; /*CSidlScreenWnd__GetSidlPiece_x*/ \
/*0x138*/ LPVOID  CSidlScreenWnd__OnPreZone1a; \
/*0x13c*/ LPVOID  SetVScrollPos; \
/*0x140*/ LPVOID  SetHScrollPos; \
/*0x144*/ LPVOID  AutoSetVScrollPos; \
/*0x148*/ LPVOID  AutoSetHScrollPos; \
/*0x14c*/ LPVOID  SetAttributesFromSidl; \
/*0x150*/ LPVOID  nullsub_1a; \
/*0x154*/ LPVOID  Unknown0x14c; \
/*0x158*/ LPVOID  nullsub_1b; \
/*0x15c*/ LPVOID  GetMinClientSize; \
/*0x160*/ LPVOID  GetMaxClientSize; \
/*0x164*/ LPVOID  CSidlScreenWnd__OnPreZone1b; \
/*0x168*/ LPVOID  UpdateLayout;
/*0x16c*/

//CSidlScreenWnd see B1116C in Nov  7 2016 eqgame.exe (Test) -eqmule
typedef struct _CSIDLWNDVFTABLE {
	VFTABLE
	/*0x16c*/ LPVOID  CSidlScreenWnd__OnPreZone1c;
	/*0x170*/ LPVOID  CSidlScreenWnd__OnPreZone1d;
	/*0x170*/ LPVOID  LoadIniInfo;
	/*0x174*/ LPVOID  StoreIniInfo;
	/*0x178*/ LPVOID  EQObject__AsObject;
#if !defined(EMU)
	/*0x17c*/ LPVOID  CTreeViewPropertySet__IsPropertySet;
#endif
	/*0x180*/
} CSIDLWNDVFTABLE, *PCSIDLWNDVFTABLE;

typedef struct _CXWNDVFTABLE {
VFTABLE
} CXWNDVFTABLE, *PCXWNDVFTABLE;

// in CChatWindow__CChatWindow
#define EQ_CHAT_HISTORY_OFFSET 0x298
// in CChatWindow__SetChatFont
#define EQ_CHAT_FONT_OFFSET    0x11c


class CDynamicArrayBase
{
public:
	/*0x00*/	int Count;
};
//this class has some members like Reset and so on
//but we dont need them right now...
//todo: spend some time on fully implementing this
template <typename ArrayType> class ArrayClass2 : public CDynamicArrayBase
{
public:
/*0x04*/	int Size;
/*0x08*/	int Mask;
/*0x0c*/	int Shift;
/*0x10*/	ArrayType** pNext;
/*0x14*/	int memAlloc;
/*0x18*/	bool bValid;
};
#define CXW_NO_VTABLE_BEGIN \
/*0x0004*/ struct _CSIDLWND**	WindowPtrs; \
/*0x0008*/ struct _CSIDLWND*	pPrevSiblingWnd; \
/*0x000c*/ struct _CSIDLWND*	pNextSiblingWnd; \
/*0x0010*/ void*	pUnknown0x10; \
/*0x0014*/ struct _CSIDLWND*	pFirstChildWnd; \
/*0x0018*/ BYTE		Unknown0x0018[0x4];

//Actual Size: 1ec (see 8C4FDD in eqgame dated Aug 21 2014) - eqmule
//Actual Size: 1c8 (see 8C614D in eqgame dated Sep 18 2014) - eqmule
// actual size 0x1C4 in Mar 23 2015 Live (see 0x6885B8) - eqmule
// actual size 0x1CC in Sep 22 2015 Live (see 0x6920D8) - eqmule
// actual size 0x1D8 in Nov 16 2015 Live (see 0x6988A8) - eqmule
// actual size 0x1D0 in Nov 19 2015 Live (see 0x697E32) - eqmule
// actual size 0x1DC in May 17 2016 Live (see 0x691958) - eqmule
// actual size 0x1D4 in Jun 10 2016 Live (see 0x692258) - eqmule
// actual size 0x1E4 in Jun 21 2016 Live (see 0x691F88) - eqmule
// actual size 0x1E0 in Oct 21 2016 Beta (see 0x6972B2) - eqmule
// actual size 0x1F8 in Oct 25 2016 Beta (see 0x6967D8) - eqmule
// actual size 0x1F0 in Nov 11 2016 Beta (see 0x697B98) - eqmule
// actual size 0x1F0 in Nov 11 2016 Beta (see 0x697B98) - eqmule
// actual size 0x1E8 in Nov 14 2016 Beta (see 0x696A98) - eqmule
#define CXW_NO_VTABLE \
CXW_NO_VTABLE_BEGIN \
/*0x001C*/ struct _CSIDLWND* pWndFocusOther; \
/*0x0020*/ bool		CloseOnESC;     /* found in CSidlScreenWnd__StoreIniInfo_x, close when ESC is pressed */ \
/*0x0024*/ RECT		Location; \
/*0x0034*/ tagSIZE	MinClientSize; \
/*0x003C*/ bool		bBottomAnchoredToTop; \
/*0x003D*/ bool		bCaptureTitle; \
/*0x003E*/ bool		Locked; /* found in CSidlScreenWnd__LoadIniInfo_x */ \
/*0x0040*/ DWORD	BGType;         /* found in CSidlScreenWnd__StoreIniInfo_x */ \
/*0x0044*/ void* pTipTextObject; \
/*0x0048*/ DWORD  BackgroundDrawType; \
/*0x004C*/ RECT		OldLocation; \
/*0x005C*/ bool		bEscapableLocked; \
/*0x005D*/ BYTE		FadeToAlpha; /* found in CSidlScreenWnd__StoreIniInfo_x */ \
/*0x005E*/ bool		Unlockable;     /* found in CSidlScreenWnd__LoadIniInfo_x related to Locked */ \
/*0x0060*/ int		ZLayer; /* found in CXWndManager__DrawWindows_x */ \
/*0x0064*/ UINT		TransitionStartTick; \
/*0x0068*/ struct _CXSTR*	Tooltip; /* found in CSidlManager__CreateLabel_x */ \
/*0x006C*/ COLORREF	CRNormal; /* found in OnProcessFrame */ \
/*0x0070*/ int		HScrollPos; \
/*0x0074*/ tagSIZE	MaxClientSize; \
/*0x007C*/ RECT		IconRect; \
/*0x008C*/ UINT		TransitionDuration; \
/*0x0090*/ COLORREF	BGColor; /* DO NOT CHNAGE THIS TO AN ARGBCOLOR, it will break the padding since its a union that has bytes in it. */ \
/*0x0094*/ int		TopOffset; \
/*0x0098*/ DWORD	FadeDelay; \
/*0x009C*/ BYTE     bResizableMask; \
/*0x00A0*/ void		*pLayoutStrategy; \
/*0x00A4*/ RECT		ClipRectScreen; \
/*0x00B4*/ LONG		BlinkStartTimer; \
/*0x00B8*/ bool bClickThrough; /* if true you can click through the window, well it doesnt work for our chatwindow (yet) so more work is needed to figure out why */ \
/*0x00B9*/ bool		bNeedsSaving; /* will be true if you move or resize the window */ \
/*0x00BA*/ bool		bLeftAnchoredToLeft; \
/*0x00BB*/ bool		Clickable;      /* found in CChatWindow__CChatWindow_x and the button handlers */ \
/*0x00BC*/ void	*IconTextureAnim; \
/*0x00C0*/ void	    *pFont; \
/*0x00C4*/ bool		bFullyScreenClipped; \
/*0x00C8*/ UINT		FadeDuration; \
/*0x00CC*/ bool		bClientRectChanged; \
/*0x00CD*/ bool		Minimized; \
/*0x00D0*/ struct _CXSTR*	XMLToolTip; /* found in CSidlManager__CreateLabel_x */ \
/*0x00D4*/ struct _CXSTR*	DataStr; \
/*0x00D8*/ BYTE		FadeAlpha; \
/*0x00D9*/ bool		Faded; \
/*0x00DA*/ BYTE		TargetAlpha; \
/*0x00DC*/ int		BlinkDuration; \
/*0x00E0*/ void*	pTextObject; \
/*0x00E4*/ BYTE		Alpha; \
/*0x00E5*/ bool		bHCenterTooltip; \
/*0x00E8*/ int		VScrollMax; \
/*0x00EC*/ bool		bMarkedForDelete; \
/*0x00F0*/ DWORD	LastTimeMouseOver; \
/*0x00F4*/ bool		bMaximized; \
/*0x00F5*/ bool		bUseInLayoutVertical; \
/*0x00F6*/ bool		bUseInLayoutHorizontal; \
/*0x00F8*/ struct _CSIDLWND*		pParentWindow; /* CXWnd__IsDescendantOf_x has this one, If this is NULL, coordinates are absolute... */ \
/*0x00FC*/ bool		bClientClipRectChanged; \
/*0x0100*/ UINT		BlinkFadeFreq; \
/*0x0104*/ bool		ValidCXWnd; /* IsValid has this one */ \
/*0x0108*/ DWORD	WindowStyle;    /* bit 1 - vertical scroll, bit 2 - horizontal scroll, bit 4 - title bar?, bit 8 - border */ \
/*0x010C*/ int		LeftOffset; \
/*0x0110*/ RECT		TransitionRect; \
/*0x0120*/ int		RightOffset; \
/*0x0124*/ void *pController; \
/*0x0128*/ DWORD	XMLIndex; \
/*0x012C*/ int		ParentAndContextMenuArrayIndex; \
/*0x0130*/ void   *TitlePiece; \
/*0x0134*/ bool		bRightAnchoredToLeft; \
/*0x0138*/ int		BottomOffset; \
/*0x013C*/ ArrayClass2<UINT> RuntimeTypes; \
/*0x0158*/ bool		bBringToTopWhenClicked; \
/*0x015C*/ struct _CXSTR*	WindowText; /* CXWnd__GetWindowTextA_x has this one */ \
/*0x0160*/ bool		bTopAnchoredToTop; \
/*0x0164*/ int		BlinkState; \
/*0x0168*/ bool		bClipToParent; \
/*0x0169*/ bool		bTiled; \
/*0x016A*/ bool		dShow; \
/*0x016C*/ int		managerArrayIndex; \
/*0x0170*/ bool		MouseOver; /* found in CXWnd__SetMouseOver_x */ \
/*0x0174*/ RECT		ClientRect; \
/*0x0184*/ bool		bIsTransitioning; \
/*0x0188*/ UINT		BlinkFadeStartTime; \
/*0x018C*/ RECT		ClipRectClient; \
/*0x019C*/ bool		bMaximizable; \
/*0x01A0*/ __int64		Data; \
/*0x01A8*/ int		DeleteCount; \
/*0x01AC*/ int	Transition; \
/*0x01B0*/ bool		bIsParentOrContextMenuWindow; \
/*0x01B4*/ UINT		LastBlinkFadeRefreshTime; \
/*0x01B8*/ bool		bAction; \
/*0x01BC*/ int		VScrollPos; \
/*0x01C0*/ bool		bScreenClipRectChanged; \
/*0x01C4*/ COLORREF DisabledBackground; \
/*0x01C8*/ int		HScrollMax; \
/*0x01CC*/ bool		Enabled; \
/*0x01CD*/ BYTE		StartAlpha; \
/*0x01D0*/ UINT		BlinkFadeDuration; \
/*0x01D4*/ bool		bActive; \
/*0x01D8*/ void	*DrawTemplate; \
/*0x01DC*/ bool bClickThroughMenuItemStatus; /* on/off */ \
/*0x01DD*/ bool		bKeepOnScreen; \
/*0x01DE*/ bool		Fades; \
/*0x01E0*/ void   *TitlePiece2; \
/*0x01E4*/ bool bShowClickThroughMenuItem;	/*shows/hides the click through option on the window menu*/ \
/*0x01e8*/


#define CXW \
/*0x000*/   struct  _CXWNDVFTABLE   *pvfTable; \
CXW_NO_VTABLE \
/*0x1e8*/

typedef struct _CXWND {
CXW
} CXWND, *PCXWND;

#define GateBind          0
//CSidlScreenWnd__CSidlScreenWnd1_x
//size is 230h in eqgame dated Feb 20 2015 (see 5CB52D(cswsize)) -eqmule
#define SIDL \
/*0x1e8*/ union { \
                        struct _CXSTR* SidlText; /*found in CChatWindow__WndNotification_x*/\
                        LONG Items; \
                }; \
/*0x1ec*/ union { \
                        struct _CXSTR* SidlScreen; \
                        DWORD SlotID; \
                }; \
/*0x1f0*/ LPVOID SidlPiece; /* CScreenPieceTemplate (important) */ \
/*0x1f4*/ union { /*find in CSidlScreenWnd__dCSidlScreenWnd_x*/ \
                struct { \
                        BYTE Checked; \
                        BYTE Highlighted; \
                        BYTE Unused0x1f6[0x2]; \
                }; \
                DWORD MaxChars; \
        }; \
/*0x1f8*/ DWORD TextureAnim; \
/*0x1fc*/ struct  _CXSTR* InputText; /*found in CChatWindow__CChatWindow_x*/\
/*0x200*/ DWORD Selector; /*found in CSidlScreenWnd__CSidlScreenWnd1_x*/\
/*0x204*/ DWORD PushToSelector; /*found in CSidlScreenWnd__LoadIniInfo_x*/\
/*0x208*/ DWORD EnableINIStorage; /*found in CSidlScreenWnd__LoadSidlScreen_x*/\
/*0x20c*/ union { \
                struct _CXSTR* INIStorageName; /*found in CSidlScreenWnd__LoadSidlScreen_x*/\
                struct _EQINVSLOT* pEQInvSlot; \
        }; \
/*0x210*/ BYTE  Unknown0x210[0x10];  \
/*0x220*/ LPVOID ContextMenu; /* CTextureAnimation */ \
/*0x224*/ DWORD Unknown0x224; /* CTextureAnimation */ \
/*0x228*/ DWORD Unknown0x228; /* CTextureAnimation */ \
/*0x22c*/ DWORD Unknown0x22c; \
/*0x230*/

#define CSW \
/*0x000*/ struct _CSIDLWNDVFTABLE* pvfTable; \
CXW_NO_VTABLE \
SIDL \

typedef struct _CSIDLWND {
CSW
} CSIDLWND, *PCSIDLWND;

typedef struct _CCONTEXTMENUVFTABLE {
VFTABLE
/*0x168*/ LPVOID CListWnd__DrawColumnSeparators;
/*0x16c*/ LPVOID CListWnd__DrawSeparator;
/*0x170*/ LPVOID CListWnd__DrawLine;
/*0x174*/ LPVOID CListWnd__DrawHeader;
/*0x178*/ LPVOID CListWnd__DrawItem;
/*0x17c*/ LPVOID CListWnd__DeleteAll;
/*0x180*/ LPVOID CListWnd__Compare;
/*0x184*/ LPVOID CListWnd__Sort;
/*0x188*/ LPVOID CContextMenu__ShowAt;
/*0x18c*/
} CCONTEXTMENUVFTABLE, *PCCONTEXTMENUVFTABLE; 

//see ref to CContextMenu__CContextMenu_x in CBuffWindow__CBuffWindow
// size 0x298 Aug 11 2016 (see 67DDBB) -eqmule
#define CONTEXTTAIL \
/*0x230*/ DWORD ZeroMe; \
/*0x234*/ DWORD SetToMinus1; /*must be set to -1 in constructor or menu wndnotification wont be called*/ \
/*0x238*/ DWORD WillBeZero1; /*set by contructor to 0*/ \
/*0x23c*/ ARGBCOLOR HightLightTextColor; /*set by contructor*/ \
/*0x240*/ ARGBCOLOR HighLightTextBkColor; /*set by contructor*/ \
/*0x244*/ BYTE ZeroMeOut1[0x24]; \
/*0x268*/ DWORD WillBeZero2; /*constructor sets this to 0*/ \
/*0x26c*/ DWORD ZeroMeOut2[0x18]; \
/*0x284*/ DWORD Unknown0x284; /*constructor sets this to 0x10000*/ \
/*0x288*/ DWORD ZeroMeAsWell; \
/*0x28c*/ DWORD Unknown0x28c; /*constructor sets this to 0*/ \
/*0x290*/ BYTE Unknown0x290[0x8]; \
/*0x298*/

#define CONTEXTW \
/*0x000*/   struct  _CCONTEXTMENUVFTABLE   *pvfTable; \
CXW_NO_VTABLE \
SIDL \
CONTEXTTAIL \

typedef struct _CCONTEXTMENU {
CONTEXTW
} CCONTEXTMENU, *PCCONTEXTMENU;

// size 0x298  09-03-2008
typedef struct _CSIDLMGR {
/*0x000*/    BYTE    Unknown0x000[0x144];
/*0x144*/    PVOID   pXMLDataMgr;
/*0x148*/    BYTE    Unknown0x11c[0x150];
/*0x298*/
} CSIDLMGR, *PCSIDLMGR;

//11-6-2003 lax
//Actual size 0x1b0 in Jun 28 2016 Test (see 4AF805) - eqmule
typedef struct _CXWNDMGR {
/*0x000*/ LPVOID CXWNDMGR__vtable;
/*0x004*/ struct _CSIDLWND **pWindowPtrs;
/*0x008*/ DWORD Count;
/*0x00c*/ struct _CSIDLWND **pWindows;
/*0x010*/ BYTE Unknown0x010[0x40];
/*0x050*/ DWORD LastInteractionTimeStamp;//when mouse or keyboard was last clicked
/*0x054*/ DWORD MouseMoveTimeStamp;//when mouse was last moved
/*0x058*/ DWORD KeyHeldTimeStamp;//how long current/last key has been held
/*0x05c*/ BYTE Unknown0x05c[0xc];
/*0x068*/ PCSIDLWND LastWindowSelected;//when you select a window its pointer shows up here
/*0x06c*/ PCSIDLWND CurrDraggedWindow;//when you drag a window its pointer shows up here
/*0x070*/ BYTE Unknown0x070[0x4];
/*0x074*/ PCSIDLWND LastMouseOver;//Current window pointer is over if this is 0 we are above MAIN Window
/*0x078*/ PCSIDLWND Tooltip;//Last Tooltip to show?
/*0x07c*/ BYTE Unknown0x07c[0x1c];
/*0x098*/ DWORD ScreenMouseX;
/*0x09c*/ DWORD ScreenMouseY;
/*0x0a0*/ BYTE Unknown0x0a0;
/*0x0a1*/ bool KeyboardFlags[4];    // 7d-80
/*0x0a5*/ BYTE Unknown0x0a5[0x27];
/*0x0cc*/ DWORD ScreenMouseXCopy;
/*0x0d0*/ DWORD ScreenMouseYCopy;
/*0x0d4*/ CHAR Unknown0x0d4[0x40];
/*0x114*/ DWORD ScreenExtentX;
/*0x118*/ DWORD ScreenExtentY;
/*0x11c*/ DWORD Unknown;
/*0x120*/ DWORD font_list_ptr;
/*0x124*/ BYTE Unknown0x124[0x14];
/*0x138*/ DWORD ScreenMouseLookX;//last position MouseX was at before we mouselooked
/*0x13c*/ DWORD ScreenMouseLookY;//last position MouseY was at before we mouselooked
/*0x140*/ BYTE Unknown0x140[0x70];
/*0x1b0*/ 
} CXWNDMGR, *PCXWNDMGR;


typedef struct _CONTENTSARRAY {
    struct _CONTENTS* Array[1];
} CONTENTSARRAY, *PCONTENTSARRAY;

// 10-27-2003 Lax
// Actual size 0x208 11-5-2003
typedef struct _EQCHATMGR
{
/*0x000*/ struct _EQCHATWINDOW* ChatWnd[0x20];
/*0x080*/ DWORD NumWindows;
/*0x084*/ DWORD ActiveWindow; // CChat::GetActiveChatWindow
/*0x088*/ DWORD LockedWindow; // CChatManager::GetActiveChatWindow
/*0x08c*/ DWORD Unknown0x08c;
/*0x090*/ struct _EQCHATWINDOW* ChannelMap[0x20];   // channel map
/*0x110*/ BYTE  Unknown0x110[0x28];
/*0x138*/ DWORD Unknown0x138;
/*0x13c*/ DWORD Unknown0x13c;
/*0x140*/ DWORD Unknown0x140;
/*0x144*/ LPVOID ChatMenu;
/*0x148*/ DWORD Unknown0x148;
/*0x14c*/ DWORD Unknown0x14c;
/*0x150*/ DWORD Unknown0x150;
/*0x154*/ DWORD Unknown0x154;
/*0x158*/ LPVOID MeleeMenu;
/*0x15c*/ DWORD Unknown0x15c;
/*0x160*/ LPVOID SpellMenu;
/*0x164*/ DWORD Unknown0x164;
/*0x168*/ DWORD Unknown0x168;
/*0x16c*/ LPVOID ChannelMenu;
/*0x170*/ LPVOID ChatMenu2;
/*0x174*/ LPVOID FilterMenu;
/*0x178*/ DWORD Unknown0x178;
/*0x17c*/ DWORD Unknown0x17c;
/*0x180*/ DWORD Unknown0x180;
/*0x184*/ LPVOID HitsModeMenu;
/*0x188*/ LPVOID MeleeMenu2;
/*0x18c*/ LPVOID HitsModeMenu2;
/*0x190*/ DWORD Unknown0x190;
/*0x194*/ LPVOID HitsModeMenu3;
/*0x198*/ DWORD Unknown0x198;
/*0x19c*/ LPVOID HitsModeMenu4;
/*0x1a0*/ DWORD Unknown0x1a0;
/*0x1a4*/ LPVOID HitsModeMenu5;
/*0x1a8*/ DWORD Unknown0x1a8;
/*0x1ac*/ LPVOID HitsModeMenu6;
/*0x1b0*/ DWORD Unknown0x1b0;
/*0x1b4*/ DWORD Unknown0x1b4; // CChatManager__UpdateTellMenus
/*0x1b8*/ DWORD Unknown0x1b8;
/*0x1bc*/ DWORD Unknown0x1bc; // CChatManager__UpdateTellMenus
/*0x1c0*/ DWORD Unknown0x1c0;
/*0x1c4*/ DWORD Unknown0x1c4; // CChatManager__UpdateTellMenus // raid
/*0x1c8*/ DWORD Unknown0x1c8;
/*0x1cc*/ DWORD Unknown0x1cc; // CChatManager__UpdateTellMenus
/*0x1d0*/ DWORD Unknown0x1d0; // CChatManager__UpdateTellMenus
/*0x1d4*/ DWORD Unknown0x1d4; // CChatManager__UpdateTellMenus
/*0x1d8*/ DWORD Unknown0x1d8;
/*0x1dc*/ DWORD Unknown0x1dc;
/*0x1e0*/ DWORD Unknown0x1e0;
/*0x1e4*/ BYTE  Unknown0x1e4[0x1c];
/*0x200*/ DWORD Unknown0x200;
/*0x204*/ DWORD Unknown0x204;
/*0x208*/
} EQCHATMGR, *PEQCHATMGR;

// Actual size 0x26c 02-14-2007
// 10-27-2003 Lax
typedef struct _EQCHATWINDOW
{
/*0x000*/ struct _CSIDLWND Wnd;
/*0x214*/ struct _EQCHATMGR *ChatManager; 
/*0x218*/ struct _CSIDLWND* InputWnd;
/*0x21c*/ struct _CSIDLWND* OutputWnd;
/*0x220*/ DWORD Unknown0x154;
/*0x224*/ DWORD Unknown0x158;       // need to update locations..
/*0x228*/ BYTE Unknown0x148;
/*0x229*/ BYTE Unknown0x149[0x3f];
/*0x268*/ DWORD Unknown0x188;
/*0x26c*/ DWORD Unknown0x18c;
/*0x270*/ struct _CXSTR *CommandHistory[0x28]; // ->0x198
/*0x310*/ DWORD Unknown0x230;       // CChatWindow::HistoryBack/forward .. maybe total or current history lines
/*0x314*/ DWORD Unknown0x234;       // CChatWindow::HistoryBack/forward .. maybe total or current history lines
/*0x318*/ DWORD FontSize;
/*0x268*/ DWORD Unknown0x23C;
/*0x26c*/
} EQCHATWINDOW, *PEQCHATWINDOW;

// actual size 0x14 10-12-2010
//I think this is correct:
//see (69FF1E) in eqgame.exe dated 2013 11 13 -eqmule
typedef struct _EQINVSLOT {
/*0x00*/    LPVOID pvfTable;        // not based on cxwnd
/*0x04*/    struct _EQINVSLOTWND *pInvSlotWnd;
/*0x08*/    DWORD Unknown0x08;
/*0x0C*/    DWORD InvSlot;
/*0x10*/    BYTE Valid;
/*0x11*/    BYTE Unknown0x11[3];
/*0x14*/    
} EQINVSLOT, *PEQINVSLOT;

// actual size 0x2014 10-12-2010
// confirmed (size) 2013 dec 10 eqgame.exe at 497E7A -eqmule
// confirmed 2014-03-31 - eqmule
typedef struct _EQINVSLOTMGR {
/*0x0000*/    DWORD Unknown0x0000;
/*0x0004*/    struct _EQINVSLOT *SlotArray[0x800];
/*0x2004*/    DWORD TotalSlots;
/*0x2008*/    DWORD Unknown0x2008;
/*0x200c*/    struct _EQINVSLOT *pSelectedItem;
/*0x2010*/    DWORD Unknown0x2010;
/*0x2014*/
} EQINVSLOTMGR, *PEQINVSLOTMGR;

// onetimehero 09-17-03
// ContainerWindow
// Actual Size 0x17C old
typedef struct _EQCONTAINERWINDOW {
/*0x000*/ struct _CSIDLWND Wnd;
/*0x148*/ struct _CONTENTS*   pContents;         // Pointer to the contents of the container;
                                                 // Matches the pointer in CHARINFO.Inventory/Bank/World
/*0x14c*/ struct _CSIDLWND*   pSlots[0x0a];
/*0x000*/ struct _CSIDLWND*   pCombine;
/*0x168*/ struct _CSIDLWND*   pDone;
/*0x16c*/ struct _CSIDLWND*   pIcon;
/*0x170*/ struct _CSIDLWND*   pUnknown;
/*0x174*/ struct _CSIDLWND*   pLabel;
/*0x178*/ BYTE Unknown0x178[4];
/*0x17c*/
} EQCONTAINERWINDOW, *PEQCONTAINERWINDOW;

// Actual Size 0x98  03/15/06
typedef struct _EQ_CONTAINERWND_MANAGER {
/*0x000*/   DWORD pvfTable;                           // NOT based on CXWnd.  Contains only destructor
/*0x004*/   PEQCONTAINERWINDOW pPCContainers[0x22];   // All open containers, including World, in order of opening...
/*0x08c*/   PCONTENTS   pWorldContents;               // Pointer to the contents of the world   If NULL, world container isn't open;
/*0x090*/   DWORD dwWorldContainerID;                 // ID of container in zone, starts at one (zero?) and goes up.
/*0x094*/   DWORD dwTimeSpentWithWorldContainerOpen;  // Cumulative counter?
/*0x078*/
} EQ_CONTAINERWND_MANAGER, *PEQ_CONTAINERWND_MANAGER;

typedef struct _POINT3 {
    FLOAT X;
    FLOAT Y;
    FLOAT Z;
} POINT3, *PPOINT3;

// plazmic 08-17-03
// Map Window
typedef struct _MAPLABEL { // sizeof() = 0x38
/*0x00*/    DWORD Unknown0x0;
/*0x04*/    struct _MAPLABEL *pNext;
/*0x08*/    struct _MAPLABEL *pPrev;
/*0x0c*/    POINT3 Location;
/*0x18*/    ARGBCOLOR Color;
/*0x1c*/    DWORD Size;     //1-3;
/*0x20*/    PCHAR Label;
/*0x24*/    DWORD Layer;    //0-3;
/*0x28*/    DWORD Width;
/*0x2c*/    DWORD Height;
/*0x30*/    DWORD unk_0x2c;
/*0x34*/    DWORD unk_0x30;
} MAPLABEL, *PMAPLABEL;

typedef struct _MAPLINE { // sizeof() = 0x28 (think this might be 0x34 now)
    struct _MAPLINE *pNext;
    struct _MAPLINE *pPrev;
    POINT3 Start;
    POINT3 End;
    ARGBCOLOR Color;
    DWORD Layer;          //0-3;
} MAPLINE, *PMAPLINE;

typedef struct _EQTRADESKILLRECIPE {
/*0x00*/ DWORD Unknown0x00;
/*0x04*/ DWORD Trivial;
/*0x08*/ DWORD Unknown0x08;
/*0x0c*/ DWORD Unknown0x0c;
/*0x10*/ DWORD Unknown0x10;
/*0x14*/ CHAR  Name[0x40];
/*0x54*/ DWORD Unknown0x54;
/*0x58*/ DWORD Ingredient[0xa];     // ID# of ingredient(s)
/*0x80*/ DWORD IngredientIcon[0xa]; //Icon# of ingredient(s) (Note: 499 is blank)
} EQTRADESKILLRECIPE, *PEQTRADESKILLRECIPE;

//Size 0x3d8 (02/10/2004)
typedef struct _EQTRADESKILLWINDOW {
/*0x000*/   struct _CSIDLWND Wnd;
/*0x148*/ DWORD Unknown0x148[0x1b];  
/*0x1b4*/ PEQTRADESKILLRECIPE SearchResults[0x64]; 
/*0x344*/ DWORD    Unknown0x344;
/*0x348*/ DWORD Unknown0x348;
/*0x34c*/ DWORD Unknown0x34c;
/*0x350*/ PCONTENTS Container;
/*0x354*/ DWORD Unknown0x354;
/*0x358*/ PEQTRADESKILLRECIPE SelectedRecipe;
/*0x35c*/ DWORD Unknown0x35c;
/*0x360*/ DWORD SkillLevel;
/*0x364*/ DWORD Unknown0x364;
/*0x368*/ DWORD Unknown0x368;
/*0x36c*/ BYTE  Unknown0x36c[0x58];
/*0x3c4*/ DWORD Unknown0x3c4;
/*0x3c8*/ DWORD Unknown0x3c8;
/*0x3cc*/ BYTE  Unknonw0x3cc[0x8];
/*0x3d4*/ DWORD Unknown0x3d4;
/*0x3d8*/ DWORD Unknown0x3d8;
} EQTRADESKILLWINDOW, *PTRADESKILLWINDOW;

typedef struct _GUILDMEMBERINFO {
/*0x000*/ DWORD Unknown0x000; //pointer?
/*0x004*/ BYTE OnlineStatus;  // 00=offline, 01=online
/*0x005*/ BYTE UnknownData0x005;
/*0x006*/ BYTE UnknownData0x006;
/*0x007*/ BYTE Unknown0x007;
/*0x008*/ BYTE Unknown0x008[0x8];
/*0x010*/ CHAR Name[0x40];
/*0x050*/ DWORD Level;
/*0x054*/ DWORD Class; 
/*0x058*/ DWORD Rank;         //0=member, 1=officer, 2=leader
/*0x05c*/ FLOAT UnknownData0x05c;
/*0x060*/ CHAR PublicNote[0x80];
/*0x0e0*/ BYTE Unknown0x0e0[0x180];
/*0x260*/ WORD UnknownData0x260;
/*0x262*/ WORD UnknownData0x262;
/*0x264*/ WORD UnknownData0x264;
/*0x266*/ WORD UnknownData0x266;
} GUILDMEMBERINFO, *PGUILDMEMBERINFO;

//Size: 0x3d0 (02/18/2004)
typedef struct _EQGUILDWINDOW {
/*0x000*/ struct _CSIDLWND Wnd;
/*0x148*/ BYTE  Unknown0x148[0x4c];
/*0x194*/ BYTE Unknown0x194;
/*0x195*/ BYTE ShowOffline;                  // 01 = show offline box is checked
/*0x196*/ BYTE Unknown0x196[0x2];
/*0x198*/ struct _GUILDMEMBERINFO **pMember; // This is a pointer to the beginning of a list of pointers, 
                                             // each representing one player in the guild.  The number of 
                                             // pointers depends upon TotalMemberCount.
/*0x19c*/ DWORD TotalMemberCount;
/*0x1a0*/ DWORD Unknown0x1a0;                // 150?
/*0x1a4*/ DWORD Unknown0x1a4;                // 1?
/*0x1a8*/ DWORD TotalMemberCountAgain;
/*0x1ac*/ BYTE Unknown0x1ac[0x1c];
/*0x1c8*/ CHAR PersonalNotesFilePath[0x40];  // path to personal notes file
/*0x208*/ BYTE Unknown0x208[0x1c0];          // empty
/*0x3c8*/ DWORD Unknown0x3c8;                // 1?
/*0x3cc*/ DWORD Unknown0x3cc;                // some sort of bizaare pointer...
/*0x3d0*/
} EQGUILDWINDOW, *PEQGUILDWINDOW;

typedef struct _EQNOTESWINDOW {
/*0x000*/ struct _CSIDLWND Wnd;
/*0x148*/ BYTE  Unknown0x148[0x04];
/*0x14c*/ struct _CSIDLWND *pEditWnd;
} EQNOTESWINDOW, *PEQNOTESWINDOW;

// Actual size 0x1c8 07-09-2008
typedef struct _EQSPELLINFOWINDOW 
{ 
/*0x000*/ struct _CSIDLWND Wnd;
/*0x208*/ PVOID Unknown0x190;
/*0x20c*/ PVOID Unknown0x194;
/*0x210*/ struct _CSIDLWND *DisplayWnd;
/*0x214*/ PVOID Unknown0x19c; 
/*0x218*/ PVOID Unknown0x1a0; 
/*0x21c*/ PCXSTR ItemInfo;
/*0x220*/ PVOID Unknown0x1a8;
/*0x224*/ DWORD Unknown0x1ac;
/*0x228*/ DWORD Unknown0x1b0;
/*0x22c*/ DWORD Unknown0x1b4;
/*0x230*/ DWORD Unknown0x1b8;
/*0x234*/ PVOID TextureAnim;
/*0x238*/ DWORD SpellID;
/*0x23c*/ DWORD TimeStamp;
/*0x240*/ 
} EQSPELLINFOWINDOW, *PEQSPELLINFOWINDOW;

// Size: 0x1a4 (4/14/2005)
typedef struct _EQMAILWINDOW {
/*0x000*/ struct _CSIDLWND Wnd;
/*0x158  Finish... */
} EQMAILWINDOW, *PEQMAILWINDOW;

#define Fly                     0

// Actual size 0x1c4 10-9-2003
typedef struct _EQHOTBUTTONWINDOW {
/*0x000*/ struct _CSIDLWND Wnd;
/*0x148*/ BYTE Unknown0x138[0xc];
/*0x000*/ DWORD   HotButtonWndView;     //0 to 9 for the different views
/*0x14c*/ struct _CSIDLWND   *HotButtons[0x0a];//these will change when you switch page...
/*0x174*/
} EQHOTBUTTONWINDOW, *PEQHOTBUTTONWINDOW;

typedef struct _LOOTCORPSE {
/*0x000*/   BYTE    Unknown0x000;   // 03 seems very common (for NPC anyway)
/*0x001*/   CHAR    Name[0x4b];
/*0x04c*/   FLOAT   Timer;          //Uncertain the equation used here, but fairly certain it's the timer
/*0x050*/   FLOAT   X;
/*0x054*/   BYTE    Unknown0x054[0x4];
/*0x058*/   FLOAT   Y;
/*0x05c*/   FLOAT   Z;
/*0x060*/   BYTE    Unknown0x060[0x24];
/*0x084*/   DWORD   Flags;
/*0x088*/   BYTE    Unknown0x088[0x4];
/*0x08c*/   DWORD   Unknown0x08c;   //256 seems very common
/*0x090*/
// I think there might be pointers at 0x0e0..but no clue what to
} LOOTCORPSE, *PLOOTCORPSE;

// size 0x180 3-10-2004
typedef struct _EQCOMPASSWINDOW {
/*0x000*/    struct _CSIDLWND Wnd;
/*0x148*/    struct _CSIDLWND *pStrip1;
/*0x14C*/    struct _CSIDLWND *pStrip2;
// in progress.
/*0x16C*/   BOOL DrawLine;
/*0x170*/   DWORD Unknown0x170;
/*0x174*/    DWORD LineRed;
/*0x178*/    DWORD LineGreen;
/*0x17C*/    DWORD LineBlue;
/*0x180*/
} EQCOMPASSWINDOW, *PEQCOMPASSWINDOW;

// used by options window
typedef struct _EQKBASSIGN
{
/*0x00*/    struct _CXSTR *pDescription;
/*0x04*/    DWORD nAssignmentNumber;
/*0x08*/
} EQKBASSIGN, *PEQKBASSIGN;

// size 0x904 3-10-2004
typedef struct _EQOPTIONSWINDOW {
/*0x000*/    struct _CSIDLWND Wnd;
/*0x148*/
/*0x264*/ struct _EQKBASSIGN Binds[0xA1];
/*0x508*/
/*0x904*/
} EQOPTIONSWINDOW, *PEQOPTIONSWINDOW;

// size 0x28 4-24-2004 Lax
typedef struct _EQGRAPHICSENGINE {
/*0x00*/ // Graphics file handler
/*0x04*/
} EQGRAPHICSENGINE, *PEQGRAPHICSENGINE;
#define SafeXLoc 0

//CButtonWnd__CButtonWnd_x
//size is 0x280 see 8D18BE in eqgame.exe dated 11 Aug 2016 - eqmule
typedef struct _CBUTTONWND {
/*0x000*/ struct    _CXWND Wnd;
/*0x1e8*/ BYTE      Unknown[0x4];
/*0x1ec*/ BYTE      State;        //1=down 0=up
/*0x1ed*/ BYTE      Unknown0x1ed; //something to do with State
/*0x1ee*/ BYTE      Unknown0x1ee[0x12];
/*0x200*/ ARGBCOLOR Color;
/*0x204*/ BYTE      Unknown0x204[0x58];
/*0x25c*/ DWORD     TextureAnim;//im guessing now but I think this is the icon displayed use iconcache->geticon to fill it in...
/*0x260*/ BYTE      Unknown0x260[0x20];
/*0x280*/
} CBUTTONWND, *PCBUTTONWND;
 
typedef struct _CTEXTENTRYWND {
/*0x000*/ struct    _CXWND Wnd;
/*0x1c4*/ DWORD     CursorPos1;
/*0x13c*/ DWORD     CursorPos2; //same as above but only this needs to be set to update cursor position
/*0x140*/
} CTEXTENTRYWND, *PCTEXTENTRYWND;

typedef struct _CLABELWND {
/*0x000*/ struct    _CXWND Wnd;
/*0x1c4*/ BYTE      Unknown[0x8];
/*0x1cc*/ DWORD     SidlPiece;
/*0x1d0*/
} CLABELWND, *PCLABELWND;

typedef struct _LOOTDETAILS
{
/*0x00*/ DWORD	CorpseID;//spawnId of the corpse that has this lootitem
/*0x04*/ WORD	StackCount;
/*0x06*/ WORD	UnknownWord;
/*0x08*/ DWORD	Expiration;
/*0x0c*/ BYTE	Locked;
/*0x0d*/ CHAR	Name[0x40];
/*0x4d*/ //more data here? -eqmule
}LOOTDETAILS,*PLOOTDETAILS;
//.text:0041ECBD                 imul    eax, 84h in Apr 15 2015 test
enum eAdvLootState
{
	eAdvLootWaiting,
	eAdvLootAsk,
	eAdvLootAskAutoRoll,
	eAdvLootStop,
	eAdvLootAskCompleted,
	eAdvLootFreeGrab,
	eAdvLootFixedAskAutoRoll,
	eAdvLootFixedAskCompleted,
	eAdvLootRemoved
};
typedef struct _LOOTITEM
{
/*0x00*/ DWORD	ItemID;
/*0x04*/ CHAR	Name[0x40];
/*0x44*/ DWORD	IconID;
/*0x48*/ BYTE  IsStackable;
/*0x49*/ BYTE   Unknown0x49[0x3];
/*0x4c*/ DWORD  MaxStack;
/*0x50*/ BYTE   NoDrop;
/*0x51*/ BYTE   Unknown0x51[0x3];
/*0x54*/ DWORD  ComboID;
/*0x58*/ DWORD  LootID;
/*0x5c*/ eAdvLootState State;
/*0x60*/ BYTE	bAutoRoll;
/*0x61*/ BYTE	ActivelyManaged; // User has the manage Window up
/*0x62*/ BYTE	ContextMenu;     // item has a context menu
/*0x63*/ BYTE	AskRandomMode; //item is in AskRandom mode
/*0x64*/ BYTE   CLootInProgress;
/*0x65*/ BYTE   PLootInProgress;
/*0x66*/ BYTE   Unknown0x66[0x6];
/*0x6c*/ struct _LOOTDETAILS	*LootDetails;
/*0x70*/ BYTE	Unknown0x70[0x8];
/*0x78*/ DWORD	AskTimer;
/*0x7c*/ BYTE	AutoRoll;
/*0x7d*/ BYTE	Unknown0x7d;
/*0x7e*/ BYTE	Need;
/*0x7f*/ BYTE	Greed;
/*0x80*/ BYTE	No;
/*0x81*/ BYTE	AlwaysNeed;
/*0x82*/ BYTE	AlwaysGreed;
/*0x83*/ BYTE	Never;
/*0x84*/
} LOOTITEM,*PLOOTITEM;

typedef struct _LOOTLIST {
/*0x000*/ BYTE	Unknown0x004[0x4];
/*0x004*/ struct _LOOTITEM *pLootItem;
/*0x008*/ LONG	 ListSize;
/*0x00c*/ LONG	 Unknown0x00c;
/*0x010*/ LONG	 Unknown0x010;
/*0x014*/ struct _CXWND	 *SharedLootList;
/*0x018*/ struct _CXWND	 *PersonalLootList;
/*0x01c*/ LONG	 Unknown0x01c;
/*0x020*/ LONG	 Unknown0x020;
/*0x024*/
} LOOTLIST,*PLOOTLIST;

//CAdvancedLootWnd__CAdvancedLootWnd_x
//size 0x2f0 see 4BB5C9 in Aug 11 2016 Test -eqmule
typedef struct _EQADVLOOTWND {
/*0x000*/ struct _CSIDLWND     Wnd;
/*0x230*/ BYTE	Unknown0x0230[0x94];
/*0x2c4*/ struct _LOOTLIST *pCLootList;//below ref to aAdlw_applyfilt
/*0x2c8*/ struct _LOOTLIST *pPLootList;//below ref to aAdlw_cllwnd
/*0x2cc*/ DWORD		Unknown0x2cc;
/*0x2d0*/ DWORD		Unknown0x2d0;
/*0x2d4*/ DWORD		Unknown0x2d4;
/*0x2d8*/ DWORD		TotalLootCount;
/*0x2dc*/ DWORD		Unknown0x2dc;
/*0x2e0*/ DWORD		ContextMenuId;
/*0x2e4*/ DWORD		CLastStackSize;
/*0x2e8*/ DWORD		Unknown0x2e8[0x8];
/*0x2f0*/
} EQADVLOOTWND, *PEQADVLOOTWND;

//.text:005FC640 ; public: __thiscall CTargetIndicator::CTargetIndicator(void)
//size 0x48 see 4BA434 in Oct 26 2015 -eqmule
typedef struct _TARGETINDICATOR
{
/*0x00*/	BYTE Unknown0x00;
/*0x01*/	BYTE Unknown0x01;
/*0x02*/	BYTE Unknown0x02[0x2];
/*0x04*/	PSPAWNINFO lasttarget;
/*0x08*/	BYTE Unknown0x08[0x10];
/*0x18*/	BYTE IndicatorHidden;//if this is 1 our mouse is over another window and not on the main one - shouldnt cast when this is 1
/*0x19*/	BYTE CanActivate;//if 0 the indicator is red if 1 its green, i.e. it can be activated.
/*0x1a*/	BYTE Unknown0x1a[0x2];
/*0x1c*/	DWORD somepointer;
/*0x20*/	DWORD somepointer2;
/*0x24*/	BYTE ValidForSplash;//this is reliable IF you actually have a target: 3 is self and your merc, 4 is other player, 0 is npc...
/*0x25*/	BYTE Unknown0x25[0x23];
/*0x48*/
} TARGETINDICATOR,*PTARGETINDICATOR;

//.text:005FE3E0 ; private: __thiscall CTargetManager::CTargetManager(void)
//size 0x1a4 see 5FE489 in Oct 26 2015 -eqmule
typedef struct _TARGETMANAGER
{
/*0x000*/ BYTE Unknown0x000[0x184];
/*0x184*/ DWORD TargetID;
/*0x188*/ BYTE Unknown0x188[0x4];
/*0x18c*/ BYTE target_ring_in_focus;
/*0x18d*/ BYTE Unknown0x18d[3];
/*0x190*/ float target_ring_range; // squared for easier math
/*0x194*/ float target_ring_x; // probably vec3 type class, this is whats passed to castspell
/*0x198*/ float target_ring_y;
/*0x19c*/ float target_ring_z;
/*0x1a0*/ BYTE target_ring_good; //0=red 1=green 
/*0x1a1*/ BYTE Unknown0x1a1[3];
/*0x1a4*/
} TARGETMANAGER, *PTARGETMANAGER;

//Jul 18 2015 -eqmule
typedef struct _ScreenVector3
{
	union {
		float x; // left to right screen coordinate
		DWORD dx;
	};
	union {
		float y; // top to bottom screen coordinate
		DWORD dy;
	};
	union {
		float z;
		DWORD dz;
	};
} ScreenVector3, *PScreenVector3;
//oct 26 2015 - eqmule 
typedef struct _TARGETRING {
/*0x00*/	DWORD Gem;//the gem the spell below is memmed in... 0-11
/*0x04*/	PSPELL thespell;
/*0x08*/	ScreenVector3 thevec;
/*0x14*/	BOOL Param;
/*0x18*/
} TARGETRING, *PTARGETRING;

/******************************************** CHECKED *****************************************************************/
//everything above this line is work in progress/stuff that can be improved/checked/fixed.
//everything below it is 100% checked -eqmule

//CBazaarSearchWnd__CBazaarSearchWnd aBazaarsearchwn
// Actual Size: 0x92d0 (see 4B96CA) in Aug 11 2016 Test - eqmule
typedef struct _BAZAARSEARCHWND {
/*0x0000*/ struct _CSIDLWND Wnd;
/*0x0230*/ BYTE Unknown0x0230[0x8ff8];
/*0x9228*/ void ** ppTraderData;
/*0x922c*/ DWORD hashVal;//find in CBazaarSearchWnd__HandleBazaarMsg_x
/*0x9230*/ BYTE Unknown0x9230[0xa0];
/*0x92d0*/
} BAZAARSEARCHWND, *PBAZAARSEARCHWND;

//CPlayerWindow__CPlayerWindow aPlayerwindow
//Note to self: cant actually find CombatState in it, so no point in looking through IDA for it, but it IS the last dword... so... until that changes, im just gonna accept it...
// Actual Size: 0x2e8 (see 4B97DD) in May  6 2016 Test - eqmule
typedef struct _CPLAYERWND {
/*0x0000*/ struct _CSIDLWND     Wnd;
/*0x0230*/ BYTE Unknown0x0230[0xb4];
/*0x02e4*/ DWORD CombatState;   // 1=debuffed, 2=combat cooldown, 3=stand, 4=sit
/*0x02e8*/
} CPLAYERWND, *PCPLAYERWND;

// CTargetWindow__CTargetWindow aTargetwindow
// Actual Size: 0x8c8 (see 4B8E60) in Aug 11 2016 Test - eqmule
typedef struct _CTARGETWND {
/*0x0000*/ struct  _CSIDLWND    Wnd;
/*0x0230*/ BYTE         Unknown0x0230[0x198];
/*0x03c8*/ struct _CBUTTONWND * pBuff[NUM_BUFF_SLOTS]; // buff icons
/*0x054c*/ int          BuffSpellID[NUM_BUFF_SLOTS]; // 0xffffffff if no buff
/*0x06d0*/ DWORD        BuffTimer[NUM_BUFF_SLOTS];
/*0x0854*/ BYTE         Unknown0x0854[0x24];
/*0x0878*/ DWORD        Type;              // 1 = self, 4 = group member, 5 = PC, 7 = NPC
/*0x087c*/ BYTE         Unknown0x087c[0x4c];
/*0x08c8*/
} CTARGETWND, *PCTARGETWND;

// CBuffWindow__CBuffWindow aBuffwindow
// this is used for both long and shortbuffs...
// Actual Size: 0x718 (see 4B9504) in May  6 2016 Test - eqmule
typedef struct _EQBUFFWINDOW {
/*0x000*/ struct _CSIDLWND     Wnd;
/*0x230*/ BYTE         Unknown0x0230[0xbc];
/*0x2ec*/ struct _CBUTTONWND * pBuff[0x24];    // CButton*
/*0x37c*/ BYTE         Unknown0x037c[0x210];
/*0x58c*/ DWORD        BuffId[NUM_LONG_BUFFS];
/*0x634*/ DWORD        BuffTimer[NUM_LONG_BUFFS];
/*0x6dc*/ BYTE         Unknown0x06dc[0x28];
/*0x704*/ DWORD        MaxLongBuffs;           //0x2a (NUM_LONG_BUFFS)
/*0x708*/ DWORD        MaxShortBuffs;          //0x37 (NUM_SHORT_BUFFS)
/*0x6f4*/ BYTE         Unknown0x06f4[0xc];
/*0x718*/
} EQBUFFWINDOW, *PEQBUFFWINDOW;

// CSpellGemWnd__CSpellGemWnd
// Individual Gems 
// Actual Size: 0x318 (see 794FC8) in Aug 11 2016 Test - eqmule
typedef struct _EQCASTSPELLGEM {
/*0x0000*/ struct    _CXWND     Wnd;
/*0x01e8*/ BYTE         Unknown0x01e8[0x44];
/*0x022c*/ DWORD        TimeStamp;
/*0x0230*/ DWORD        RecastTime;
/*0x0234*/ BYTE         Unknown0x0234[0xb8];
/*0x02ec*/ DWORD        spellicon;       //if this is equal to FFFFFFFF there is no spell memmed in this slot...
/*0x02f0*/ DWORD        spellstate;      // 1 = cast in progress or refreshtime not met 2 means we ducked or aborted cast, 0 means its ok to cast
/*0x02f4*/ BYTE         Unknown0x02f4[0x24];
/*0x0318*/
} EQCASTSPELLGEM, *PEQCASTSPELLGEM;

//pinstCCastSpellWnd_x
// CCastSpellWnd__CCastSpellWnd aCastspellwnd
// Actual Size: 0x2c8 (see 4B90DE) in Aug 11 2016 Test - eqmule
typedef struct _EQCASTSPELLWINDOW {
/*0x0000*/ struct _CSIDLWND     Wnd;
/*0x0230*/ BYTE Unknown0x0230[0x14];
/*0x0244*/ struct _EQCASTSPELLGEM * SpellSlots[NUM_SPELL_GEMS];//CSPW_Spell%d
/*0x0274*/ BYTE Unknown0x0274[0x54];
/*0x02c8*/
} EQCASTSPELLWINDOW, *PEQCASTSPELLWINDOW;

// Actual Size: 0x2d0 (see 7950D9) in Aug 11 2016 Test - eqmule
typedef struct _EQINVSLOTWND {
/*0x0000*/ struct _CXWND Wnd;      //----/ actually CButtonWnd
/*0x01e8*/ BYTE Unknown0x01e8[0x8c];
/*0x0274*/ BYTE Unknown0x0274[0x10];
/*0x0284*/ LONG WindowType;        // ieatacid has this as InventoryType
									// 00 for inventory
									// 01 for bank
									// 02 for shared bank
									// 03 for trader window
									// 04 for World/Tradeskill/Combine
									// 11 for loot window
/*0x0288*/ WORD InvSlot;
/*0x028a*/ WORD BagSlot;
/*0x028c*/ WORD GlobalSlot;
/*0x028e*/ WORD RandomNum;              //no idea what this is, it changes upon login but we need it for moveitem... -eqmule
/*0x0290*/ BYTE Unknown0x0290[0x20];
/*0x02b0*/ struct _EQINVSLOT *  pInvSlot;
/*0x02b4*/ BYTE Unknown0x02b4[0x8];
/*0x02bc*/ BOOL ProcessClick;
/*0x02c0*/ BYTE Unknown0x02c0[0x10];
/*0x02d0*/
} EQINVSLOTWND, *PEQINVSLOTWND;

// CItemDisplayWindow__CItemDisplayWindow aItemdisplaywin
// Actual Size: 0x608 (see 6DFEDF) in Aug 11 2016 Test - eqmule
typedef struct _EQITEMWINDOW {
/*0x0000*/ struct _CSIDLWND     Wnd;
/*0x0230*/ struct _CSIDLWND *   DisplayWnd;//ItemDescription
/*0x0234*/ BYTE         Unknown0x0234[0x4];
/*0x0238*/ struct _CSIDLWND *IconButton;
/*0x023c*/ struct _CSIDLWND *ItemLore;
/*0x0240*/ struct _CSIDLWND *ItemDescriptionTabBox;
/*0x0244*/ struct _CSIDLWND *ItemDescriptionTab;
/*0x0248*/ struct _CSIDLWND *ItemLoreTab;
/*0x024c*/ BYTE         Unknown0x024c[0x58];
/*0x02a4*/ PCXSTR       ItemInfo;//this item is placable in yards, guild yards blah blah , This item can be used in tradeskills
/*0x02a8*/ PCXSTR       WindowTitle;
/*0x02ac*/ PCXSTR       ItemAdvancedLoreText;
/*0x02b0*/ PCXSTR       ItemMadeByText;
/*0x02b4*/ PCXSTR       UnknownCXStr; // if this is NULL don't populate item data in MQ2ItemDisplay
/*0x02b8*/ BYTE         Unknown0x02b8[0x4];
/*0x02bc*/ PCXSTR       ItemInformationText;//Item Information: Placing this augment into blah blah, this armor can only be used in blah blah
/*0x02c0*/ PCONTENTS    pItem;
/*0x02c4*/ BYTE         Unknown0x02c4[0x340];
/*0x0604*/ DWORD        ItemWndIndex;//0-5? you can have max 6 windows up I think before it starts overwriting the sixth.
/*0x0608*/
} EQITEMWINDOW, *PEQITEMWINDOW;

//CLootWnd__CLootWnd aLootwnd
// Actual Size: 0x398 (see 4B94A0) in Aug 11 2016 Test - eqmule
typedef struct _EQLOOTWINDOW {
/*0x0000*/ struct _CSIDLWND     Wnd;
/*0x0230*/ void *       vftable; // for CLootWnd::DialogResponse handler
/*0x0234*/ BYTE         Unknown0x0234[0x98];
/*0x02cc*/ DWORD        NumOfSlots;
/*0x02d0*/ BYTE         Unknown0x02d0[0x4];
/*0x02d4*/ struct _INVENTORYARRAY *     pInventoryArray;
/*0x02d8*/ DWORD        Size;
/*0x02dc*/ DWORD        NumOfSlots3;
/*0x02e0*/ BYTE         Unknown0x02e0[0x8];
/*0x02e8*/ BYTE         Unknown0x02e8;
/*0x02e9*/ BYTE         Unknown0x02e9;
/*0x02ea*/ BYTE         Unknown0x02ea;
/*0x02eb*/ BYTE         Unknown0x02eb;
/*0x02ec*/ struct _CSIDLWND *   LootInvWnd;
/*0x02f0*/ struct _CSILDWND *   LootSlotWnd[0x22];
/*0x0378*/ struct _CSIDLWND *   LW_CorpseName;
/*0x037c*/ struct _CSIDLWND *   DoneButton;
/*0x0380*/ struct _CSIDLWND *   BroadcastButton;
/*0x0384*/ struct _CSIDLWND *   LootAllButton;
/*0x0388*/ BYTE         Unknown0x0388[0x10];
/*0x0398*/
} EQLOOTWINDOW, *PEQLOOTWINDOW;

// pLines address = 0x254 + 0x035c = 0x05b0 (address of pMapViewMapVfTable)
// CMapViewWnd__CMapViewWnd_x
// Actual Size: 0x648 (see 4B8B55) in Aug 11 2016 Test - eqmule
typedef struct _EQMAPWINDOW {
/*0x0000*/ struct _CSIDLWND Wnd;
/*0x0230*/ BYTE Unknown0x0230[0x40];
/*0x0270*/ CHAR shortzonename[0x20];
/*0x0290*/ BYTE Unknown0x0290[0x9c];
/*0x032c*/ struct _CXWND * wnd;           // its the MVW_MapRenderArea window... found at aMvw_maprendera
/*0x0330*/ BYTE Unknown0x0330[0x38];
/*0x0368*/ struct _CSIDLWNDVFTABLE * pMapViewMapVfTable; // found at aMapviewmap
/*0x036c*/ BYTE Unknown0x036c[0x25c];
/*0x05c8*/ PMAPLINE pLines;     //0x260 + 0x0368 = 0x5c8 (address of pMapViewMapVfTable)
/*0x05cc*/ PMAPLABEL pLabels;    //0x264
/*0x05d0*/ BYTE Unknown0x05d0[0x78];
/*0x0648*/
} EQMAPWINDOW, *PEQMAPWINDOW;

// 20120316 - ieatacid
// 20130819 confirmed  - eqmule
// 20160317 confirmed Size unknown - eqmule
struct merchdata {
/*0x00*/ void   *vftable;
/*0x04*/ BYTE   Unknown0x4[0x8];
/*0x0c*/ DWORD  MerchSlots;
/*0x10*/ DWORD  Unknown0x10;
/*0x14*/ struct _CONTENTSARRAY *pMerchArray;
/*0x18*/ DWORD  MerchMaxSlots;
/*0x1c*/
};

struct merch_other {
merchdata *pMerchData;
void *other;
void *other2;
};

//CMerchantWnd__CMerchantWnd (aMerchantwnd)
// Actual Size: 0x2d0 (see 4B95B9) in Aug 11 2016 Test - eqmule
typedef struct _EQMERCHWINDOW {
/*0x0000*/ struct _CSIDLWND Wnd;
/*0x0230*/ BYTE Unknown0x0230[0x10];
/*0x0240*/ merch_other * pMerchOther;//found in CMerchantWnd__CMerchantWnd
/*0x0244*/ BYTE Unknown0x0244[0x8];
/*0x024c*/ FLOAT Markup;//found in CMerchantWnd__DisplayBuyOrSellPrice_x
/*0x0250*/ BYTE Unknown0x0250[0xc];
/*0x025c*/ DWORD SelectedSlotID;//found in CMerchantWnd__RequestBuyItem_x
/*0x0260*/ BYTE Unknown0x0260[0x70];
/*0x02d0*/
} EQMERCHWINDOW, *PEQMERCHWINDOW;

//CPetInfoWindow__CPetInfoWindow aPetinfowindow
// Actual Size: 0x8c8 (see 4B8590) in Aug 11 2016 Test - eqmule
typedef struct _EQPETINFOWINDOW {
/*0x0000*/ struct _CSIDLWND     Wnd;
/*0x0230*/ DWORD PetSpawnID;//The pets SpawnID
/*0x0234*/ BYTE Unknown0x0234[0x4];
/*0x0238*/ struct _CBUTTONWND * pButton[0xe];//there are 14 buttons on the petinfowin with text that can be set to attack,none,back and so on, these are those...
/*0x0270*/ BYTE Unknown0x0270[0x8];
/*0x0278*/ struct _CBUTTONWND * pAttackButton;//if 0 its not assigned.
/*0x027c*/ struct _CBUTTONWND * pQAttackButton;//if 0 its not assigned.
/*0x0280*/ struct _CBUTTONWND * pFollowButton;//if 0 its not assigned.
/*0x0284*/ struct _CBUTTONWND * pGuardButton;//if 0 its not assigned.
/*0x0288*/ struct _CBUTTONWND * pSitButton;//if 0 its not assigned.
/*0x028c*/ struct _CBUTTONWND * pStopButton;//if 0 its not assigned.
/*0x0290*/ BYTE Unknown0x029c[0x174];
/*0x0404*/ struct _CSIDLWND *   pWnd[NUM_BUFF_SLOTS]; // buff icons?
/*0x0588*/ int Buff[NUM_BUFF_SLOTS];        // Spell ID# of each buff -- 97 total
/*0x070c*/ BYTE Unknown0x070c[0x20];
/*0x072c*/ DWORD PetBuffTimer[NUM_BUFF_SLOTS]; // duration until buff fades, in thousands of a second
/*0x08b0*/ BYTE Unknown0x08b0[0xd];
/*0x08bd*/ BYTE Sit;//0/1 is off/on
/*0x08be*/ BYTE Stop;//0/1 is off/on
/*0x08bf*/ BYTE ReGroup;//0/1 is off/on
/*0x08c0*/ BYTE Follow;//0/1 is off/on
/*0x08c1*/ BYTE Guard;//0/1 is off/on
/*0x08c2*/ BYTE Taunt;//0/1 is off/on
/*0x08c3*/ BYTE Hold;//0/1 is off/on
/*0x08c4*/ BYTE GHold;//0/1 is off/on
/*0x08c5*/ BYTE Focus;//0/1 is off/on
/*0x08c6*/ BYTE Unknown0x08c6[0x2];
/*0x08c8*/
} EQPETINFOWINDOW, *PEQPETINFOWINDOW;

//CRaidWindow__CRaidWindow
//aClasscolorD
// Actual Size: 0x3a8 (see 4B8883) in Aug 11 2016 Test - eqmule
typedef struct _EQRAIDWINDOW {
/*0x0000*/ struct _CSIDLWND     Wnd;
/*0x0230*/ BYTE Unknown0x0230[0xb8];
/*0x02e8*/ DWORD ClassColors[0x10];
/*0x0328*/ BYTE Unknown0x0328[0x80];
/*0x03a8*/
} EQRAIDWINDOW, *PEQRAIDWINDOW;

//CTradeWnd__CTradeWnd aTradewnd
// Actual Size: 0x2f0 (see 4B9614) in Aug 11 2016 Test - eqmule
typedef struct _EQTRADEWINDOW {
/*0x0000*/ struct _CSIDLWND Wnd;
/*0x0230*/ BYTE Unknown0x0230[0xb8];
/*0x02e8*/ BYTE HisTradeReady;
/*0x02e9*/ BYTE MyTradeReady;
/*0x02ea*/ BYTE TradeWndOpen;
/*0x02eb*/ BYTE Unknown0x02eb[0x5];
/*0x02f0*/
} EQTRADEWINDOW, *PEQTRADEWINDOW;

};
using namespace EQUIStructs;
#pragma pack(pop)