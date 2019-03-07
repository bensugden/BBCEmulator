#pragma once

//-------------------------------------------------------------------------------------------------
class CRTC_6845
{
public:
	CRTC_6845( class VideoULA& );
	void Tick();
	
	//
	// Register Names & Usage
	//
	enum Register
	{
		Horizontal_Total = 0,					// R0. Total length of line (displayed and non-displayed cycles (retrace) in CCLK cycles minus 1 )
		Horizontal_displayed_character_lines,	// R1. Number of characters displayed in a line
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
		Lightpen_address_high,					// R16. (6_bit) Light Pen H
		Lightpen_address_low,					// R17. (8_bit)	Light Pen L
	};

	//-------------------------------------------------------------------------------------------------
	inline void						SetRegister( u8 reg, u8 value ) { r[ m_nCurrentRegister ] = value; }
	u8								GetRegisterValue( u8 reg ) const { return r[ reg ]; }
	void							VSync();
private:
	u8								WriteRegisterAddress(  u16 address, u8 value  );
	u8								WriteRegisterFile(  u16 address, u8 value  );
	u8								ReadRegisterFile(  u16 address, u8 value  );
	int								m_nCurrentRegister;
	u8								r[18];
	class VideoULA&					m_videoULA;
};

//-------------------------------------------------------------------------------------------------
