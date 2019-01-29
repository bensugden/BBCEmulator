#pragma once

//-------------------------------------------------------------------------------------------------
//
// SHEILA I/O Interface
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------

namespace SHEILA
{
	enum Write_Address
	{
		WRITE_6845_CRTC_Address_register							= 0xFE00,
		WRITE_6845_CRTC_Register_file								= 0xFE01,
		WRITE_6850_ACIA_Control_register							= 0xFE08,
		WRITE_6850_ACIA_Transmit_data_register						= 0xFE09,
		WRITE_Serial_ULA_Control_register							= 0xFE10,
		WRITE_Video_ULA_Control_register							= 0xFE20,
		WRITE_Video_ULA_Palette_register							= 0xFE21,
		WRITE_LS161_Paged_ROM_RAM_ID								= 0xFE30,
		WRITE_Shadow_RAM_select_top_bit								= 0xFE34,
		WRITE_6522_VIA_MOS_input_output								= 0xFE40,
		WRITE_6522_VIA_ORB_IRB_Output_register_B					= 0xFE60,
		WRITE_6522_VIA_ORA_IRA_Output_register_A					= 0xFE61,
		WRITE_6522_VIA_DDRB_Data_direction_register_B				= 0xFE62,
		WRITE_6522_VIA_DDRA_Data_direction_register_A				= 0xFE63,
		WRITE_6522_VIA_T1CL_T1_low_order_latches					= 0xFE65,
		WRITE_6522_VIA_T1CH_T1_high_order_counter					= 0xFE65,
		WRITE_6522_VIA_T1LL_T1_low_order_latches					= 0xFE66,
		WRITE_6522_VIA_T1LH_T1_high_order_latches					= 0xFE67,
		WRITE_6522_VIA_T2CL_T2_low_order_latches					= 0xFE68,
		WRITE_6522_VIA_T2CH_T2_high_order_counter					= 0xFE69,
		WRITE_6522_VIA_SR_Shift_register							= 0xFE6A,
		WRITE_6522_VIA_ACR_Auxiliary_control_register				= 0xFE6B,
		WRITE_6522_VIA_PCR_Peripheral_control_register				= 0xFE6C,
		WRITE_6522_VIA_IFR_Interrupt_flag_register					= 0xFE6D,
		WRITE_6522_VIA_IER_Interrupt_enable_register				= 0xFE6E,
		WRITE_6522_VIA_ORA_IRA_NO_HANDSHAKE							= 0xFE6F,
		WRITE_8271_FDC_Command_register								= 0xFE80,
		WRITE_8271_FDC_Parameter_register							= 0xFE81,
		WRITE_8271_FDC_Reset_register								= 0xFE82,
		WRITE_8271_FDC_Illegal										= 0xFE83,
		WRITE_8271_FDC_Write_data									= 0xFE84,
		WRITE_1770_FDC_Drive_select									= 0xFE80,
		WRITE_1770_FDC_Control										= 0xFE84,
		WRITE_1770_FDC_Track										= 0xFE85,
		WRITE_1770_FDC_Sector										= 0xFE86,
		WRITE_1770_FDC_Data											= 0xFE87,
		WRITE_68B54_ADLC_CR1_SR1_Control_register_1					= 0xFEA0,
		WRITE_68B54_ADLC_CR2_SR2_Control_register_2_3				= 0xFEA1,
		WRITE_68B54_ADLC_TxFIFO_RxFIFO_Transmit_FIFO_continue		= 0xFEA2,
		WRITE_68B54_ADLC_TxFIFO_RxFIFO_Transmit_FIFO_terminate		= 0xFEA3,
		WRITE_µPD7002_Data_latch_A_D_start							= 0xFEC0,
		WRITE_ADC													= 0xFEC1,
	};

	//-------------------------------------------------------------------------------------------------

	enum Read_Address
	{
		READ_6850_ACIA_Status_register								= 0xFE08,
		READ_6850_ACIA_Receive_data_register						= 0xFE09,
		READ_6522_VIA_Input_register_B								= 0xFE60,
		READ_6522_VIA_Input_register_A								= 0xFE61,
		READ_6522_VIA_T1_low_order_counter							= 0xFE65,
		READ_6522_VIA_T2_low_order_counter							= 0xFE68,
		READ_8271_FDC_Status_register								= 0xFE80,
		READ_8271_FDC_Result_register								= 0xFE81,
		READ_8271_FDC_Illegal										= 0xFE83,
		READ_8271_FDC_Read_data										= 0xFE84,
		READ_1770_FDC_Status										= 0xFE84,
		READ_1770_FDC_Track											= 0xFE85,
		READ_1770_FDC_Sector										= 0xFE86,
		READ_1770_FDC_Data											= 0xFE87,
		READ_68B54_ADLC_Status_register_1							= 0xFEA0,
		READ_68B54_ADLC_Status_register_2							= 0xFEA1,
		READ_68B54_ADLC_Receive_FIFO_0								= 0xFEA2,
		READ_68B54_ADLC_Receive_FIFO_1								= 0xFEA3,
		READ_µPD7002_Status											= 0xFEC0,
		READ_ADC_High_byte_of_result								= 0xFEC1,
	};

	//-------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 