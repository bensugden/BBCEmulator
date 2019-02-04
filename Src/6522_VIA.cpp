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
	if ( m_register[ IFR ] & ( ~INTERRUPT_SET_CLEAR ) ) // 0x7f
	{
		m_register[ IFR ] |= INTERRUPT_SET_CLEAR; // ( bit 7 or 0x80 )
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
	if ( value & INTERRUPT_SET_CLEAR )  // ( bit 7 or 0x80 )
	{
		//
		// Set mode. Sets any 1's in "value" to 1 in the IER
		//
		m_register[ IER ] |= value & ( ~INTERRUPT_SET_CLEAR ); // 0x7f
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

	m_register[ IER ] |= INTERRUPT_SET_CLEAR; // always set to 1 for reading ( bit 7 or 0x80 )

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

		if ( in == RW_T1C_L )
		{
			//
			// Update memory to reflect new latch [direct memory write - no mem map callback]
			//
			mem.Write_Internal( m_baseAddress + REG_T1_LATCH_H, m_register[ REG_T1_LATCH_H ] );

			//
			// Reset memory at this address to be counter ( it has latch in currently )
			// It will get rewritten by the memory map function
			//
			value = m_register[ REG_T1_COUNTER_L ];
		}
	}
	else if ( ( in == REG_T1_COUNTER_H ) || ( in == REG_T1_LATCH_H ) )
	{
		//
		// These are the same. Copy to high latch
		//
		m_register[ REG_T1_LATCH_H ] = value;

		//
		// REG_T1_COUNTER_H also transfers latch into low and high counters (i.e. readies another timer)
		//
		if ( reg == REG_T1_COUNTER_H )
		{
			m_register[ REG_T1_COUNTER_L ] = m_register[ REG_T1_LATCH_L ];
			m_register[ REG_T1_COUNTER_H ] = m_register[ REG_T1_LATCH_H ];

			//
			// One shot mode. PB7 low on load
			//
			if ( ( m_register[ REG_ACR ] >> 6 ) == 2 )
			{
				m_register[ REG_PCR ] &= 0x7f; // clear PB7
			}
			//
			// Update memory to reflect new latch   [direct memory write - no mem map callback]
			// Update memory to reflect low counter [direct memory write - no mem map callback]
			//
			mem.Write_Internal( m_baseAddress + REG_T1_LATCH_H, m_register[ REG_T1_LATCH_H ] );
			mem.Write_Internal( m_baseAddress + REG_T1_COUNTER_L, m_register[ REG_T1_COUNTER_L ] );

			//
			// Reset memory at this address to be counter ( it has latch in currently )
			// It will get rewritten by the memory map function
			//
			value = m_register[ REG_T1_COUNTER_H ];
		}
		//
		// Reset T1 interrupt flag
		//
		WriteIFR( m_baseAddress + REG_IFR, INTERRUPT_TIMER1 );
	}
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::ReadT1CL( u16 address, u8 value )
{
	//
	// Clear T1 interrupt flag
	//
	WriteIFR( m_baseAddress + REG_IFR, INTERRUPT_TIMER1 );

	return m_register[ REG_T1_COUNTER_L ];
}
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::ReadT2CL( u16 address, u8 value )
{
	//
	// Clear T2 interrupt flag
	//
	WriteIFR( m_baseAddress + REG_IFR, INTERRUPT_TIMER2 );

	return m_register[ REG_T1_COUNTER_L ];
}
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteT2( u16 address, u8 value )
{
	Register reg = (Register)( address - m_baseAddress );

	if ( reg == REG_T2_COUNTER_L )
	{
		m_register[ _REG_T2_LATCH_L ] = value;
		value = m_register[ REG_T2_COUNTER_L ];
	}
	else if ( reg == REG_T2_COUNTER_H )
	{
		m_register[ REG_T2_COUNTER_L ] = m_register[ _REG_T2_LATCH_L ];
		m_register[ REG_T2_COUNTER_H ] = value;
		WriteIFR( m_baseAddress + REG_IFR, INTERRUPT_TIMER2 );
	}
		
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteACR( u16 address, u8 value )
{
	m_register[ REG_ACR ] = value;
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteOR( u16 address, u8 value )
{
	Register reg = (Register)( address - m_baseAddress );

	if ( re)
	//
	// If a bit in DDR == 0 then it's input
	//                 == 1			  output
	//
	// If bit is INPUT then we ignore write of that bit
	//
	if ( reg == REG_ORA_IRA )
	{
		m_register[ REG_PA ] &= ~m_register[ REG_DDRA ];
		m_register[ REG_PA ] |= value & m_register[ REG_DDRA ];
	}
	else
	{
		//	if ( reg == ORB_IRB )
		m_register[ REG_PB ] &= ~m_register[ REG_DDRB ];
		m_register[ REG_PB ] |= value & m_register[ REG_DDRB ];
	}
	m_register[ REG_ORA ];
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteDDR( u16 address, u8 value )
{
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteShift( u16 address, u8 value )
{
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WritePCR( u16 address, u8 value )
{
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

	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B			, MemoryMapHandler( VIA_6522::ReadOR ));
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_Output_register_A			, MemoryMapHandler( VIA_6522::ReadOR ));

	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T1CL_T1_low_order_latches			, MemoryMapHandler( VIA_6522::ReadT1CL ));
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_T2CL_T2_low_order_latches			, MemoryMapHandler( VIA_6522::ReadT2CL ));
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::Tick()
{
	//
	// Timer 1 enabled?
	//
	if ( m_register[ REG_IER ] & INTERRUPT_TIMER1 )
	{
		u16 timer = ( u16( m_register[ REG_T1_COUNTER_H ] ) << 8 ) + m_register[ REG_T1_COUNTER_L ];
		
		if ( --timer == 0 )
		{
			u8 mode = m_register[ REG_ACR ] >> 6;
			if ( mode == 2 )
			{
				//
				// One shot mode "2". PB7 high on terminate.
				//
				m_register[ REG_PCR ] |= 0x80; // set PB7
			}
			else if ( mode == 1 )
			{
				//
				// Free running mode "1"
				//
				m_register[ REG_T1_COUNTER_L ] = m_register[ REG_T1_LATCH_L ];
				m_register[ REG_T1_COUNTER_H ] = m_register[ REG_T1_LATCH_H ];
			}
			else if ( mode == 3 )
			{
				//
				// Free running mode "3"
				//
				m_register[ REG_T1_COUNTER_L ] = m_register[ REG_T1_LATCH_L ];
				m_register[ REG_T1_COUNTER_H ] = m_register[ REG_T1_LATCH_H ];
				m_register[ REG_PCR ] ^= 0x80; // toggle PB7
			}
			
			m_register[ REG_IFR ] |= INTERRUPT_SET_CLEAR | INTERRUPT_TIMER1 ; // set bit 7 of Interrupt flag register plus the relevant bit for that interrupt
			cpu.ThrowInterrupt();
		}
		m_register[ REG_T1_COUNTER_H ] = timer >> 8;
		m_register[ REG_T1_COUNTER_L ] = timer & 0xff;
	}

	//
	// Timer 2
	//
	if ( m_register[ REG_IER ] & INTERRUPT_TIMER2 )
	{
		u16 timer = ( u16( m_register[ REG_T2_COUNTER_H ] ) << 8 ) + m_register[ REG_T2_COUNTER_L ];
		u8 mode = m_register[ REG_ACR ] & 0x20;
		
		// BEN - not sure about all this. need to clarify functionality
		// re-read page 8 on data sheet about this
		if  ( mode == 0 )
		{
			if ( --timer == 0 )
			{
				//
				// Clear T2 interrupt flag
				//
				WriteIFR( m_baseAddress + REG_IFR, INTERRUPT_TIMER2 );
				// set it. this is clearly wrong.
				m_register[ REG_IFR ] |= INTERRUPT_SET_CLEAR | INTERRUPT_TIMER2 ; // set bit 7 of Interrupt flag register plus the relevant bit for that interrupt
				cpu.ThrowInterrupt();
			}
		}
		else
		{
			if ( m_register[ REG_PCR ] & 0x40 )
			{
				if ( --timer == 0 )
				{
					m_register[ REG_IFR ] |= INTERRUPT_SET_CLEAR | INTERRUPT_TIMER2 ; // set bit 7 of Interrupt flag register plus the relevant bit for that interrupt
					cpu.ThrowInterrupt();
				}
			}
		}
		m_register[ REG_T2_COUNTER_H ] = timer >> 8;
		m_register[ REG_T2_COUNTER_L ] = timer & 0xff;
	}
}

//-------------------------------------------------------------------------------------------------