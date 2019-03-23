//-------------------------------------------------------------------------------------------------
// 
// BBC Micro Video ULA
// 
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------

static u32 s_physialColorPalette[ 16 ][ 2 ] =
{
	//
	// ARGB 32 Bit Colors. 
	// 2 Entries - primary flash / secondary flash
	//
	{ 0xFF000000, 0xFF000000 },	// black
	{ 0xFF0000FF, 0xFF0000FF }, // red
	{ 0xFF00FF00, 0xFF00FF00 }, // green
	{ 0xFF00FFFF, 0xFF00FFFF }, // yellow
	{ 0xFFFF0000, 0xFFFF0000 }, // blue
	{ 0xFFFF00FF, 0xFFFF00FF }, // magenta
	{ 0xFFFFFF00, 0xFFFFFF00 }, // cyan
	{ 0xFFFFFFFF, 0xFFFFFFFF }, // white
	{ 0xFF000000, 0xFFFFFFFF }, // black, white
	{ 0xFF0000FF, 0xFFFFFF00 }, // red, cyan
	{ 0xFF00FF00, 0xFFFF00FF }, // green, magenta
	{ 0xFF00FFFF, 0xFFFF0000 }, // yellow, blue
	{ 0xFFFF0000, 0xFF00FFFF }, // blue, yellow
	{ 0xFFFF00FF, 0xFF00FF00 }, // magenta, green
	{ 0xFFFFFF00, 0xFF0000FF }, // cyan, red
	{ 0xFFFFFFFF, 0xFF000000 }, // white, black
};

//-------------------------------------------------------------------------------------------------

VideoULA::VideoULA( SAA5050& teletextChip, CRTC_6845& crtcChip )
	: m_teletext( teletextChip )
	, m_CRTC( crtcChip )
{
	mem.RegisterMemoryMap_Write( SHEILA::WRITE_Serial_ULA_Control_register, MemoryMapHandler( VideoULA::WRITE_Serial_ULA_Control_register ) );
	mem.RegisterMemoryMap_Write( SHEILA::WRITE_Video_ULA_Control_register,  MemoryMapHandler( VideoULA::WRITE_Video_ULA_Control_register  ) );
	mem.RegisterMemoryMap_Write( SHEILA::WRITE_Video_ULA_Palette_register,  MemoryMapHandler( VideoULA::WRITE_Video_ULA_Palette_register  ) );

	for ( int n = 0; n < 4; n++ )
	{
		int nBitsPerPixel = 1 << n;
		for ( int i = 0; i < 256; i++ )
		{
			int nBitSkip = 8 / nBitsPerPixel;

			for ( int o = 0; o < nBitSkip; o++ )
			{
				int nStartBit = 7 - o;

				int nAccum = 0;
				for ( int x = 0; x < nBitsPerPixel; x++ )
				{
					nAccum = ( nAccum << 1 ) + (( i >> nStartBit ) & 1 );
					nStartBit -= nBitSkip;
				}
				m_colorLookup[ n ][ o ][ i ] = nAccum;
			}
		}
	}	
	for ( int i = 0; i < 16; i++ )
	{
		m_logicalToPhyscialColor[ i ] = i;
	}

	m_hardwareScrollOffset = 0;
	m_clock = 0;
	m_registerOpsThisFrame.resize(0);
	m_nRegisterChangeIndex = 0;
}

//-------------------------------------------------------------------------------------------------

void VideoULA::RefreshDisplay()
{
	if ( m_ulaState.bTeletextMode )
	{
		Tick( (int)(cpu.GetClockCounter() - m_clock) );
		m_teletext.RenderScreen();
		return;
	}
	//
	// Scan screen and render pixels, servicing stored interrups along the way
	//
	RenderScreen();
	m_registerOpsThisFrame.resize(0);
	m_nRegisterChangeIndex = 0;
}

//-------------------------------------------------------------------------------------------------

void VideoULA::NotifyRegisterWrite( u8 reg, u8 value, bool bIsCRTCRegister )
{
	RegisterWriteOp op;

	op.m_bIsCRTCRegister	= bIsCRTCRegister;
	op.m_value				= value;
	op.m_register			= reg;
	op.m_cpuClockTick		= cpu.GetClockCounter();

	m_registerOpsThisFrame.push_back( op );
}

//-------------------------------------------------------------------------------------------------

void VideoULA::SetHardwareScrollScreenOffset( u32 offset )
{
	m_hardwareScrollOffset = offset ; // NOTE: not using this ATM- need to work out how/when it's used
}

//-------------------------------------------------------------------------------------------------

