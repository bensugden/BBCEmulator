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
	m_register[ IFR ] &= ~value; 

	//
	// Set top bit if any bits are set, otherwise clear
	//
	if ( m_register[ IFR ] & ( ~INTERRUPT_SET ) ) // 0x7f
	{
		m_register[ IFR ] |= INTERRUPT_SET; // ( bit 7 or 0x80 )
	}
	else
	{
		m_register[ IFR ] = 0; // no interrupts flagged at this stage
	}
	return m_register[ IFR ];
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
		m_register[ IER ] |= value & ( ~INTERRUPT_SET ); // 0x7f
	}
	else
	{
		//
		// Clear mode. Any 1's in "value" will be cleared in the IER
		//
		m_register[ IER ] &= ~value; // NOTE: bit 7 is zero, so this is "masked in" implicitly

		//
		// Clear corresponding bits in IFR. 
		// Calling via mem interface so the memory mapped address in RAM also gets updated
		//
		mem.Write( m_baseAddress + IFR, value );
	}

	m_register[ IER ] |= INTERRUPT_SET; // always set to 1 for reading ( bit 7 or 0x80 )

	return m_register[ IER ];
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteT1( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	if ( ( in == RW_T1C_L ) || ( in == RW_T1L_L ) )
	{
		//
		// These are the same op. Copy to low latch
		//
		m_register[ T1_LATCH_L ] = value;
	}
	else if ( ( in == RW_T1C_H ) || ( in == RW_T1L_H ) )
	{
		//
		// Copy to high latch
		//
		m_register[ T1_LATCH_H ] = value;

		//
		// Reset T1 interrupt flag
		//
		mem.Write( m_baseAddress + RW_IFR, INTERRUPT_TIMER1 );

		//
		// RW_T1C_H also transfers latch into low and high counters (i.e. readies another timer)
		//
		if ( in == RW_T1C_H )
		{
			m_register[ T1_COUNTER_L ] = m_register[ T1_LATCH_L ];
			m_register[ T1_COUNTER_H ] = m_register[ T1_LATCH_H ];

			//
			// One shot mode. PB7 low on load
			//
			if ( ( m_register[ ACR ] >> 6 ) == 2 )
			{
				m_register[ PCR ] &= 0x7f; // clear PB7
			}
		}
	}
	return value;
}


//-------------------------------------------------------------------------------------------------

