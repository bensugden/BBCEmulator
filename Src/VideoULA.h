#pragma once

//-------------------------------------------------------------------------------------------------

class VideoULA
{
public:
	VideoULA( class SAA5050& teletextChip, class CRTC_6845& crtcChip, class System_VIA_6522& sysVIA );

	void		RefreshDisplay();
	void		SetHardwareScrollScreenOffset( u32 offset );
	void		NotifyRegisterWrite( u8 reg, u8 value, bool bIsCRTCRegister );
	bool		Tick( int nCycles );

	//-------------------------------------------------------------------------------------------------

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

	
	//-------------------------------------------------------------------------------------------------

	struct RegisterWriteOp
	{
		void	Commit();
		bool	m_bIsCRTCRegister;
		u8		m_value;
		u8		m_register;
		u64		m_cpuClockTick;
	};

	//-------------------------------------------------------------------------------------------------

	u8			WRITE_Serial_ULA_Control_register( u16 address, u8 value );
	u8			WRITE_Video_ULA_Control_register( u16 address, u8 value );
	u8			WRITE_Video_ULA_Palette_register( u16 address, u8 value );

	void		RenderScreen();
	void		SetControlRegister( u8 value );
	void		SetPaletteRegister( u8 value );
	//-------------------------------------------------------------------------------------------------

	u8								m_colorLookup[ 4 ][ 8 ][ 256 ];
	u8								m_logicalToPhyscialColor[ 16 ];
	u32								m_hardwareScrollOffset = 0 ;
	SAA5050&						m_teletext;
	CRTC_6845&						m_CRTC;
	System_VIA_6522&				m_sysVIA;
	ULAState						m_ulaState;
	u64								m_clock;
	int								m_nRegisterChangeIndex; 
	std::vector< RegisterWriteOp >	m_registerOpsThisFrame;
};

//-------------------------------------------------------------------------------------------------
