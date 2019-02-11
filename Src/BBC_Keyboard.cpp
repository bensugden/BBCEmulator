//-------------------------------------------------------------------------------------------------
//
// Keyboard - these values derived from map on this page http://beebwiki.mdfs.net/Keyboard
//
//-------------------------------------------------------------------------------------------------
#include "stdafx.h"
//-------------------------------------------------------------------------------------------------

struct KeyCodeMapping
{
	u8		scanCode;  // in ROW:4, COLUMN:4 format
	string	name; 
	int		vkey;
	int		vkey_shift;
};
	
//-------------------------------------------------------------------------------------------------

static KeyCodeMapping g_keycodeMapping[] = 
{
	{	0x20,	"f0",			VK_F10,			VK_F10	   },
	{	0x71,	"f1",			VK_F1,			VK_F1	   },
	{	0x72,	"f2",			VK_F2,			VK_F2	   },
	{	0x73,	"f3",			VK_F3,			VK_F3	   },
	{	0x14,	"f4",			VK_F4,			VK_F4	   },
	{	0x74,	"f5",			VK_F5,			VK_F5	   },
	{	0x75,	"f6",			VK_F6,			VK_F6	   },
	{	0x16,	"f7",			VK_F7,			VK_F7	   },
	{	0x76,	"f8",			VK_F8,			VK_F8	   },
	{	0x77,	"f9",			VK_F9,			VK_F9	   },
	{	0x70,	"ESCAPE",		VK_ESCAPE,		VK_ESCAPE  },
	{	0x30,	"!1",			'1',			'!'		   },
	{	0x31,	"\"2",			'2',			'\"'	   },
	{	0x11,	"#3",			'3',			'#'		   },
	{	0x12,	"$4",			'4',			'$'		   },
	{	0x13,	"%5",			'5',			'%'		   },
	{	0x34,	"&6",			'6',			'&'		   },
	{	0x24,	"'7",			'7',			'\''	   },
	{	0x15,	"(8",			'8',			'('		   },
	{	0x26,	")9",			'9',			')'		   },
	{	0x27,	"Ø",			'0',			'0'		   },
	{	0x17,	"=-",			'-',			'='		   },
	{	0x18,	"~^",			'^',			'~'		   },
	{	0x78,	"|\\",			'\\',			'|'		   },
	{	0x19,	"LEFT",			VK_LEFT,		VK_LEFT	   },
	{	0x79,	"RIGHT",		VK_RIGHT,		VK_RIGHT   },
	{	0x60,	"TAB",			VK_TAB,			VK_TAB	   },
	{	0x10,	"Q",			'Q',			'q'		   },
	{	0x21,	"W",			'W',			'w'		   },
	{	0x22,	"E",			'E',			'e'		   },
	{	0x33,	"R",			'R',			'r'		   },
	{	0x23,	"T",			'T',			't'		   },
	{	0x44,	"Y",			'Y',			'y'		   },
	{	0x35,	"U",			'U',			'u'		   },
	{	0x25,	"I",			'I',			'i'		   },
	{	0x36,	"O",			'O',			'o'		   },
	{	0x37,	"P",			'P',			'p'		   },
	{	0x47,	"@",			'@',			'@'		   },
	{	0x38,	"{[",			'[',			'{'		   },
	{	0x28,	"£-",			'-',			VK_F11	   }, // Note: no 1:1 mapping for '£' on a PC keyboard, so using F11 as a proxy
	{	0x39,	"UP",			VK_UP,			VK_UP	   },
	{	0x29,	"DOWN",			VK_DOWN,		VK_DOWN	   },
	{	0x40,	"CAPS LOCK",	VK_CAPITAL,		VK_CAPITAL },
	{	0x01,	"CTRL",			VK_CONTROL,		VK_CONTROL },
	{	0x41,	"A",			'A',			'a'		   },
	{	0x51,	"S",			'S',			's'		   },
	{	0x32,	"D",			'D',			'd'		   },
	{	0x43,	"F",			'F',			'f'		   },
	{	0x53,	"G",			'G',			'g'		   },
	{	0x54,	"H",			'H',			'h'		   },
	{	0x45,	"J",			'J',			'j'		   },
	{	0x46,	"K",			'K',			'k'		   },
	{	0x56,	"L",			'L',			'l'		   },
	{	0x57,	"+;",			';',			'+'		   },
	{	0x48,	"*:",			':',			'*'		   },
	{	0x58,	"}]",			']',			'}'		   },
	{	0x49,	"RETURN",		VK_RETURN,		VK_RETURN  },
	{	0x50,	"SHIFT LOCK",	VK_RSHIFT,		VK_RSHIFT  }, // Note: no 1:1 mapping on a PC keyboard
	{	0x00,	"SHIFT",		VK_LSHIFT,		VK_LSHIFT  },
	{	0x61,	"Z",			'Z',			'z'		   },
	{	0x42,	"X",			'X',			'x'		   },
	{	0x52,	"C",			'C',			'c'		   },
	{	0x63,	"V",			'V',			'v'		   },
	{	0x64,	"B",			'B',			'b'		   },
	{	0x55,	"N",			'N',			'n'		   },
	{	0x65,	"M",			'M',			'm'		   },
	{	0x66,	"<,",			',',			'<'		   },
	{	0x67,	">.",			'.',			'>'		   },
	{	0x68,	"?/",			'/',			'?'		   },
	{	0x59,	"DELETE",		VK_DELETE,		VK_DELETE  },
	{	0x69,	"COPY",			VK_MENU,		VK_MENU	   },	// Note: no 1:1 mapping on a PC keyboard
	{	0x62,	"SPACE",		VK_SPACE,		VK_SPACE   },
	{	0x02,	"BIT7",			7,				7		   },   // Note: These are not really valid as there is no mapping to keyboard - just exposed because Bits 0-7 are included on KB map table
	{	0x03,	"BIT6",			6,				7		   },   // Note: These are not really valid as there is no mapping to keyboard - just exposed because Bits 0-7 are included on KB map table
	{	0x04,	"BIT5",			5,				7		   },   // Note: These are not really valid as there is no mapping to keyboard - just exposed because Bits 0-7 are included on KB map table
	{	0x05,	"BIT4",			4,				7		   },   // Note: These are not really valid as there is no mapping to keyboard - just exposed because Bits 0-7 are included on KB map table
	{	0x06,	"BIT3",			3,				7		   },   // Note: These are not really valid as there is no mapping to keyboard - just exposed because Bits 0-7 are included on KB map table
	{	0x07,	"BIT2",			2,				7		   },   // Note: These are not really valid as there is no mapping to keyboard - just exposed because Bits 0-7 are included on KB map table
	{	0x08,	"BIT1",			1,				7		   },   // Note: These are not really valid as there is no mapping to keyboard - just exposed because Bits 0-7 are included on KB map table
	{	0x09,	"BIT0",			0,				7		   },   // Note: These are not really valid as there is no mapping to keyboard - just exposed because Bits 0-7 are included on KB map table
};

