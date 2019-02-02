//-------------------------------------------------------------------------------------------------
//
// Teletext Character Generator
//
// Much left to do here - graphics, colors and interlacing
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "TeletextFont.h"

//-------------------------------------------------------------------------------------------------

SAA5050::SAA5050( const CRTC_6845& CRTC )
	: m_CRTC( CRTC )
{
}

//-------------------------------------------------------------------------------------------------

void SAA5050::RenderScreen()
{
	u8 hi = m_CRTC.GetRegisterValue( CRTC_6845::Display_start_address_high );
	u8 lo = m_CRTC.GetRegisterValue( CRTC_6845::Display_start_address_low );
	//
	// Do screen address hi byte calculations ( see chpt 18.11.3 in Adv user guide )
	//
	hi = ( hi ^ 0x20 ) + 0x74;

	u16 uStartAddress = MakeAddress( hi, lo );

	u16 nScreenWidth = m_CRTC.GetRegisterValue( CRTC_6845::Horizontal_displayed_character_lines );
	u16 nScreenHeight = m_CRTC.GetRegisterValue( CRTC_6845::Vertical_displayed_character_lines );

	GFXSystem::FrameBufferInfo fbInfo;

	if ( nScreenWidth * nScreenHeight == 0 )
		return;

	static const int nCharWidth = 6;
	static const int nCharHeight = 10;
	static const int nCharWidthPlusSpace = nCharWidth;
	static const int nCharHeightPlusSpace = nCharHeight;

	int nPixelWidth = nScreenWidth * nCharWidthPlusSpace;
	int nPixelHeight = nScreenHeight * nCharHeightPlusSpace;

	fbInfo = GFXSystem::LockFrameBuffer( nPixelWidth, nPixelHeight );
	
	//
	// Render characters to frame buffer
	//
	u16 nAddress = uStartAddress;

	for ( int y = 0 ; y < nScreenHeight; y++ )
	{
		u32* pFB0 = fbInfo.m_pData + y * fbInfo.m_pitch * nCharHeightPlusSpace;

		for ( int x = 0 ; x < nScreenWidth; x++ )
		{
			u8 displayChar = mem.Read_Internal( nAddress++ );

			u32* pFB1 = pFB0 + x * nCharWidthPlusSpace;

			if ( displayChar < 0x20 || ( displayChar >= ( 0x20 + 96 ) ) )
				continue;

			u8* lookupChar = s_teletextCharacters + ( displayChar - 0x20 ) * 60;
			for ( int dy = 0; dy < nCharHeight; dy++ )
			{
				for ( int dx = 0; dx < nCharWidth; dx++ )
				{
					if ( *lookupChar++ == 0 )
					{
						pFB1[ dx ] = 0;
					}
					else
					{
						pFB1[ dx ] = 0xffffffff;
					}
				}
				pFB1 += fbInfo.m_pitch;
			}
		}
	}

	GFXSystem::UnlockFrameBuffer();
}

//-------------------------------------------------------------------------------------------------