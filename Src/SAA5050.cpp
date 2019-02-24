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
	//
	// Scale Font
	//
	m_scaledFonts = new u32[ 3 * 96 * 60  * 2 * 2 ];
	for ( int font = 0; font < 3; font++)
	{
		u32 uFontOffset = font * 96 * 60  * 2 * 2 ;
		for ( int character = 0; character < 96; character++ )
		{
			u32 uCharacterOffset = uFontOffset + character * 60 * 2 * 2 ;
			//
			// Scale up
			//
			for ( int y = 0; y < 10; y++ )
			{
				for ( int x = 0; x < 6; x++ )
				{
					u8 uCenter = s_teletextCharacters[ font ][ character * 60 + y * 6 + x ];
					u8 uN = 0;
					u8 uS = 0;
					u8 uW = 0;
					u8 uE = 0;
					u8 uSW = 0;
					u8 uSE = 0;
					u8 uNW = 0;
					u8 uNE = 0;

					if ( uCenter == 0 )
					{
						uS = y == 0 ? 0 : s_teletextCharacters[ font ][ character * 60 + ( y - 1 ) * 6 + x + 0 ];
						uN = y == 9 ? 0 : s_teletextCharacters[ font ][ character * 60 + ( y + 1 ) * 6 + x + 0 ];
						uW = x == 0 ? 0 : s_teletextCharacters[ font ][ character * 60 + ( y + 0 ) * 6 + x - 1 ];
						uE = x == 5 ? 0 : s_teletextCharacters[ font ][ character * 60 + ( y + 0 ) * 6 + x + 1 ];
						uSW= ( y == 0 || x == 0 ) ? 0 : s_teletextCharacters[ font ][ character * 60 + ( y - 1 ) * 6 + x - 1 ];
						uSE= ( y == 0 || x == 5 ) ? 0 : s_teletextCharacters[ font ][ character * 60 + ( y - 1 ) * 6 + x + 1 ];
						uNW= ( y == 9 || x == 0 ) ? 0 : s_teletextCharacters[ font ][ character * 60 + ( y + 1 ) * 6 + x - 1 ];
						uNE= ( y == 9 || x == 5 ) ? 0 : s_teletextCharacters[ font ][ character * 60 + ( y + 1 ) * 6 + x + 1 ];
					}
					
					m_scaledFonts[ uCharacterOffset + y * 24 + x * 2 +  0 ] = ( uCenter || ( ( uW == 1 ) && ( uS == 1 ) && ( uSW == 0 ) ) ) ? 0xffffffff : 0;
					m_scaledFonts[ uCharacterOffset + y * 24 + x * 2 +  1 ] = ( uCenter || ( ( uE == 1 ) && ( uS == 1 ) && ( uSE == 0 ) ) ) ? 0xffffffff : 0;
					m_scaledFonts[ uCharacterOffset + y * 24 + x * 2 + 12 ] = ( uCenter || ( ( uW == 1 ) && ( uN == 1 ) && ( uNW == 0 ) ) ) ? 0xffffffff : 0;
					m_scaledFonts[ uCharacterOffset + y * 24 + x * 2 + 13 ] = ( uCenter || ( ( uE == 1 ) && ( uN == 1 ) && ( uNE == 0 ) ) ) ? 0xffffffff : 0;
				}
			}
		}
	}
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
	u16 nBaseAddress = 0x7C00;
	u16 nRomAddress = 0x8000;
	u16 nScreenSizeInBytes = nRomAddress - nBaseAddress; 

	u16 nScreenWidth = m_CRTC.GetRegisterValue( CRTC_6845::Horizontal_displayed_character_lines );
	u16 nScreenHeight = m_CRTC.GetRegisterValue( CRTC_6845::Vertical_displayed_character_lines );

	GFXSystem::FrameBufferInfo fbInfo;

	if ( nScreenWidth * nScreenHeight == 0 )
		return;

	static const int nCharWidth = 12;
	static const int nCharHeight = 20;
	static const int nCharWidthPlusSpace = nCharWidth;
	static const int nCharHeightPlusSpace = nCharHeight;

	int nPixelWidth = nScreenWidth * nCharWidthPlusSpace;
	int nPixelHeight = nScreenHeight * nCharHeightPlusSpace;

	fbInfo = GFXSystem::LockFrameBuffer( nPixelWidth, nPixelHeight );
	
	//
	// Render characters to frame buffer
	//
	u16 nCurrentAddress = uStartAddress; 

	for ( int y = 0 ; y < nScreenHeight; y++ )
	{
		u32* pFB0 = fbInfo.m_pData + y * fbInfo.m_pitch * nCharHeightPlusSpace;

		for ( int x = 0 ; x < nScreenWidth; x++ )
		{
			u8 displayChar = mem.Read_Internal( nCurrentAddress++ );
			//
			// Do wrap around for vertical scroll
			//
			if ( nCurrentAddress >= nRomAddress )
			{
				nCurrentAddress -= nScreenSizeInBytes;
			}

			u32* pFB1 = pFB0 + x * nCharWidthPlusSpace;

			if ( displayChar < 0x20 || ( displayChar >= ( 0x20 + 96 ) ) )
				continue;

			u32* lookupChar = m_scaledFonts + ( displayChar - 0x20 ) * 60 * 4;
			for ( int dy = 0; dy < nCharHeight; dy++ )
			{
				for ( int dx = 0; dx < nCharWidth; dx++ )
				{
					*pFB1++ = *lookupChar++;
				}
				pFB1 += fbInfo.m_pitch - nCharWidth;
			}
		}
	}

	GFXSystem::UnlockFrameBuffer();
	GFXSystem::SetAnisotropicFiltering( true );
}

//-------------------------------------------------------------------------------------------------