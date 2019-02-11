//-------------------------------------------------------------------------------------------------
//
// 6522 Versatile Interface Adapters
//
// TODO - work out correct functionality for timer2
//        get correct timing for timer1
//		  shift registers
//		  handshake
//		  port a/port b registers
//		  ....a lot still to do!
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------
//
// Interrupt flag register	
//
// Writing 1 to any bit (except 7) will clear that flag ( i.e. we acknowledge that interrupt )
// Bit 7 is set if any bits (0-6) in the register are set
//
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteIFR( u16 address, u8 value )
{
	//
	// Clear any bits specified
	//
	reg.IFR &= ~value;

	UpdateIFR();

	return reg.IFR;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::ReadIFR( u16 address, u8 value )
{
	return reg.IFR;
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::UpdateIFR()
{
	//
	// Set top bit if any bits are set & throw interrupt, otherwise clear
	//
	if ( ( reg.IFR & ( ~INTERRUPT_SET ) ) & ( reg.IER & 0x7f )) // 0x7f
	{
		reg.IFR |= INTERRUPT_SET; // ( bit 7 or 0x80 )
		cpu.ThrowInterrupt( INTERRUPT_IRQ );
	}
	else
	{
		reg.IFR &= ~INTERRUPT_SET ; // no interrupts flagged at this stage
	}
}

//-------------------------------------------------------------------------------------------------
//
// Interrupt enable register	
//
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteIER( u16 address, u8 value )
{
	if ( value & INTERRUPT_SET )  // ( bit 7 or 0x80 )
	{
		//
		// Set mode. Sets any 1's in "value" to 1 in the IER
		//
		reg.IER |= value & ( ~INTERRUPT_SET ); // 0x7f
	}
	else
	{
		//
		// Clear mode. Any 1's in "value" will be cleared in the IER
		//
		reg.IER &= ~value; // NOTE: bit 7 is zero, so this is "masked in" implicitly

		//
		// Clear corresponding bits in IFR. 
		//
	}

	reg.IER |= INTERRUPT_SET; // always set to 1 for reading ( bit 7 or 0x80 )
	UpdateIFR();

	return reg.IER;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteT1( u16 address, u8 value )
{
	ReadWriteChannel in = ( ReadWriteChannel )( address - m_baseAddress );

	if ( ( in == RW_T1C_L ) || ( in == RW_T1L_L ) )
	{
		//
		// These are the same op. Copy to low latch
		//
		reg.T1_LATCH_L = value;
	}
	else if ( ( in == RW_T1C_H ) || ( in == RW_T1L_H ) )
	{
		//
		// Copy to high latch
		//
		reg.T1_LATCH_H = value;

		//
		// Reset T1 interrupt flag
		//
		mem.Write( m_baseAddress + RW_IFR, INTERRUPT_TIMER1 );

		//
		// RW_T1C_H also transfers latch into low and high counters (i.e. readies another timer)
		//
		if ( in == RW_T1C_H )
		{
			reg.T1_COUNTER_L = reg.T1_LATCH_L;
			reg.T1_COUNTER_H = reg.T1_LATCH_H;

			//
			// One shot mode. PB7 low on load
			//
			if ( ( reg.ACR >> 6 ) == 2 )
			{
				reg.PCR &= 0x7f; // clear PB7
			}
		}
	}
	return value;
}


//-------------------------------------------------------------------------------------------------

u8 VIA_6522::ReadT1( u16 address, u8 value )
{
	ReadWriteChannel in = ( ReadWriteChannel )( address - m_baseAddress );

	switch ( in )
	{
		case RW_T1C_L:
		{
			//
			// Clear T1 interrupt flag
			//
			mem.Write( m_baseAddress + RW_IFR, INTERRUPT_TIMER1 );

			return reg.T1_COUNTER_L;
		}
		case RW_T1C_H:
			return reg.T1_COUNTER_H;
		case RW_T1L_L:
			return reg.T1_LATCH_L;
		case RW_T1L_H:
			return reg.T1_LATCH_H;
	}
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteT2( u16 address, u8 value )
{
	ReadWriteChannel in = ( ReadWriteChannel )( address - m_baseAddress );

	if ( in == RW_T2C_L )
	{
		reg.T2_LATCH_L = value;
	}
	else if ( in == RW_T2C_H )
	{
		reg.T2_COUNTER_H = value;
		reg.T2_COUNTER_L = reg.T2_LATCH_L;
		reg.T2_INTERRUPT_ENABLED = 1;
		//
		// Clear T2 interrupt flag
		//
		mem.Write( m_baseAddress + RW_IFR, INTERRUPT_TIMER2 );
	}

	return value;
}
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::ReadT2( u16 address, u8 value )
{
	ReadWriteChannel in = ( ReadWriteChannel )( address - m_baseAddress );

	if ( in == RW_T2C_L )
	{
		//
		// Clear T2 interrupt flag
		//
		mem.Write( m_baseAddress + RW_IFR, INTERRUPT_TIMER2 );
		return  reg.T2_COUNTER_L;
	}
	else // if ( in == RW_T2C_H )
	{
		return reg.T2_COUNTER_H;
	}
}


//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteACR( u16 address, u8 value )
{
	reg.ACR = value;
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteOR( u16 address, u8 value )
{
	ReadWriteChannel in = ( ReadWriteChannel )( address - m_baseAddress );

	if ( in != RW_ORA_IRA_NO_HANDSHAKE )
	{
		UpdateControlChannel_DuringReadOrWriteOfPort( in );
	}

	//
	// Only copy those bits set in DDR from ORA / ORB to PA / PB
	//
	if ( in == RW_ORB_IRB )
	{
		//
		// Note from page 7 of Datasheet:
		//
		// PB7 will act as an output if DDRB7 = 1 OR if ACR7 = 1
		// However, if both DRB7 and ACR7 are logic 1, PB7 will be controlled from Timer 1
		// and ORB7 will have no effect on the pin.
		//
		u8 mask = reg.DDRB;
		if ( ( reg.DDRB& reg.ACR ) & 0x80 )
		{
			mask = reg.DDRB & 0x7f;
		}
		WritePortB( ( value & mask ) | ( ~reg.DDRB ) );
		reg.ORB = value;
	}
	else //	if ( in == RW_ORA_IRA || in == RW_ORA_IRA_NO_HANDSHAKE ) )
	{
		WritePortA( ( value & reg.DDRA ) | ( ~reg.DDRA ) );
		reg.ORA = value;
	}
	return value;
}


//-------------------------------------------------------------------------------------------------

u8 VIA_6522::ReadIR( u16 address, u8 value )
{
	ReadWriteChannel in = ( ReadWriteChannel )( address - m_baseAddress );

	if ( in != RW_ORA_IRA_NO_HANDSHAKE )
	{
		UpdateControlChannel_DuringReadOrWriteOfPort( in );
	}

	if ( in == RW_ORB_IRB )
	{
		//
		// check if latching - if yes, use IRB else use PB
		//
		value = ( reg.ORB & reg.DDRB ); 
		if ( reg.ACR & 2 )
		{
			value |= ( reg.IRB & ( ~reg.DDRB ) );  // IRB gets set when CB1 made active transition  - read LATCH
		}
		else
		{
			value |= ( ReadPortB() & ( ~reg.DDRB ) ); // Read current value from port B
		}
		return value;
	}
	else
	{
		//
		// check if latching - if yes, use IRA else use PA
		//
		value = ( reg.ORA & reg.DDRA ); 
		if ( reg.ACR & 1 )
		{
			value |= ( reg.IRA & ( ~reg.DDRA ) ); // IRA gets set when CA1 made active transition - read LATCH
		}
		else
		{
			value |= ( ReadPortA() & ( ~reg.DDRA ) ); // Read current value from port A
		}
		return value;
	}
}
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteDDR( u16 address, u8 value )
{
	ReadWriteChannel in = ( ReadWriteChannel )( address - m_baseAddress );
	//
	// Refresh PB by rewriting ORA/ORB registers
	//
	if ( in == RW_DDRA )
	{
		reg.DDRA = value;
		WritePortA( ( reg.ORA & reg.DDRA ) | ( ~reg.DDRA ) );
	}
	else // if ( in == RW_DDRB )
	{
		reg.DDRB = value;
		WritePortB( ( reg.ORB & reg.DDRB ) | ( ~reg.DDRB ) );
	}
	return value;
}
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::GetShiftMode( ) const
{
	return ( reg.ACR >> 2 ) & 7;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteShift( u16 address, u8 value )
{
	reg.SHIFT = value;
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WritePCR( u16 address, u8 value )
{
	reg.PCR = value;
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::GetControlLineModeCA1( ) const
{
	return reg.PCR & 1;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::GetControlLineModeCA2( ) const
{
	return ( reg.PCR >> 1 ) & 7;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::GetControlLineModeCB1( ) const
{
	return ( reg.PCR >> 4 ) & 1;
}


//-------------------------------------------------------------------------------------------------

u8 VIA_6522::GetControlLineModeCB2( ) const
{
	return ( reg.PCR >> 5 ) & 7;
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::UpdateControlChannel_DuringReadOrWriteOfPort( ReadWriteChannel in )
{
	InterruptFlags   controlChannelInterrupt1 = ( in == RW_ORA_IRA ) ? INTERRUPT_CA1 : INTERRUPT_CB1;
	InterruptFlags   controlChannelInterrupt2 = ( in == RW_ORA_IRA ) ? INTERRUPT_CA2 : INTERRUPT_CB2;

	u8 mode = ( in == RW_ORA_IRA ) ? GetControlLineModeCA2( ) : GetControlLineModeCB2( );
	if ( mode == 0 || mode == 2 )
	{
		//
		// Clear Interrupt Flag for control channel 2
		//
		mem.Write( m_baseAddress + RW_IFR, controlChannelInterrupt2 );
	}
	else
	if ( mode == 4 )
	{
		//
		// Mode 4:
		// Handshake output mode - Set CA2/CB2 output pin low on a read or write of the Port 'A'/'B' data register. 
		// Reset CA2 pin to high with an active transition of the CA1 input pin.
		// 
		if ( in == RW_ORA_IRA )
			reg.CA2 = 0;
		else
			reg.CB2 = 0;
	}
	else
	if ( mode == 5 )
	{
		//
		// Mode 5:
		// Pulse output mode - CA2/CB2 goes low for one cycle following a read or write of the Port 'A'/'B' data register.
		// 
		if ( in == RW_ORA_IRA )
		{
			reg.CA2 = 0;
			reg.CA2_TIMER = 2;  // 2 or 1 ?
		}
		else
		{
			reg.CB2 = 0;
			reg.CB2_TIMER = 2;  // 2 or 1 ?
		}
	}
	//
	// Clear Interrupt Flag CA1/CB1
	//
	mem.Write( m_baseAddress + RW_IFR, controlChannelInterrupt1 );
}
//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCA1( u8 value )
{
	assert( value < 2 ); // only 1 or 0 allowed as input
	//
	// Check CA1 Control Bit
	//
	if ( GetControlLineModeCA1( ) & 1 )
	{
		//
		// Interrupt on Low to High transition
		//
		if ( reg.CA1 < value )
		{
			ThrowInterrupt( INTERRUPT_CA1 );
		}
	}
	else
	{
		//
		// Interrupt on High to Low transition
		//
		if ( reg.CA1 > value )
		{
			ThrowInterrupt( INTERRUPT_CA1 );
		}
	}
	//
	// Handshake output mode - Reset CA2/CB2 pin to high with an active transition of the CA1/CB1 input pin.
	//
	if ( ( GetControlLineModeCA2( ) == 4 ) && ( value > 0 ) )
	{
		reg.CA2 = 1;
	}
	reg.CA1 = value;
}
//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCB1( u8 value )
{
	assert( value < 2 ); // only 1 or 0 allowed as input
	//
	// Check CB1 Control Bit
	//
	if ( GetControlLineModeCB1( ) & 1 )
	{
		//
		// Interrupt on Low to High transition
		//
		if ( reg.CB1 < value )
		{
			ThrowInterrupt( INTERRUPT_CB1 );
		}
	}
	else
	{
		//
		// Interrupt on High to Low transition
		//
		if ( reg.CB1 > value )
		{
			ThrowInterrupt( INTERRUPT_CB1 );
		}
	}
	//
	// Handshake output mode - Reset CA2/CB2 pin to high with an active transition of the CA1/CB1 input pin.
	//
	if ( ( GetControlLineModeCB2( ) == 4 ) && ( value > 0 ) )
	{
		reg.CB2 = 1;
	}
	reg.CB1 = value;
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCA2( u8 value )
{
	assert( value < 2 ); // only 1 or 0 allowed as input
	//
	// Check GetControlLineMode
	//
	u8 mode = GetControlLineModeCA2( );

	switch ( mode )
	{
		case 0:
		case 1:
		{
			//
			// Mode 0:
			// Interrupt input mode - Set CA2 interrupt flag (IFR0) on a negative transition of the CA2 input pin. 
			// Clear the interrupt flag (IFR0) on a read or write of the Port 'A' data register.
			//
			// Mode 1:
			// Independent interrupt input mode - Set CA2 interrupt flag (IFR0) on a negative transition of the CA2 input pin. 
			// Reading or writing of the Port 'A' data register does not affect the status of the interrupt flag (IFR0).
			// 
			if ( reg.CA2 > value )
			{
				ThrowInterrupt( INTERRUPT_CA2 );
			}
			break;
		}
		case 2:
		case 3:
		{
			//
			// Mode 2:
			// Interrupt input mode - Set CA2 interrupt flag (IFR0) on a positive transition of the CA2 input pin. 
			// Clear the interrupt flag (IFR0) on a read or write of the Port 'A' data register.
			//
			// Mode 3:
			// Independent interrupt input mode - Set CA2 interrupt flag (IFR0) on a positive transition of the CA2 input pin.
			// Reading or writing of the Port 'A' data register does not affect the status of the interrupt flag (IFR0).
			// 
			if ( reg.CA2 < value )
			{
				ThrowInterrupt( INTERRUPT_CA2 );
			}
			break;
		}

		case 6:
		{
			// 
			// Manual output mode - The CA2 output is held low in this mode.
			// 
			value = 0;
			break;
		}
		case 7:
		{
			// 
			// Manual output mode - The CA2 output is held high in this mode.
			// 
			value = 1;
			break;
		}
		default:
		{
			//
			// Modes 4/5 are no-op here
			//
			break;
		}
	}
	reg.CA2 = value;
}
//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCB2( u8 value )
{
	assert( value < 2 ); // only 1 or 0 allowed as input
	//
	// Check GetControlLineMode
	//
	u8 mode = GetControlLineModeCB2( );

	switch ( mode )
	{
		case 0:
		case 1:
		{
			//
			// Mode 0:
			// Interrupt input mode - Set CB2 interrupt flag (IFR0) on a negative transition of the CB2 input pin. 
			// Clear the interrupt flag (IFR0) on a read or write of the Port 'A' data register.
			//
			// Mode 1:
			// Independent interrupt input mode - Set CB2 interrupt flag (IFR0) on a negative transition of the CB2 input pin. 
			// Reading or writing of the Port 'A' data register does not affect the status of the interrupt flag (IFR0).
			// 
			if ( reg.CB2 > value )
			{
				ThrowInterrupt( INTERRUPT_CB2 );
			}
			break;
		}
		case 2:
		case 3:
		{
			//
			// Mode 2:
			// Interrupt input mode - Set CB2 interrupt flag (IFR0) on a positive transition of the CB2 input pin. 
			// Clear the interrupt flag (IFR0) on a read or write of the Port 'A' data register.
			//
			// Mode 3:
			// Independent interrupt input mode - Set CB2 interrupt flag (IFR0) on a positive transition of the CB2 input pin.
			// Reading or writing of the Port 'A' data register does not affect the status of the interrupt flag (IFR0).
			// 
			if ( reg.CB2 < value )
			{
				ThrowInterrupt( INTERRUPT_CB2 );
			}
			break;
		}

		case 6:
		{
			// 
			// Manual output mode - The CB2 output is held low in this mode.
			// 
			value = 0;
			break;
		}
		case 7:
		{
			// 
			// Manual output mode - The CB2 output is held high in this mode.
			// 
			value = 1;
			break;
		}
		default:
		{
			//
			// Modes 4/5 are no-op here
			//
			break;
		}
	}
	reg.CB2 = value;
}
//-------------------------------------------------------------------------------------------------

VIA_6522::VIA_6522( u16 viaMemoryMappedStartAddressForORA_B )
	: m_baseAddress( viaMemoryMappedStartAddressForORA_B )
{
	memset( &reg, 0, sizeof( Registers ) );

	u16 offset = m_baseAddress - SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B;

	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B, MemoryMapHandler( VIA_6522::WriteOR ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_Output_register_A, MemoryMapHandler( VIA_6522::WriteOR ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_DDRB_Data_direction_register_B, MemoryMapHandler( VIA_6522::WriteDDR ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_DDRA_Data_direction_register_A, MemoryMapHandler( VIA_6522::WriteDDR ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1CL_T1_low_order_latches, MemoryMapHandler( VIA_6522::WriteT1 ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1CH_T1_high_order_counter, MemoryMapHandler( VIA_6522::WriteT1 ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1LL_T1_low_order_latches, MemoryMapHandler( VIA_6522::WriteT1 ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1LH_T1_high_order_latches, MemoryMapHandler( VIA_6522::WriteT1 ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T2CL_T2_low_order_latches, MemoryMapHandler( VIA_6522::WriteT2 ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T2CH_T2_high_order_counter, MemoryMapHandler( VIA_6522::WriteT2 ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_SR_Shift_register, MemoryMapHandler( VIA_6522::WriteShift ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ACR_Auxiliary_control_register, MemoryMapHandler( VIA_6522::WriteACR ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_PCR_Peripheral_control_register, MemoryMapHandler( VIA_6522::WritePCR ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_IFR_Interrupt_flag_register, MemoryMapHandler( VIA_6522::WriteIFR ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_IER_Interrupt_enable_register, MemoryMapHandler( VIA_6522::WriteIER ) );
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_NO_HANDSHAKE, MemoryMapHandler( VIA_6522::WriteOR ) );

	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B, MemoryMapHandler( VIA_6522::ReadIR ) );
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_Output_register_A, MemoryMapHandler( VIA_6522::ReadIR ) );
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T1CL_T1_low_order_latches, MemoryMapHandler( VIA_6522::ReadT1 ) );
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T1CH_T1_high_order_counter, MemoryMapHandler( VIA_6522::ReadT1 ) );
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T1LL_T1_low_order_latches, MemoryMapHandler( VIA_6522::ReadT1 ) );
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T1LH_T1_high_order_latches, MemoryMapHandler( VIA_6522::ReadT1 ) );
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T2CL_T2_low_order_latches, MemoryMapHandler( VIA_6522::ReadT2 ) );
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T2CH_T2_high_order_counter, MemoryMapHandler( VIA_6522::ReadT2 ) );
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_IFR_Interrupt_flag_register, MemoryMapHandler( VIA_6522::ReadIFR ) );
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::ThrowInterrupt( InterruptFlags interrupt )
{
	reg.IFR |= interrupt ;
	if ( reg.IER & interrupt )
	{
		reg.IFR |= INTERRUPT_SET;
		cpu.ThrowInterrupt( INTERRUPT_IRQ );
	}
	mem.Write_Internal( m_baseAddress + RW_IFR, reg.IFR );
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::Tick( )
{
	//
	// Timer 1 enabled?
	//
	if ( reg.IER & INTERRUPT_TIMER1 )
	{
		u16 timer = ( u16( reg.T1_COUNTER_H ) << 8 ) + reg.T1_COUNTER_L;

		if ( --timer == 0 )
		{
			u8 mode = reg.ACR >> 6;
			if ( mode == 2 )
			{
				//
				// One shot mode "2". PB7 high on terminate.
				//
				reg.PCR |= 0x80; // set PB7
			}
			else if ( mode == 1 )
			{
				//
				// Free running mode "1"
				//
				reg.T1_COUNTER_L = reg.T1_LATCH_L;
				reg.T1_COUNTER_H = reg.T1_LATCH_H;
			}
			else if ( mode == 3 )
			{
				//
				// Free running mode "3"
				//
				reg.T1_COUNTER_L = reg.T1_LATCH_L;
				reg.T1_COUNTER_H = reg.T1_LATCH_H;
				reg.PCR ^= 0x80; // toggle PB7
			}

			ThrowInterrupt( INTERRUPT_TIMER1 );
		}
		reg.T1_COUNTER_H = timer >> 8;
		reg.T1_COUNTER_L = timer & 0xff;
	}

	//
	// Timer 2
	//
	if ( reg.IER & INTERRUPT_TIMER2 )
	{
		u16 timer = ( u16( reg.T2_COUNTER_H ) << 8 ) + reg.T2_COUNTER_L;
		u8 mode = reg.ACR & 0x20;

		if ( mode == 0 )
		{
			if ( --timer == 0 )
			{
				if ( reg.T2_INTERRUPT_ENABLED )
				{
					//
					// Set T2 interrupt flag
					// Disable further interrupts until T2_H re-written
					//
					ThrowInterrupt( INTERRUPT_TIMER2 );
					reg.T2_INTERRUPT_ENABLED = 0;
				}
			}
		}
		else
		{
			if ( ReadPortB() & 0x40 )
			{
				if ( --timer == 0 )
				{
					if ( reg.T2_INTERRUPT_ENABLED )
					{
						//
						// Set T2 interrupt flag
						// Disable further interrupts until T2_H re-written
						//
						ThrowInterrupt( INTERRUPT_TIMER2 );
						reg.T2_INTERRUPT_ENABLED = 0;
					}
				}
			}
		}
		reg.T2_COUNTER_H = timer >> 8;
		reg.T2_COUNTER_L = timer & 0xff;
	}

	//
	// Check for CA2 / CB2 mode 5 pulse
	//
	u8 CA2Mode = GetControlLineModeCA2( );
	if ( ( CA2Mode == 5 ) && ( reg.CA2 == 0 ) && ( --reg.CA2_TIMER == 0 ) )
	{
		reg.CA2 = 1;
	}
	u8 CB2Mode = GetControlLineModeCB2( );
	if ( ( CB2Mode == 5 ) && ( reg.CB2 == 0 ) && ( --reg.CB2_TIMER == 0 ) )
	{
		reg.CB2 = 1;
	}

}

//-------------------------------------------------------------------------------------------------