//-------------------------------------------------------------------------------------------------
//
// Keyboard - these values derived from map on this page http://beebwiki.mdfs.net/Keyboard
//
//-------------------------------------------------------------------------------------------------
#include "stdafx.h"

//-------------------------------------------------------------------------------------------------

static BBC_Keyboard::KeyCodeMapping g_keycodeMapping[] = 
{
	{	0x20,	"f0",			VK_F10,			VK_F10	   , 1},
	{	0x71,	"f1",			VK_F1,			VK_F1	   , 1},
	{	0x72,	"f2",			VK_F2,			VK_F2	   , 1},
	{	0x73,	"f3",			VK_F3,			VK_F3	   , 1},
	{	0x14,	"f4",			VK_F4,			VK_F4	   , 1},
	{	0x74,	"f5",			VK_F5,			VK_F5	   , 1},
	{	0x75,	"f6",			VK_F6,			VK_F6	   , 1},
	{	0x16,	"f7",			VK_F7,			VK_F7	   , 1},
	{	0x76,	"f8",			VK_F8,			VK_F8	   , 1},
	{	0x77,	"f9",			VK_F9,			VK_F9	   , 1},
	{	0x70,	"ESCAPE",		VK_ESCAPE,		VK_ESCAPE  , 1},
	{	0x70,	"ESCAPE",		0x1B,			0x1B	   , 0},
	{	0x30,	"!1",			'1',			'!'		   , 0},
	{	0x31,	"\"2",			'2',			'\"'	   , 0},
	{	0x11,	"#3",			'3',			'#'		   , 0},
	{	0x12,	"$4",			'4',			'$'		   , 0},
	{	0x13,	"%5",			'5',			'%'		   , 0},
	{	0x34,	"&6",			'6',			'&'		   , 0},
	{	0x24,	"'7",			'7',			'\''	   , 0},
	{	0x15,	"(8",			'8',			'('		   , 0},
	{	0x26,	")9",			'9',			')'		   , 0},
	{	0x27,	"Ø",			'0',			'0'		   , 0},
	{	0x17,	"=-",			'-',			'='		   , 0},
	{	0x18,	"~^",			'^',			'~'		   , 0},
	{	0x78,	"|\\",			'\\',			'|'		   , 0},
	{	0x19,	"LEFT",			VK_LEFT,		VK_LEFT	   , 1},
	{	0x79,	"RIGHT",		VK_RIGHT,		VK_RIGHT   , 1},
	{	0x60,	"TAB",			VK_TAB,			VK_TAB	   , 1},
	{	0x10,	"Q",			'Q',			'q'		   , 0},
	{	0x21,	"W",			'W',			'w'		   , 0},
	{	0x22,	"E",			'E',			'e'		   , 0},
	{	0x33,	"R",			'R',			'r'		   , 0},
	{	0x23,	"T",			'T',			't'		   , 0},
	{	0x44,	"Y",			'Y',			'y'		   , 0},
	{	0x35,	"U",			'U',			'u'		   , 0},
	{	0x25,	"I",			'I',			'i'		   , 0},
	{	0x36,	"O",			'O',			'o'		   , 0},
	{	0x37,	"P",			'P',			'p'		   , 0},
	{	0x47,	"@",			'@',			'@'		   , 0},
	{	0x38,	"{[",			'[',			'{'		   , 0},
	{	0x28,	"-",			'-',			-1		   , 0},
	{	0x28,	"£",			VK_F11,			VK_F11	   , 1}, // Note: no 1:1 mapping for '£' on a PC keyboard, so using F11 as a proxy
	{	0x39,	"UP",			VK_UP,			VK_UP	   , 1},
	{	0x29,	"DOWN",			VK_DOWN,		VK_DOWN	   , 1},
	{	0x40,	"CAPS LOCK",	VK_CAPITAL,		VK_CAPITAL , 1},
	{	0x01,	"CTRL",			VK_CONTROL,		VK_CONTROL , 1},
	{	0x41,	"A",			'A',			'a'		   , 0},
	{	0x51,	"S",			'S',			's'		   , 0},
	{	0x32,	"D",			'D',			'd'		   , 0},
	{	0x43,	"F",			'F',			'f'		   , 0},
	{	0x53,	"G",			'G',			'g'		   , 0},
	{	0x54,	"H",			'H',			'h'		   , 0},
	{	0x45,	"J",			'J',			'j'		   , 0},
	{	0x46,	"K",			'K',			'k'		   , 0},
	{	0x56,	"L",			'L',			'l'		   , 0},
	{	0x57,	"+;",			';',			'+'		   , 0},
	{	0x48,	"*:",			':',			'*'		   , 0},
	{	0x58,	"}]",			']',			'}'		   , 0},
	{	0x49,	"RETURN",		13,				13		   , 0},
	{	0x49,	"RETURN",		VK_RETURN,		VK_RETURN  , 1},
	{	0x50,	"SHIFT LOCK",	VK_RSHIFT,		VK_RSHIFT  , 1}, // Note: no 1:1 mapping on a PC keyboard
	{	0x00,	"SHIFT",		VK_SHIFT,		VK_SHIFT   , 1},
	{	0x61,	"Z",			'Z',			'z'		   , 0},
	{	0x42,	"X",			'X',			'x'		   , 0},
	{	0x52,	"C",			'C',			'c'		   , 0},
	{	0x63,	"V",			'V',			'v'		   , 0},
	{	0x64,	"B",			'B',			'b'		   , 0},
	{	0x55,	"N",			'N',			'n'		   , 0},
	{	0x65,	"M",			'M',			'm'		   , 0},
	{	0x66,	"<,",			',',			'<'		   , 0},
	{	0x67,	">.",			'.',			'>'		   , 0},
	{	0x68,	"?/",			'/',			'?'		   , 0},
	{	0x59,	"DELETE",		8,				8		   , 0},
	{	0x59,	"DELETE",		VK_BACK,		VK_BACK    , 1},
	{	0x69,	"COPY",			VK_MENU,		VK_MENU	   , 1},	// Note: no 1:1 mapping on a PC keyboard
	{	0x62,	"SPACE",		VK_SPACE,		VK_SPACE   , 1},
	{	0x62,	"SPACE",		' ',			' '        , 0},
};

