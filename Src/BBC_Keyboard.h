#pragma once

//-------------------------------------------------------------------------------------------------
//
// BBC Keyboard
//
//-------------------------------------------------------------------------------------------------

class BBC_Keyboard : public IKeyboard
{
	//-------------------------------------------------------------------------------------------------
public:
	BBC_Keyboard( class System_VIA_6522& sysVia );

	//-------------------------------------------------------------------------------------------------

	void				SetKeyDown( u8 vkey, bool bIsVKey );
	void				SetKeyUp( u8 vkey, bool bIsVKey );
	virtual bool		IsKeyDown_ScanCode( u8 scancode );
	virtual bool		IsResetDown() { return m_bBreakPressed; }
	void				ClearKeys();

	//-------------------------------------------------------------------------------------------------

	struct KeyCodeMapping
	{
		u8		scanCode;  // in ROW:4, COLUMN:4 format
		string	name; 
		int		vkey;
		int		vkey_shift;
		u8		is_a_vkey;
	};

	void				ScanKeyboard( );

	//-------------------------------------------------------------------------------------------------

private:
	void				SetKeyState( u8 vkey, bool bIsVKey, u8 down );

	//-------------------------------------------------------------------------------------------------

	u8					m_getCurrentScanCode;
	KeyCodeMapping*		m_vkeyToScanCodeMap[ 256 ];
	KeyCodeMapping*		m_asciiToScanCodeMap[ 256 ];
	u8					m_keys[ 256 ];
	bool				m_bBreakPressed;
	System_VIA_6522&	m_sysVia;
};

//-------------------------------------------------------------------------------------------------

