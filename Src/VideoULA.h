#pragma once

//-------------------------------------------------------------------------------------------------

class VideoULA
{
public:
	VideoULA( class SAA5050& teletextChip, class CRTC_6845& crtcChip );
	void RefreshDisplay();
	void SetHardwareScrollScreenOffset( u32 offset );
private:
	struct ULAState
	{
		u8		nCtrlRegister;
		u8		nSelectedFlashColor;
		bool	bTeletextMode;
		u8		nCharactersPerLine;
		bool	bHighFrequencyClock;
		u8		nWidthOfCursorInBytes;
		bool	bLargeCursor;
		bool	bHideCursor ;
	};

	u8	WRITE_Serial_ULA_Control_register( u16 address, u8 value );
	u8	WRITE_Video_ULA_Control_register( u16 address, u8 value );
	u8	WRITE_Video_ULA_Palette_register( u16 address, u8 value );

	void RenderScreen();

	u8 m_colorLookup[ 4 ][ 8 ][ 256 ];
	u8 m_logicalToPhyscialColor[ 16 ];
	u32	m_hardwareScrollOffset = 0 ;
	SAA5050& m_teletext;
	CRTC_6845& m_CRTC;
	ULAState m_ulaState;
};

//-------------------------------------------------------------------------------------------------
