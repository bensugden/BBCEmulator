//-------------------------------------------------------------------------------------------------
//
// 6845 CRTC Video Controller
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------
//
// Status Register - [Address 0 - Read]
//
//-------------------------------------------------------------------------------------------------
/*
 7      UR - Update ready (Rockwell R6545 only)
          0     Register 31 has been either read or written by the CPU
          1     an update strobe has occured
 6      LRF - LPEN Register Full 
          0     Register 16 or 17 has been read by the CPU
          1     LPEN strobe has occured
 5      VRT - Vertical retrace 
          0     Scan is currently not in the vertical blanking position
          1     [MOS6545] Scan is currently in its vertical blanking time 
          1     [R6545] Scan is currently in its vertical re-trace time.
                   Note: this bit goes to a 1 one when vertical re-trace starts.
                   It goes to a 0 five character clock times before vertical
                   re-trace ends to ensure that critical timings for refresh
                   RAM operations are met.
*/

//-------------------------------------------------------------------------------------------------

u8 CRTC_6845::WriteRegisterFile( u16 address, u8 value )
{
	if ( address == SHEILA::WRITE_6845_CRTC_Address_register )
	{
		assert( value < 18 );
		m_nCurrentRegister = value;
		//if ( value == 12 )
		//	cpu.ThrowBreakpoint(std::string("hit set screen"));
	}
	else
	//if ( address == SHEILA::WRITE_6845_CRTC_Register_file )
	{
		//
		// Must have passed register before file
		//
		assert( m_nCurrentRegister != -1 );

		r[ m_nCurrentRegister ] = value;
		//
		// Don't write this again
		//
		m_nCurrentRegister = -1;
	}
	return value;
}

//-------------------------------------------------------------------------------------------------
CRTC_6845::CRTC_6845()
{
	m_nCurrentRegister = -1;
	memset( r, 0, sizeof( r ) );
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6845_CRTC_Address_register, MemoryMapHandler( CRTC_6845::WriteRegisterFile ) );
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6845_CRTC_Register_file,    MemoryMapHandler( CRTC_6845::WriteRegisterFile ) );
}

//-------------------------------------------------------------------------------------------------

void CRTC_6845::Tick()
{
}

//-------------------------------------------------------------------------------------------------
