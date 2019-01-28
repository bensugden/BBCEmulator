//-------------------------------------------------------------------------------------------------
//
// 6845 CRTC Video Controller
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

class CRTC_6845
{
public:
	CRTC_6845();
	void Tick();
	
	//
	// Register Names & Usage
	//
	enum Register
	{
		Horizontal_Total = 0,					// R0. Total length of line (displayed and non-displayed cycles (retrace) in CCLK cylces minus 1 )
		Horizontal_Display,						// R1. Number of characters displayed in a line
		Horizontal_Sync_position,				// R2. The position of the horizontal sync pulse start in distance from line start
		Horizontal_Sync_Width,					// R3. (Bits 0-3) The width of the horizontal sync pulse in CCLK cycles (0 means 16) .
												//     (Bits 4-7) Length of vertical sync pulse in times of a rasterline (Bits 4-7)
		Vertical_total_character_lines,			// R4. (7_bit) The number of character lines of the screen minus 1
		Vertical_total_adjust_rasterlines,		// R5. (5_bit) The additional number of scanlines to complete a screen
		Vertical_displayed_character_lines,		// R6. (7_bit) Number character lines that are displayed
		Vertical_Sync_position,					// R7. (7_bit) Position of the vertical sync pulse in character lines.
		Mode_control,							// R8.
		Number_of_rasterlines_per_characterline,// R9. Number of scanlines per character minus 1
		Cursor_start_rasterline_mode_control,	// R10. (5+2_bit)  Bits 0-4 start scanline of cursor
												//		 Bits 6,5: 0  0    non-blink
												//				   0  1    Cursor non-display
												//				   1  0    blink, 1/16 frame rate
												//				   1  1    blink, 1/32 frame rate
		Cursor_end_rasterline,					// R11. (5_bit) Bits 0-4 last scanline of cursor
		Display_start_address_high,				// R12. (6_bit) Bits 8-13 of the start of display memory address
		Display_start_address_low,				// R13. (8_bit) Bits 0-7 of the start of display memory address
		Cursor_address_high,					// R14. (6_bit) Bits 8-13 of the memory address where Cursor Enable should be active 
		Cursor_address_low,						// R15. (8_bit) Bits 0-7 of the Cursor Enable memory register
		Lightpen_address_high,					// R16. (6_bit)
		Lightpen_address_low,					// R17. (8_bit)
	};
private:
	u8 r[18];
};

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

CRTC_6845::CRTC_6845()
{
}

//-------------------------------------------------------------------------------------------------

void CRTC_6845::Tick()
{
}

//-------------------------------------------------------------------------------------------------