//-------------------------------------------------------------------------------------------------

BBC_Keyboard::BBC_Keyboard( System_VIA_6522& sysVia )
	: m_sysVia( sysVia )
{
	memset( m_vkeyToScanCodeMap, -1, sizeof( m_vkeyToScanCodeMap ) );
	memset( m_scancodeToKeyCodeMap, -1, sizeof( m_scancodeToKeyCodeMap ) );
	memset( m_keys, 0, sizeof( m_keys ) );

	for ( int i =0 ; i < sizeof(g_keycodeMapping)/sizeof(KeyCodeMapping); i++ )
	{
		const KeyCodeMapping& mapping = g_keycodeMapping[ i ];

		m_vkeyToScanCodeMap[ mapping.vkey		] = mapping.scanCode;
		m_vkeyToScanCodeMap[ mapping.vkey_shift ] = mapping.scanCode;
		m_scancodeToKeyCodeMap[ mapping.scanCode ] = mapping.vkey;
	}
}

//-------------------------------------------------------------------------------------------------

void BBC_Keyboard::SetKeyDown( u8 vkey )
{
	m_keys[ vkey ] = 1;
	m_sysVia.ScanKeyboard();
}

//-------------------------------------------------------------------------------------------------

void BBC_Keyboard::SetKeyUp( u8 vkey )
{
	m_keys[ vkey ] = 0;
	m_sysVia.ScanKeyboard();
}

//-------------------------------------------------------------------------------------------------

bool BBC_Keyboard::IsKeyDown_ScanCode( u8 scancode )
{
	return m_keys[ m_scancodeToKeyCodeMap[ scancode ] ] != 0;
}

//-------------------------------------------------------------------------------------------------

