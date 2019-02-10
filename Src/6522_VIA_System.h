#pragma once

//-------------------------------------------------------------------------------------------------
//
// 6522 VIA System MOS Chip
//
//-------------------------------------------------------------------------------------------------

#include "6522_VIA.h"

class IKeyboard
{
public:
	virtual bool IsKeyDown_ScanCode( u8 scancode ) = 0 ;
};

//-------------------------------------------------------------------------------------------------

class System_VIA_6522 : public VIA_6522
{
public:
	System_VIA_6522( IKeyboard& keyboard );

	u8				WriteORA( u16 address, u8 value );
	u8				WriteORB( u16 address, u8 value );
	u8				ReadIRA( u16 address, u8 value );
private:
	void			UpdateSlowDataBus();
	void			WriteToIC32( u8 value );
	void			ScanKeyboard();

	u8				m_nIC32;
	IKeyboard&		m_keyboard;
	u8				m_nSDB;
};

//-------------------------------------------------------------------------------------------------

