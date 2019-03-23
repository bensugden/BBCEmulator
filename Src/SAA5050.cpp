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

static u32 s_colorLookup[]=
{
	0xFF000000,
	0xFF0000FF,
	0xFF00FF00,
	0xFF00FFFF,
	0xFFFF0000,
	0xFFFF00FF,
	0xFFFFFF00,
	0xFFFFFFFF,
};

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
	bool bFlashOffThisFrame = false; // todo
	
	for ( int y = 0 ; y < nScreenHeight; y++ )
	{
		int lastChar				= 0x20;
		int heldChar				= 0x20;
		u32 uForegroundColorMask    = s_colorLookup[ 7 ]; // white
		u32 uBackgroundColorMask	= s_colorLookup[ 0 ]; // black
		u32 uCurrentColorMask		= uForegroundColorMask;
		bool bDoubleHeight			= false;
		bool bConceal				= false;
		int nFontOffset				= 0;
		u32* pFrameBufferPtr0		= fbInfo.m_pData + y * fbInfo.m_pitch * nCharHeightPlusSpace;

		for ( int x = 0 ; x < nScreenWidth; x++ )
		{
			int displayChar = mem.Read_Internal( nCurrentAddress++ ) & 0x7f;

			//
			// Process control code
			//
			if ( displayChar < 0x20 )
			{
				switch ( displayChar )
				{
					case 0x00:
					case 0x01:
					case 0x02:
					case 0x03:
					case 0x04:
					case 0x05:
					case 0x06:
					case 0x07:
					{
						// alpha color
						uCurrentColorMask = uForegroundColorMask = s_colorLookup[ displayChar ];
						nFontOffset = 0 ;
						bConceal = false;
						break;
					}
					case 0x08:
					{
						// flash
						if ( bFlashOffThisFrame )
						{
							uCurrentColorMask = uBackgroundColorMask;
						}
					}
					case 0x09:
					{
						// steady
						uCurrentColorMask = uForegroundColorMask;
					}
					case 0x0a:
					{
						// end box ??
						break;
					}
					case 0x0b:
					{
						// start box??
						break;
					}
					case 0x0c:
					{
						// normal height
						bDoubleHeight = false;
						break;
					}
					case 0x0d:
					{
						// double height
						bDoubleHeight = true;
						break;
					}
					case 0x0e:
					{
						// S0?
						break;
					}
					case 0x0f:
					{
						// S1?
						break;
					}
					case 0x10:
					case 0x11:
					case 0x12:
					case 0x13:
					case 0x14:
					case 0x15:
					case 0x16:
					case 0x17:
					{
						// set graphics & color
						uCurrentColorMask = s_colorLookup[ displayChar & 0x7 ];
						nFontOffset = 96 ;
						bConceal = false;
						break;
					}
					case 0x18:
					{
						// conceal
						bConceal = true;
						break;
					}
					case 0x19:
					{
						// contiguous graphics
						nFontOffset = 96;
						break;
					}
					case 0x1a:
					{
						// separated graphics
						nFontOffset = 96 *2 ;
						break;
					}
					case 0x1b:
					{
						// black background
						uBackgroundColorMask = s_colorLookup[ 0 ];
						break;
					}
					case 0x1c:
					{
						// esc ??
						break;
					}
					case 0x1d:
					{
						// 	new background
						uBackgroundColorMask = uForegroundColorMask;
						break;
					}
					case 0x1e:
					{
						// hold graphics
						heldChar = lastChar;
						break;
					}
					case 0x1f:
					{
						// release graphics
						heldChar = 0x20;
						break;
					}
				}

				displayChar = heldChar;				
			}

			if ( bConceal )
			{
				displayChar = 0x20;
			}
			else
			{
				displayChar += nFontOffset;
			}

			//
			// Do wrap around for vertical scroll
			//
			if ( nCurrentAddress >= nRomAddress )
			{
				nCurrentAddress -= nScreenSizeInBytes;
			}

			//
			// Render Character
			//
			u32* pFrameBufferPtr1 = pFrameBufferPtr0 + x * nCharWidthPlusSpace;
			
			u32* lookupChar = m_scaledFonts + ( displayChar - 0x20 ) * 60 * 4;

			if (bDoubleHeight && (( y != 0 )&&( mem.Read_Internal( nCurrentAddress-nScreenWidth-1 ) == displayChar )))
			{
				lookupChar += 30 * 4;
			}
			for ( int dy = 0; dy < nCharHeight; dy++ )
			{
				for ( int dx = 0; dx < nCharWidth; dx++ )
				{
					pFrameBufferPtr1[ dx ] = ( lookupChar[dx] & uCurrentColorMask ) | uBackgroundColorMask;
				}
				if ((!bDoubleHeight)||(dy&1))
				{
					lookupChar += nCharWidth;
				}
				pFrameBufferPtr1 += fbInfo.m_pitch;
			}
			lastChar = displayChar;
		}
	}

	GFXSystem::UnlockFrameBuffer();
	GFXSystem::SetAnisotropicFiltering( true );
}

//-------------------------------------------------------------------------------------------------