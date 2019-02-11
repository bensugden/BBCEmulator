//-------------------------------------------------------------------------------------------------
//
// 6522 VIA System MOS Chip
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------

struct IC32
{
	u8 m_nState;

};
//-------------------------------------------------------------------------------------------------

System_VIA_6522::System_VIA_6522( IKeyboard& keyboard )
	: VIA_6522( SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B  )
	, m_keyboard( keyboard )
{
	m_nIC32 = 0;
	m_nSDB = 0;
}

//-------------------------------------------------------------------------------------------------

void System_VIA_6522::WriteToIC32( u8 value )
{
	//
	// Note: Bit 3 (0x80) controls autoscan of keyboard
	//
	u8 ic32_bit = 1 << ( value & 0x7 );
	u8 i32_value = ( value & 0x8 );
	if ( i32_value )
	{
		m_nIC32 |= ic32_bit; 
	}
	else
	{
		m_nIC32 &= ~ic32_bit;
	}

	//
	// handle keyboard
	//
	//
	// TODO
	//
	// check bit 0 - if set do sound
	// check bit 1/2 for speech
	// check bits 6/7 - controls caps lock LED / shift lock LEDs
	//

	//
	// TODO
	//
	// use bits 4/5 for screen offset with modes 0-6 [ not referenced ]
	u16 screenOffsets[] = { 16 * 1024, 24 * 1024, 22 * 1024, 12 * 1024 };
	u8 offset = ( m_nIC32 >> 4 ) & 3;
}
//-------------------------------------------------------------------------------------------------
/*

TODO
	on write to A : get sbdval
        keyrow = (sdbval >> 4) & 7;
        keycol = sdbval & 0xF;
        key_update();
        if (!(IC32 & 8) && !bbckey[keycol][keyrow]) sdbval &= 0x7f;
	on write to B : update ic32
		*/

//-------------------------------------------------------------------------------------------------

void System_VIA_6522::ScanKeyboard()
{
	if ( m_nIC32 & 0x8 )
	{
		//
		// Autoscan enabled
		//
		for (int col = 0; col < 15; col++)
		{
			for (int row = 0; row < 15; row++)
			{
				if ( m_keyboard.IsKeyDown_ScanCode(  ( row << 4 ) | col ) )
				{
					SetCA2( 1 );
					return;
				}
			}
		}
	}
	else
	{
		//
		// No autoscan?
		//
        int col = m_nSDB & 0xF;
		for (int row = 0; row < 15; row++)
		{
			if ( m_keyboard.IsKeyDown_ScanCode(  ( row << 4 ) | col ) )
			{
				SetCA2( 1 );
				return;
			}
		}
	}
	SetCA2( 0 );
}

//-------------------------------------------------------------------------------------------------

void System_VIA_6522::UpdateSlowDataBus()
{
	ScanKeyboard();
	if (!(m_nIC32 & 8) && !m_keyboard.IsKeyDown_ScanCode( m_nSDB & 0x7f )) 
		m_nSDB &= 0x7f;
	/*
	if ( m_nIC32 & 0x8 )
	{
		if ( m_keyboard.IsKeyDown_ScanCode( m_nSDB & 0x7f ) )
		{
			m_nSDB |= 0x80;
		}
		else
		{
			m_nSDB &= 0x7f;
		}
	}
	*/
}

//-------------------------------------------------------------------------------------------------

u8 System_VIA_6522::ReadPortA( )
{
	UpdateSlowDataBus();
	return m_nSDB;
}

//-------------------------------------------------------------------------------------------------

void System_VIA_6522::WritePortA( u8 value )
{
	m_nSDB = value;
	UpdateSlowDataBus();
}

//-------------------------------------------------------------------------------------------------

void System_VIA_6522::WritePortB( u8 value )
{
	WriteToIC32( value );
	UpdateSlowDataBus();
}

//-------------------------------------------------------------------------------------------------