u8 VideoULA::WRITE_Serial_ULA_Control_register( u16 address, u8 value )
{
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VideoULA::WRITE_Video_ULA_Control_register( u16 address, u8 ctrl_register )
{
	assert( address == SHEILA::WRITE_Video_ULA_Control_register );

	NotifyRegisterWrite( 0, ctrl_register, false );
	SetControlRegister( ctrl_register );
	return ctrl_register;
}

//-------------------------------------------------------------------------------------------------

void VideoULA::SetControlRegister( u8 ctrl_register )
{ 
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

	for ( int i = 0; i < 16; i++ )
	{
		m_logicalToPhyscialColor[ i ] = i;
	}
}

//-------------------------------------------------------------------------------------------------

void VideoULA::SetPaletteRegister( u8 value )
{ 
	u16 nScreenWidthInChars = m_CRTC.GetRegisterValue( CRTC_6845::Horizontal_displayed_character_lines );
	u16 bitsPerPixel = nScreenWidthInChars / m_ulaState.nCharactersPerLine;

	switch( bitsPerPixel )
	{
		case 1:
			m_logicalToPhyscialColor[ ( ( value & 0x80 ) >> 7 ) ] = 7 ^ ( value & 15 );
		break;
		case 2:
			m_logicalToPhyscialColor[ ( ( value & 0x80 ) >> 6 ) + ( ( value & 0x20 ) >> 5 ) ] = 7 ^ ( value & 15 );
		break;
		default:
			m_logicalToPhyscialColor[ value >> 4 ] = 7 ^ ( value & 15 );
		break;
	}
}

//-------------------------------------------------------------------------------------------------

u8 VideoULA::WRITE_Video_ULA_Palette_register( u16 address, u8 value )
{
	NotifyRegisterWrite( 1, value, false );
	
	//SetPaletteRegister( value );
	return value;
}

//-------------------------------------------------------------------------------------------------

bool VideoULA::Tick( int nCycles )
{
	m_clock += nCycles;

	if ( m_nRegisterChangeIndex == ((int)m_registerOpsThisFrame.size()) )
		return false;

	bool bUpdated;

	while ( m_clock > m_registerOpsThisFrame[ m_nRegisterChangeIndex ].m_cpuClockTick )
	{
		bUpdated = true;

		RegisterWriteOp op = m_registerOpsThisFrame[ m_nRegisterChangeIndex++ ];
		if ( op.m_bIsCRTCRegister )
		{
			m_CRTC.SetRegister( op.m_register, op.m_value );
		}
		else
		{ 
			if ( op.m_register == 0 )
			{
				SetControlRegister( op.m_value );
			}
			else
			{
				SetPaletteRegister( op.m_value );
			}
		}
		if ( m_nRegisterChangeIndex == ((int)m_registerOpsThisFrame.size()) )
		{
			break;
		}
	}
	return bUpdated;
}

//-------------------------------------------------------------------------------------------------

void VideoULA::RenderScreen( )
{
	u8 hi = m_CRTC.GetRegisterValue( CRTC_6845::Display_start_address_high );
	u8 lo = m_CRTC.GetRegisterValue( CRTC_6845::Display_start_address_low );

	u16 uStartAddress = MakeAddress( hi, lo ) * 8;
	u16 nRomAddress = 0x8000;

	static const int c_nFBWidth = 640;
	static const int c_nFBHeight = 256;

	GFXSystem::FrameBufferInfo fbInfo;
	fbInfo = GFXSystem::LockFrameBuffer( c_nFBWidth, c_nFBHeight );
	u16 nCurrentAddress = uStartAddress; 
	
	//
	// Setup values from previous video register states
	//
	u16 nScreenHeightInChars = m_CRTC.GetRegisterValue( CRTC_6845::Vertical_displayed_character_lines );
	u16 nPixelHeight = nScreenHeightInChars * 8;

	u16 nScreenWidthInChars = m_CRTC.GetRegisterValue( CRTC_6845::Horizontal_displayed_character_lines );
	u16 bitsPerPixel = nScreenWidthInChars / m_ulaState.nCharactersPerLine;
	if ( bitsPerPixel == 0 )
		bitsPerPixel = 1;
	u16 pixelsPerByte = 8 / bitsPerPixel;
	u16 nPixelWidth = m_ulaState.nCharactersPerLine * 8;

	u16 nScreenSizeInBytes = nPixelWidth * nPixelHeight * bitsPerPixel / 8;
	u8 nHorizPixDup = c_nFBWidth / nPixelWidth;

	int table = 0;
	while ( ( 1 << table ) != bitsPerPixel )
	{
		table++;
	}

	int nTicks = 0;
	for ( int y = 0 ; y < nPixelHeight; y += 8 )
	{
		for ( int x = 0 ; x < nPixelWidth; x+= pixelsPerByte )
		{
			//
			// Poll video registers & commit changes if they were submitted during the elapsed cycles
			//
			if ( Tick( nTicks ) )
			{
				//
				// Need to refresh pixel sizes etc.
				//
				nScreenWidthInChars = m_CRTC.GetRegisterValue( CRTC_6845::Horizontal_displayed_character_lines );
				bitsPerPixel = nScreenWidthInChars / m_ulaState.nCharactersPerLine;
				if ( bitsPerPixel == 0 )
					bitsPerPixel = 1;
				pixelsPerByte = 8 / bitsPerPixel;
				nPixelWidth = m_ulaState.nCharactersPerLine * 8;
				nHorizPixDup = c_nFBWidth / nPixelWidth;
				table = 0;
				while ( ( 1 << table ) != bitsPerPixel )
				{
					table++;
				}
			}
			
			//
			// Render Char
			//
			for ( int dy = 0 ; dy < 8; dy++ )
			{
				u32* pFB0 = fbInfo.m_pData + x * nHorizPixDup + ( y + dy ) * fbInfo.m_pitch;

				u8 value = mem.Read_Internal( nCurrentAddress++ );

				//
				// Do wrap around for vertical scroll
				//
				if ( nCurrentAddress >= nRomAddress )
				{
					nCurrentAddress -= nScreenSizeInBytes;
				}
				for ( int pixel = 0 ; pixel < pixelsPerByte; pixel++ )
				{
					u8 logicalIndex = m_colorLookup[ table ][ pixel ][ value ];
					u8 physicalIndex = m_logicalToPhyscialColor[ logicalIndex ];
					for ( int d = 0; d< nHorizPixDup; d++ )
					{
						*pFB0++ = s_physialColorPalette[ physicalIndex ][ 0 ];
					}
				}
			}
			nTicks = 16;
		}
	}
	Tick( int( cpu.GetClockCounter() - m_clock ) );

	GFXSystem::UnlockFrameBuffer();
	GFXSystem::SetAnisotropicFiltering( false );
}

//-------------------------------------------------------------------------------------------------
