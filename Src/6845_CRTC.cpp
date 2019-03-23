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
//
// 7      UR - Update ready (Rockwell R6545 only)
//          0     Register 31 has been either read or written by the CPU
//          1     an update strobe has occured
// 6      LRF - LPEN Register Full 
//          0     Register 16 or 17 has been read by the CPU
//          1     LPEN strobe has occured
// 5      VRT - Vertical retrace 
//          0     Scan is currently not in the vertical blanking position
//          1     [MOS6545] Scan is currently in its vertical blanking time 
//          1     [R6545] Scan is currently in its vertical re-trace time.
//                   Note: this bit goes to a 1 one when vertical re-trace starts.
//                   It goes to a 0 five character clock times before vertical
//                   re-trace ends to ensure that critical timings for refresh
//                   RAM operations are met.
//
//-------------------------------------------------------------------------------------------------
//
// Registers
//
//-------------------------------------------------------------------------------------------------
//
// From Here: http://www.6502.org/users/andre/hwinfo/crtc/crtc.html
//
//		R0. Total length of line (displayed and non-displayed cycles (retrace) in CCLK cycles minus 1 )
//		R1. Number of characters displayed in a line
//		R2. The position of the horizontal sync pulse start in distance from line start
//		R3. (Bits 0-3) The width of the horizontal sync pulse in CCLK cycles (0 means 16) .
//		    (Bits 4-7) Length of vertical sync pulse in times of a rasterline (Bits 4-7)
//		R4. (7_bit) The number of character lines of the screen minus 1
//		R5. (5_bit) The additional number of scanlines to complete a screen
//		R6. (7_bit) Number character lines that are displayed
//		R7. (7_bit) Position of the vertical sync pulse in character lines.
//		R8.
//		R9. Number of scanlines per character minus 1
//		R10. (5+2_bit)  Bits 0-4 start scanline of cursor
//				 Bits 6,5: 0  0    non-blink
//						   0  1    Cursor non-display
//						   1  0    blink, 1/16 frame rate
//						   1  1    blink, 1/32 frame rate
//		R11. (5_bit) Bits 0-4 last scanline of cursor
//		R12. (6_bit) Bits 8-13 of the start of display memory address
//		R13. (8_bit) Bits 0-7 of the start of display memory address
//		R14. (6_bit) Bits 8-13 of the memory address where Cursor Enable should be active 
//		R15. (8_bit) Bits 0-7 of the Cursor Enable memory register
//		R16. (6_bit) Light Pen H
//		R17. (8_bit) Light Pen L
//
//-------------------------------------------------------------------------------------------------

u8 CRTC_6845::WriteRegisterAddress( u16 address, u8 value )
{
	assert( value < 18 );
	m_nCurrentRegister = value;
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 CRTC_6845::WriteRegisterFile( u16 address, u8 value )
{	
	//
	// Must have passed register before file
	//
	assert( m_nCurrentRegister != -1 );

	m_videoULA.NotifyRegisterWrite( m_nCurrentRegister, value, true );
	//SetRegister( m_nCurrentRegister, value );
	//
	// Don't write this again
	//
	m_nCurrentRegister = -1;
	
	return value;
}


//-------------------------------------------------------------------------------------------------
u8 CRTC_6845::ReadRegisterFile( u16 address, u8 value )
{
	return r[ m_nCurrentRegister ];
}

//-------------------------------------------------------------------------------------------------
CRTC_6845::CRTC_6845( VideoULA& videoULA )
	: m_videoULA( videoULA )
{
	m_nCurrentRegister = -1;
	memset( r, 0, sizeof( r ) );
	mem.RegisterMemoryMap_Write( SHEILA::WRITE_6845_CRTC_Address_register,	MemoryMapHandler( CRTC_6845::WriteRegisterAddress ) );
	mem.RegisterMemoryMap_Write( SHEILA::WRITE_6845_CRTC_Register_file,		MemoryMapHandler( CRTC_6845::WriteRegisterFile ) );
	mem.RegisterMemoryMap_Read( SHEILA::READ_6845_CRTC_Register_file,		MemoryMapHandler( CRTC_6845::ReadRegisterFile ) );
}

//-------------------------------------------------------------------------------------------------

void CRTC_6845::VSync()
{
}

//-------------------------------------------------------------------------------------------------

void CRTC_6845::Tick()
{
}

//-------------------------------------------------------------------------------------------------
