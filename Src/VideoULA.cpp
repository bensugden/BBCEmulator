//-------------------------------------------------------------------------------------------------
// 
// BBC Micro Video ULA
// 
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------

VideoULA::VideoULA()
{
}

//-------------------------------------------------------------------------------------------------

static u32 s_physialColorPalette[ 16 ][ 2 ] =
{
	//
	// ARGB 32 Bit Colors. 
	// 2 Entries - primary flash / secondary flash
	//
	{ 0xFF000000, 0xFF000000 },	// black
	{ 0xFFFF0000, 0xFFFF0000 }, // red
	{ 0xFF00FF00, 0xFF00FF00 }, // green
	{ 0xFFFFFF00, 0xFFFFFF00 }, // yellow
	{ 0xFF0000FF, 0xFF0000FF }, // blue
	{ 0xFFFF00FF, 0xFFFF00FF }, // magenta
	{ 0xFF00FFFF, 0xFF00FFFF }, // cyan
	{ 0xFFFFFFFF, 0xFFFFFFFF }, // white
	{ 0xFF000000, 0xFFFFFFFF }, // black, white
	{ 0xFFFF0000, 0xFF00FFFF }, // red, cyan
	{ 0xFF00FF00, 0xFFFF00FF }, // green, magenta
	{ 0xFFFFFF00, 0xFF0000FF }, // yellow, blue
	{ 0xFF0000FF, 0xFFFFFF00 }, // blue, yellow
	{ 0xFFFF00FF, 0xFF00FF00 }, // magenta, green
	{ 0xFF00FFFF, 0xFFFF0000 }, // cyan, red
	{ 0xFFFFFFFF, 0xFF000000 }, // white, black
};

//-------------------------------------------------------------------------------------------------

void VideoULA::RenderScreen()
{
	u8 ctrl_register = SHEILA::Read( SHEILA::WRITE_Video_ULA_Control_register );

	//
	// Parse bitfield
	//
	m_ulaState.nCtrlRegister		= ctrl_register;
	m_ulaState.nSelectedFlashColor	= ctrl_register & 1;
	m_ulaState.bTeletextMode		= ( ctrl_register & 2 ) == 2;
	m_ulaState.nCharactersPerLine	= ( 1 << ( ( ctrl_register >> 2 ) & 3 ) ) * 10; // [2bits] 0,1,2,3 -> 10,20,40,80
	m_ulaState.bHighFrequencyClock	= ( ctrl_register & 0x10 ) == 0x10;
		
	m_ulaState.nWidthOfCursorInBytes = ( ( ctrl_register >> 5 ) & 3 ) + 1 ;
	if ( m_ulaState.nWidthOfCursorInBytes == 3 )
	{
		m_ulaState.nWidthOfCursorInBytes = 2;									// [2bits] 0,1,2,3 -> 1,*,2,4 (there's probably a nicer arithmetic way to calculate this)
	}

	m_ulaState.bLargeCursor			= ( ctrl_register & 0x80 ) == 0x80;
	m_ulaState.bHideCursor			= ( ctrl_register & 0xe1 ) == 0xe1;
		
	//-------------------------------------------------------------------------------------------------
	//
	// Official Modes:
	//
	//-------------------------------------------------------------------------------------------------
	// MODE	 Control register
	//-------------------------------------------------------------------------------------------------
	//	0	&9C (%100 1 11 0 0)
	//	1	&D8 (%110 1 10 0 0)
	//	2	&F4 (%111 1 01 0 0)
	//	3	&9C (%100 1 11 0 0)
	//	4	&88 (%100 0 10 0 0)
	//	5	&C4 (%110 0 01 0 0)
	//	6	&88 (%100 0 10 0 0)
	//	7	&4B (%010 0 10 1 1)
		
	//-------------------------------------------------------------------------------------------------
	//
	// Unofficial "extra" Modes
	//
	//-------------------------------------------------------------------------------------------------
	// MODE	  Control register
	//-------------------------------------------------------------------------------------------------
	//	8	&E0 (%111 0 00 0 0)
	//	9	&80 (%100 0 00 0 0)
	//	10	&84 (%100 0 01 0 0)
}

//-------------------------------------------------------------------------------------------------