//-------------------------------------------------------------------------------------------------

BBC_Keyboard::BBC_Keyboard( System_VIA_6522& sysVia )
	: m_sysVia( sysVia )
{
	memset( m_asciiToScanCodeMap, 0, sizeof( m_asciiToScanCodeMap ) );
	memset( m_vkeyToScanCodeMap, 0, sizeof( m_vkeyToScanCodeMap ) );
	memset( m_keys, 0, sizeof( m_keys ) );

	for ( int i =0 ; i < sizeof(g_keycodeMapping)/sizeof(KeyCodeMapping); i++ )
	{
		KeyCodeMapping& mapping = g_keycodeMapping[ i ];
		if ( mapping.is_a_vkey )
		{
			m_vkeyToScanCodeMap[ mapping.vkey		] = &mapping;
			m_vkeyToScanCodeMap[ mapping.vkey_shift ] = &mapping;
		}
		else
		{
			m_asciiToScanCodeMap[ mapping.vkey		 ] = &mapping;
			m_asciiToScanCodeMap[ mapping.vkey_shift ] = &mapping;
		}
	}
	ClearKeys();
}

//-------------------------------------------------------------------------------------------------
void BBC_Keyboard::SetKeyState( u8 vkey, bool bIsVKey, u8 down )
{
	KeyCodeMapping* mappping;
	bool shift = false;

	//
	// One mapping for vkeys another for ascii chars
	//
	if ( bIsVKey )
	{
		mappping = m_vkeyToScanCodeMap[ vkey ];
		if ( mappping )
		{
			m_keys[ mappping->scanCode ] = down;
			m_sysVia.ScanKeyboard();
		}
	}
	else
	{
		mappping = m_asciiToScanCodeMap[ vkey ];
		if ( mappping )
		{
			//
			// If this key corresponded to a SHIFTED value on the BBC keyboard force the shift key to reflect that
			// And if it didn't, then force SHIFT off (even if we are pressing it on the PC keyboard)
			//
			m_keys[ mappping->scanCode ] = down;
			if ( mappping->vkey_shift == vkey )
			{
				m_keys[ m_vkeyToScanCodeMap[ VK_SHIFT ]->scanCode ] = down;
			}
			else
			{
				m_keys[ m_vkeyToScanCodeMap[ VK_SHIFT ]->scanCode ] = 0;
			}
			m_sysVia.ScanKeyboard();
		}
	}
	//
	// Handle break
	//
	if ( bIsVKey && vkey == VK_PAUSE )
	{
		m_bBreakPressed = true;
	}
}

//-------------------------------------------------------------------------------------------------

void BBC_Keyboard::SetKeyDown( u8 vkey, bool bIsVKey )
{
	SetKeyState( vkey, bIsVKey, 1 );
}

//-------------------------------------------------------------------------------------------------

void BBC_Keyboard::SetKeyUp( u8 vkey, bool bIsVKey )
{
	SetKeyState( vkey, bIsVKey, 0 );
}

//-------------------------------------------------------------------------------------------------

void BBC_Keyboard::ClearKeys( )
{
	memset( m_keys, 0, sizeof( m_keys ) );
	m_bBreakPressed = false;
	m_sysVia.ScanKeyboard();
}

//-------------------------------------------------------------------------------------------------

bool BBC_Keyboard::IsKeyDown_ScanCode( u8 scancode )
{
	return m_keys[ scancode ] != 0;
}

//-------------------------------------------------------------------------------------------------

