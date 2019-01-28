#pragma once

//-------------------------------------------------------------------------------------------------
//
// SHEILA I/O Interface
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------

class SHEILA
{
public:
	enum Write_Address
	{
		WRITE_6845_CRTC_Address_register							= 0x00,
		WRITE_6845_CRTC_Register_file								= 0x01,
		WRITE_6850_ACIA_Control_register							= 0x08,
		WRITE_6850_ACIA_Transmit_data_register						= 0x09,
		WRITE_Serial_ULA_Control_register							= 0x10,
		WRITE_Video_ULA_Control_register							= 0x20,
		WRITE_Video_ULA_Palette_register							= 0x21,
		WRITE_LS161_Paged_ROM_RAM_ID								= 0x30,
		WRITE_Shadow_RAM_select_top_bit								= 0x34,
		WRITE_6522_VIA_MOS_input_output								= 0x40,
		WRITE_6522_VIA_ORB_IRB_Output_register_B					= 0x60,
		WRITE_6522_VIA_ORA_IRA_Output_register_A					= 0x61,
		WRITE_6522_VIA_DDRB_Data_direction_register_B				= 0x62,
		WRITE_6522_VIA_DDRA_Data_direction_register_A				= 0x63,
		WRITE_6522_VIA_T1CL_T1_low_order_latches					= 0x65,
		WRITE_6522_VIA_T1CH_T1_high_order_counter					= 0x65,
		WRITE_6522_VIA_T1LL_T1_low_order_latches					= 0x66,
		WRITE_6522_VIA_T1LH_T1_high_order_latches					= 0x67,
		WRITE_6522_VIA_T2CL_T2_low_order_latches					= 0x68,
		WRITE_6522_VIA_T2CH_T2_high_order_counter					= 0x69,
		WRITE_6522_VIA_SR_Shift_register							= 0x6A,
		WRITE_6522_VIA_ACR_Auxiliary_control_register				= 0x6B,
		WRITE_6522_VIA_PCR_Peripheral_control_register				= 0x6C,
		WRITE_6522_VIA_IFR_Interrupt_flag_register					= 0x6D,
		WRITE_6522_VIA_IER_Interrupt_enable_register				= 0x6E,
		WRITE_6522_VIA_ORA_IRA_NO_HANDSHAKE							= 0x6F,
		WRITE_8271_FDC_Command_register								= 0x80,
		WRITE_8271_FDC_Parameter_register							= 0x81,
		WRITE_8271_FDC_Reset_register								= 0x82,
		WRITE_8271_FDC_Illegal										= 0x83,
		WRITE_8271_FDC_Write_data									= 0x84,
		WRITE_1770_FDC_Drive_select									= 0x80,
		WRITE_1770_FDC_Control										= 0x84,
		WRITE_1770_FDC_Track										= 0x85,
		WRITE_1770_FDC_Sector										= 0x86,
		WRITE_1770_FDC_Data											= 0x87,
		WRITE_68B54_ADLC_CR1_SR1_Control_register_1					= 0xA0,
		WRITE_68B54_ADLC_CR2_SR2_Control_register_2_3				= 0xA1,
		WRITE_68B54_ADLC_TxFIFO_RxFIFO_Transmit_FIFO_continue		= 0xA2,
		WRITE_68B54_ADLC_TxFIFO_RxFIFO_Transmit_FIFO_terminate		= 0xA3,
		WRITE_µPD7002_Data_latch_A_D_start							= 0xC0,
		WRITE_ADC													= 0xC1,
	};

	//-------------------------------------------------------------------------------------------------

	enum Read_Address
	{
		READ_6850_ACIA_Status_register								= 0x08,
		READ_6850_ACIA_Receive_data_register						= 0x09,
		READ_6522_VIA_Input_register_B								= 0x60,
		READ_6522_VIA_Input_register_A								= 0x61,
		READ_6522_VIA_T1_low_order_counter							= 0x65,
		READ_6522_VIA_T2_low_order_counter							= 0x68,
		READ_8271_FDC_Status_register								= 0x80,
		READ_8271_FDC_Result_register								= 0x81,
		READ_8271_FDC_Illegal										= 0x83,
		READ_8271_FDC_Read_data										= 0x84,
		READ_1770_FDC_Status										= 0x84,
		READ_1770_FDC_Track											= 0x85,
		READ_1770_FDC_Sector										= 0x86,
		READ_1770_FDC_Data											= 0x87,
		READ_68B54_ADLC_Status_register_1							= 0xA0,
		READ_68B54_ADLC_Status_register_2							= 0xA1,
		READ_68B54_ADLC_Receive_FIFO_0								= 0xA2,
		READ_68B54_ADLC_Receive_FIFO_1								= 0xA3,
		READ_µPD7002_Status											= 0xC0,
		READ_ADC_High_byte_of_result								= 0xC1,
	};

	//-------------------------------------------------------------------------------------------------

	static void		Init();
	
	//-------------------------------------------------------------------------------------------------
	//
	// Attach functions to get callbacks when memory locations change
	//
	//-------------------------------------------------------------------------------------------------
	static void		RegisterListener( Write_Address address,  void (*listenerFunction)( Write_Address, u8 ) );
	static void		NotifyMemWrite( u16 address, u8 value );

	//-------------------------------------------------------------------------------------------------
	//
	// These are for the other subsystems to write these values
	//
	//-------------------------------------------------------------------------------------------------
	static u8		Read( Write_Address address );
	static void		Write( Read_Address address, u8 value );
};

//-------------------------------------------------------------------------------------------------
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 