u8 VIA_6522::ReadT1( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	switch ( in )
	{
		case RW_T1C_L:
		{
			//
			// Clear T1 interrupt flag
			//
			mem.Write( m_baseAddress + RW_IFR, INTERRUPT_TIMER1 );

			return m_register[ T1_COUNTER_L ];
		}
		case RW_T1C_H:
			return m_register[ T1_COUNTER_H ];
		case RW_T1L_L:
			return m_register[ T1_LATCH_L ];
		case RW_T1L_H:
			return m_register[ T1_LATCH_H ];
	}
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteT2( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	if ( in == RW_T2C_L )
	{
		m_register[ T2_LATCH_L ] = value;
	}
	else if ( in == RW_T2C_H )
	{
		m_register[ T2_COUNTER_H ] = value;
		m_register[ T2_COUNTER_L ] = m_register[ T2_LATCH_L ];
		m_register[ T2_INTERRUPT_ENABLED ] = 1;
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
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	if ( in == RW_T2C_L )
	{
		//
		// Clear T2 interrupt flag
		//
		mem.Write( m_baseAddress + RW_IFR, INTERRUPT_TIMER2 );
		return  m_register[ T2_COUNTER_L ];
	}
	else // if ( in == RW_T2C_H )
	{
		return m_register[ T2_COUNTER_H ];
	}
}


//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteACR( u16 address, u8 value )
{
	m_register[ ACR ] = value;
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteOR( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	if ( in != RW_ORA_IRA_NO_HANDSHAKE )
	{
		UpdateControlChannel_DuringReadOrWriteOfPort( in );
	}

	//
	// Only copy those bits set in DDR from ORA / ORB to PA / PB
	//
	if ( in == RW_ORA_IRA || in == RW_ORA_IRA_NO_HANDSHAKE )
	{
		m_register[ ORA ] = value;
		m_register[ PA ] &= ~m_register[ DDRA ];
		m_register[ PA ] |= value & m_register[ DDRA ];
	}
	else //	if ( in == RW_ORB_IRB )
	{
		//
		// Note from page 7 of Datasheet:
		//
		// PB7 will act as an output if DDRB7 = 1 OR if ACR7 = 1
		// However, if both DRB7 and ACR7 are logic 1, PB7 will be controlled from Timer 1
		// and ORB7 will have no effect on the pin.
		//
		u8 mask = m_register[  DDRB ];
		if ( ( m_register[  DDRB ] & m_register[ ACR ] ) & 0x80 )
		{
			mask = m_register[  DDRB ] & 0x7f;
		}
		m_register[ ORB ] = value;
		m_register[ PB ] &= ~mask;
		m_register[ PB ] |= value & mask;
	}
	return value;
}


//-------------------------------------------------------------------------------------------------

u8 VIA_6522::ReadIR( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	UpdateControlChannel_DuringReadOrWriteOfPort( in );

	if ( in == RW_ORA_IRA )
	{
		//
		// check if latching - if yes, use IRA else use PA
		//
		return ( m_register[ PCR ] & 1 ) ? m_register[ IRA ] : m_register[ PA ]; // IRA gets set when CA1 made active transition 
	}
	else
	{
		//
		// check if latching - if yes, use IRB else use PB
		//
		u8 input =  ( m_register[ PCR ] & 2 ) ? m_register[ IRB ] : m_register[ PB ]; // IRB gets set when CB1 made active transition 
		return ( m_register[ DDRB ] & m_register[ ORB ] ) | ( (~m_register[ DDRB ]) & input ); 
	}
}
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteDDR( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );
	//
	// Refresh PB by rewriting ORA/ORB registers
	//
	if ( in == RW_DDRA )
	{
		m_register[ DDRA ] = value;
		WriteOR( m_baseAddress + RW_ORA_IRA, m_register[ ORA ] );
	}
	else // if ( in == RW_DDRB )
	{
		m_register[ DDRB ] = value;
		WriteOR( m_baseAddress + RW_ORB_IRB, m_register[ ORB ] );
	}
	return value;
}
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::GetShiftMode( ) const
{
	return ( m_register[ ACR ] >> 2 ) & 7;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteShift( u16 address, u8 value )
{
	m_register[ SHIFT ] = value;
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WritePCR( u16 address, u8 value )
{
	m_register[ PCR ] = value;
	assert( false );//todo
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::GetControlLineMode( InternalRegister reg ) const
{
	if ( reg == CA1 )
		return m_register[ PCR ] & 1;
	if ( reg == CA2 )
		return ( m_register[ PCR ] >> 1 ) & 7;
	if ( reg == CB1 )
		return ( m_register[ PCR ] >> 4 ) & 1;

	return ( m_register[ PCR ] >> 5 ) & 7;
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::UpdateControlChannel_DuringReadOrWriteOfPort( ReadWriteChannel in )
{
	InternalRegister controlChannel2 =			( in == RW_ORA_IRA ) ? CA2				: CB2;
	InternalRegister controlChannelTimer2 =		( in == RW_ORA_IRA ) ? CA2_TIMER		: CB2_TIMER;
	InterruptFlags   controlChannelInterrupt1 =	( in == RW_ORA_IRA ) ? INTERRUPT_CA1	: INTERRUPT_CB1;
	InterruptFlags   controlChannelInterrupt2 =	( in == RW_ORA_IRA ) ? INTERRUPT_CA2	: INTERRUPT_CB2;

	u8 mode = GetControlLineMode( controlChannel2 );
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
		m_register[ controlChannel2 ] = 0;
	}
	else 
	if ( mode == 5 )
	{
		//
		// Mode 5:
		// Pulse output mode - CA2/CB2 goes low for one cycle following a read or write of the Port 'A'/'B' data register.
		// 
		m_register[ controlChannel2 ] = 0;
		m_register[ controlChannelTimer2 ] = 2; // 2 or 1 ?
	}
	//
	// Clear Interrupt Flag CA1/CA2
	//
	mem.Write( m_baseAddress + RW_IFR, controlChannelInterrupt1  );
}
//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCAB1( u8 value, InternalRegister reg, InterruptFlags interrupt )
{
	assert( value < 2 ); // only 1 or 0 allowed as input
	//
	// Check CA1 Control Bit
	//
	if ( GetControlLineMode( reg ) & 1 )
	{
		//
		// Interrupt on Low to High transition
		//
		if ( m_register[ CA1 ] < value )
		{
			mem.Write( m_baseAddress + RW_IFR, INTERRUPT_SET | interrupt );
		}
	}
	else
	{
		//
		// Interrupt on High to Low transition
		//
		if ( m_register[ CA1 ] > value )
		{
			mem.Write( m_baseAddress + RW_IFR, INTERRUPT_SET | interrupt );
		}
	}
	//
	// Handshake output mode - Reset CA2/CB2 pin to high with an active transition of the CA1/CB1 input pin.
	//
	if ( reg == CA1 )
	{
		if ( ( GetControlLineMode( CA2 ) == 4 ) && ( value > 0 ) )
		{
			m_register[ CA2 ] = 1;
		}
	}
	else
	{
		if ( ( GetControlLineMode( CB2 ) == 4 ) && ( value > 0 ) )
		{
			m_register[ CB2 ] = 1;
		}
	}
	m_register[ reg ] = value;
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCAB2( u8 value, InternalRegister reg, InterruptFlags interrupt )
{
	assert( value < 2 ); // only 1 or 0 allowed as input
	//
	// Check GetControlLineMode
	//
	u8 mode = GetControlLineMode( reg );

	switch ( mode )
	{
		case 0 :
		case 1 :
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
			if ( m_register[ CA2 ] > value )
			{
				mem.Write( m_baseAddress + RW_IFR, INTERRUPT_SET | interrupt );
			}
			break;
		}
		case 2 :
		case 3 :
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
			if ( m_register[ CA2 ] < value )
			{	
				mem.Write( m_baseAddress + RW_IFR, INTERRUPT_SET | interrupt );
			}
			break;
		}

		case 6 :
		{
			// 
			// Manual output mode - The CA2 output is held low in this mode.
			// 
			value = 0;
			break;
		}
		case 7 :
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
	m_register[ reg ] = value;
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCA1( u8 value )
{
	SetCAB1( value, CA1, INTERRUPT_CA1 );
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCA2( u8 value )
{
	SetCAB2( value, CA2, INTERRUPT_CA2 );
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCB1( u8 value )
{
	SetCAB1( value, CB1, INTERRUPT_CB1 );
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCB2( u8 value )
{
	SetCAB2( value, CB2, INTERRUPT_CB2 );
}

//-------------------------------------------------------------------------------------------------

VIA_6522::VIA_6522( u16 viaMemoryMappedStartAddressForORA_B )
	: m_baseAddress( viaMemoryMappedStartAddressForORA_B )
{
	memset( m_register, 0, sizeof( m_register ) );

	u16 offset = m_baseAddress - SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B;

	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B		, MemoryMapHandler( VIA_6522::WriteOR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_Output_register_A		, MemoryMapHandler( VIA_6522::WriteOR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_DDRB_Data_direction_register_B	, MemoryMapHandler( VIA_6522::WriteDDR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_DDRA_Data_direction_register_A	, MemoryMapHandler( VIA_6522::WriteDDR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1CL_T1_low_order_latches		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1CH_T1_high_order_counter		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1LL_T1_low_order_latches		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1LH_T1_high_order_latches		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T2CL_T2_low_order_latches		, MemoryMapHandler( VIA_6522::WriteT2 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T2CH_T2_high_order_counter		, MemoryMapHandler( VIA_6522::WriteT2 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_SR_Shift_register				, MemoryMapHandler( VIA_6522::WriteShift ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ACR_Auxiliary_control_register	, MemoryMapHandler( VIA_6522::WriteACR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_PCR_Peripheral_control_register	, MemoryMapHandler( VIA_6522::WritePCR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_IFR_Interrupt_flag_register		, MemoryMapHandler( VIA_6522::WriteIFR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_IER_Interrupt_enable_register	, MemoryMapHandler( VIA_6522::WriteIER ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_NO_HANDSHAKE				, MemoryMapHandler( VIA_6522::WriteOR ));

	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B			, MemoryMapHandler( VIA_6522::ReadIR ));
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_Output_register_A			, MemoryMapHandler( VIA_6522::ReadIR ));
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T1CL_T1_low_order_latches			, MemoryMapHandler( VIA_6522::ReadT1 ));
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T1CH_T1_high_order_counter		, MemoryMapHandler( VIA_6522::ReadT1 ));
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T1LL_T1_low_order_latches			, MemoryMapHandler( VIA_6522::ReadT1 ));
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T1LH_T1_high_order_latches		, MemoryMapHandler( VIA_6522::ReadT1 ));
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T2CL_T2_low_order_latches			, MemoryMapHandler( VIA_6522::ReadT2 ));
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T2CH_T2_high_order_counter		, MemoryMapHandler( VIA_6522::ReadT2 ));
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::Tick()
{
	//
	// Timer 1 enabled?
	//
	if ( m_register[ IER ] & INTERRUPT_TIMER1 )
	{
		u16 timer = ( u16( m_register[ T1_COUNTER_H ] ) << 8 ) + m_register[ T1_COUNTER_L ];
		
		if ( --timer == 0 )
		{
			u8 mode = m_register[ ACR ] >> 6;
			if ( mode == 2 )
			{
				//
				// One shot mode "2". PB7 high on terminate.
				//
				m_register[ PCR ] |= 0x80; // set PB7
			}
			else if ( mode == 1 )
			{
				//
				// Free running mode "1"
				//
				m_register[ T1_COUNTER_L ] = m_register[ T1_LATCH_L ];
				m_register[ T1_COUNTER_H ] = m_register[ T1_LATCH_H ];
			}
			else if ( mode == 3 )
			{
				//
				// Free running mode "3"
				//
				m_register[ T1_COUNTER_L ] = m_register[ T1_LATCH_L ];
				m_register[ T1_COUNTER_H ] = m_register[ T1_LATCH_H ];
				m_register[ PCR ] ^= 0x80; // toggle PB7
			}
			
			m_register[ IFR ] |= INTERRUPT_SET | INTERRUPT_TIMER1 ; // set bit 7 of Interrupt flag register plus the relevant bit for that interrupt
			cpu.ThrowInterrupt();
		}
		m_register[ T1_COUNTER_H ] = timer >> 8;
		m_register[ T1_COUNTER_L ] = timer & 0xff;
	}

	//
	// Timer 2
	//
	if ( m_register[ IER ] & INTERRUPT_TIMER2 )
	{
		u16 timer = ( u16( m_register[ T2_COUNTER_H ] ) << 8 ) + m_register[ T2_COUNTER_L ];
		u8 mode = m_register[ ACR ] & 0x20;
		
		if  ( mode == 0 )
		{
			if ( --timer == 0 )
			{
				if ( m_register[ T2_INTERRUPT_ENABLED ] ) 
				{
					//
					// Set T2 interrupt flag
					// Disable further interrupts until T2_H re-written
					//
					WriteIFR( m_baseAddress + IFR, INTERRUPT_SET | INTERRUPT_TIMER2 );
					cpu.ThrowInterrupt();
					m_register[ T2_INTERRUPT_ENABLED ] = 0;
				}
			}
		}
		else
		{
			if ( m_register[ PB ] & 0x40 )
			{
				if ( --timer == 0 )
				{
					if ( m_register[ T2_INTERRUPT_ENABLED ] ) 
					{
						//
						// Set T2 interrupt flag
						// Disable further interrupts until T2_H re-written
						//
						WriteIFR( m_baseAddress + IFR, INTERRUPT_SET | INTERRUPT_TIMER2 );
						cpu.ThrowInterrupt();
						m_register[ T2_INTERRUPT_ENABLED ] = 0;
					}
				}
			}
		}
		m_register[ T2_COUNTER_H ] = timer >> 8;
		m_register[ T2_COUNTER_L ] = timer & 0xff;
	}

	//
	// Check for CA2 / CB2 mode 5 pulse
	//
	u8 CA2Mode = GetControlLineMode( CA2 );
	if (( CA2Mode == 5 ) && ( m_register[ CA2 ] == 0 ) && ( --m_register[ CA2_TIMER ] == 0 ) )
	{
		m_register[ CA2 ] = 1;
	}
	u8 CB2Mode = GetControlLineMode( CB2 );
	if (( CB2Mode == 5 ) && ( m_register[ CB2 ] == 0 ) && ( --m_register[ CB2_TIMER ] == 0 ) )
	{
		m_register[ CB2 ] = 1;
	}

}

//-------------------------------------------------------------------------------------------------