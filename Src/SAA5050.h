//-------------------------------------------------------------------------------------------------
//
// Teletext Character Generator
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
class CRTC_6845;

class SAA5050
{
public:
	SAA5050( const CRTC_6845& CRTC );

	void RenderScreen();
	const CRTC_6845& m_CRTC ;
	u32* m_scaledFonts;
};

//-------------------------------------------------------------------------------------------------


