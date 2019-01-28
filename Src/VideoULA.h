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


	ULAState	m_ulaState;
};

//-------------------------------------------------------------------------------------------------
