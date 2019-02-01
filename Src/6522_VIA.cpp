//-------------------------------------------------------------------------------------------------
//
// 6522 Versatile Interface Adapters
//
//-------------------------------------------------------------------------------------------------
#include "stdafx.h"

//-------------------------------------------------------------------------------------------------

void VIA_6522::Write6522Via_A( u16 address, u8 value )
{
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::Write6522Via_B( u16 address, u8 value )
{
}

//-------------------------------------------------------------------------------------------------

VIA_6522::VIA_6522()
{
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B			, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_ORA_IRA_Output_register_A			, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_DDRB_Data_direction_register_B	, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_DDRA_Data_direction_register_A	, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_T1CL_T1_low_order_latches			, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_T1CH_T1_high_order_counter		, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_T1LL_T1_low_order_latches			, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_T1LH_T1_high_order_latches		, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_T2CL_T2_low_order_latches			, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_T2CH_T2_high_order_counter		, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_SR_Shift_register					, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_ACR_Auxiliary_control_register	, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_PCR_Peripheral_control_register	, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_IFR_Interrupt_flag_register		, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_IER_Interrupt_enable_register		, MemoryMapHandler( VIA_6522::Write6522Via_A ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_A_ORA_IRA_NO_HANDSHAKE				, MemoryMapHandler( VIA_6522::Write6522Via_A ));

	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_ORB_IRB_Output_register_B			, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_ORA_IRA_Output_register_A			, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_DDRB_Data_direction_register_B	, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_DDRA_Data_direction_register_A	, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_T1CL_T1_low_order_latches			, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_T1CH_T1_high_order_counter		, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_T1LL_T1_low_order_latches			, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_T1LH_T1_high_order_latches		, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_T2CL_T2_low_order_latches			, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_T2CH_T2_high_order_counter		, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_SR_Shift_register					, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_ACR_Auxiliary_control_register	, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_PCR_Peripheral_control_register	, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_IFR_Interrupt_flag_register		, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_IER_Interrupt_enable_register		, MemoryMapHandler( VIA_6522::Write6522Via_B ));
	mem.RegisterMemoryMappedAddress( SHEILA::WRITE_6522_VIA_B_ORA_IRA_NO_HANDSHAKE				, MemoryMapHandler( VIA_6522::Write6522Via_B ));
}

//-------------------------------------------------------------------------------------------------