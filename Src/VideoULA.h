#pragma once

//-------------------------------------------------------------------------------------------------

class VideoULA
{
public:
	VideoULA();
	void RenderScreen();

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

	static void	WRITE_Serial_ULA_Control_register( u16 address, u8 value );
	static void	WRITE_Video_ULA_Control_register( u16 address, u8 value );
	static void	WRITE_Video_ULA_Palette_register( u16 address, u8 value );
	
	ULAState	m_ulaState;
};

//-------------------------------------------------------------------------------------------------
