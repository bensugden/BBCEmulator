#pragma once

//-------------------------------------------------------------------------------------------------
//
// BBC Keyboard
//
//-------------------------------------------------------------------------------------------------

class BBC_Keyboard : public IKeyboard
{
public:
	BBC_Keyboard();

	void SetKeyDown( u8 vkey );
	void SetKeyUp( u8 vkey );
	virtual bool IsKeyDown_ScanCode( u8 scancode );

private:
	u8 m_getCurrentScanCode;
	u8 m_vkeyToScanCodeMap[ 256 ];
	u8 m_scancodeToKeyCodeMap[ 256 ];
	u8 m_keys[ 256 ];
};

//-------------------------------------------------------------------------------------------------

