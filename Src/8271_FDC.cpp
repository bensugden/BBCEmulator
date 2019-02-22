//-------------------------------------------------------------------------------------------------
//
// 8271 Floppy Disk Controller
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------

FDC_8271::FDC_8271()
{
	mem.RegisterMemoryMap_Write( SHEILA::WRITE_8271_FDC_Command_register	, MemoryMapHandler( FDC_8271::WriteCommandRegister	 ) );
	mem.RegisterMemoryMap_Write( SHEILA::WRITE_8271_FDC_Parameter_register	, MemoryMapHandler( FDC_8271::WriteParameterRegister ) );
	mem.RegisterMemoryMap_Write( SHEILA::WRITE_8271_FDC_Reset_register		, MemoryMapHandler( FDC_8271::WriteResetRegister	 ) );
	mem.RegisterMemoryMap_Write( SHEILA::WRITE_8271_FDC_Write_data			, MemoryMapHandler( FDC_8271::WriteData				 ) );

	mem.RegisterMemoryMap_Read ( SHEILA::READ_8271_FDC_Status_register		, MemoryMapHandler( FDC_8271::ReadStatusRegister	 ) );
	mem.RegisterMemoryMap_Read ( SHEILA::READ_8271_FDC_Result_register		, MemoryMapHandler( FDC_8271::ReadResultRegister	 ) );
	mem.RegisterMemoryMap_Read ( SHEILA::READ_8271_FDC_Read_data			, MemoryMapHandler( FDC_8271::ReadData				 ) );

	m_currentPhase = Phase_None;
	m_uStatusRegister = 0;
	m_uCurrentTrack[ 0 ] = 0;
	m_uCurrentTrack[ 1 ] = 0;
}

//-------------------------------------------------------------------------------------------------

void FDC_8271::Tick()
{
	switch ( m_currentPhase )
	{
		case Phase_Command:
		{
			break;
		}
		case Phase_Execution:
		{
			bool bStandardResult = m_uCurrentCommand < 0x2C;
			if ( !bStandardResult )
			{
				bool bImmediateResult = ( m_uCurrentCommand & 0xf ) >= 0xC;
				if ( bImmediateResult )
				{
					// wait for result
					if ( m_uStatusRegister & Status_Command_Busy )
					{
						break;
					}
				}
			}
			m_currentPhase = Phase_Result;

			break;
		}
		case Phase_Result:
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

//-------------------------------------------------------------------------------------------------

u8 FDC_8271::WriteCommandRegister( u16 address, u8 value )
{
	//-------------------------------------------------------------------------------------------------
	//
	// Command Register Format
	//
	//-------------------------------------------------------------------------------------------------
	//
	// 0-5 Command Opcode
	// 6-7 Surface/Drive (select bit)
	// A0 = 0
	// A1 = 0
	//
	//-------------------------------------------------------------------------------------------------
	if ( m_uStatusRegister & Status_Command_Busy )
	{
		assert( false ); // cannot issue command while one is busy
		return value;
	}

	if ( m_currentPhase == Phase_None )
	{
		m_uCommandDrive = value & 0x80 ? 1 : 0;
		m_uCurrentCommand = value & 0x3f;
		m_currentPhase = Phase_Command;
		m_uNumParameters = 0;

		//
		// Setup # parameters per command
		//
		switch ( m_uCurrentCommand )
		{
			case Command_Read_Drive_Status:
			{
				m_uNumParametersRequired = 0;
				break;
			}
			case Command_Specify:
			case Command_Seek:
			case Command_Read_Special_Reg:
			{
				m_uNumParametersRequired = 1;
				break;
			}
			case Command_Write_Special_Reg:
			case Command_128byte_Read_Data:
			case Command_128byte_Read_Data_And_Deleted:
			case Command_128byte_Write_Data:
			case Command_128byte_Write_Deleted:
			case Command_128byte_Verify_Data_And_Deleted:
			{
				m_uNumParametersRequired = 2;
				break;
			}
			case Command_Read_ID:
			case Command_Var_Read_Data:
			case Command_Var_Read_Data_And_Deleted:
			case Command_Var_Write_Data:
			case Command_Var_Write_Deleted:
			case Command_Var_Verify_Data_And_Deleted:
			{
				m_uNumParametersRequired = 3;
				break;
			}
			case Command_Format:
			case Command_Var_Scan_Data:
			case Command_Var_Scan_Data_And_Deleted:
			{
				m_uNumParametersRequired = 5;
				break;
			}
			default:
			{
				m_uNumParametersRequired = 0;
				break;
			}
		}

		// initialize DMA channel ( fig 19 on datasheet ) ????

		m_uStatusRegister |= Status_Command_Busy;
	}
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 FDC_8271::WriteParameterRegister( u16 address, u8 value )
{
	//-------------------------------------------------------------------------------------------------
	//
	// Parameter Register Format
	//
	//-------------------------------------------------------------------------------------------------
	//
	// 0-7 Parameter
	// A0 = 0
	// A1 = 1
	//
	//-------------------------------------------------------------------------------------------------
	if ( m_uStatusRegister & Status_Parameter_Register_Full )
		return value;

	if ( !( m_uStatusRegister & Status_Command_Busy ) )
		return value;

	if ( m_currentPhase != Phase_Command )
		return value;

	m_uStatusRegister |= Status_Parameter_Register_Full;

	switch ( m_uCurrentCommand )
	{
		case Command_Specify:
		{
			//
			// Choose mode based on 1st parameter & modify parameters / drive accordingly
			//
			if ( value == 0xD )
			{
				m_uCurrentCommand = Command_Initialization;
				m_uNumParametersRequired = 4;
			}
			else if ( value == 0x10 )
			{
				m_uCurrentCommand = Command_Load_Bad_Tracks;
				m_uCommandDrive = 0;
				m_uNumParametersRequired = 2;
			}
			else if ( value == 0x18 )
			{
				m_uCurrentCommand = Command_Load_Bad_Tracks;
				m_uCommandDrive = 1;
				m_uNumParametersRequired = 2;
			}
			assert( m_uCurrentCommand != Command_Specify ); // invalid parameter
			break;
		}
	}

	m_uParameters[ m_uNumParameters++ ] = value;
	m_uStatusRegister &= ~Status_Parameter_Register_Full;

	if ( m_uNumParametersRequired == m_uNumParameters )
	{
		m_currentPhase = Phase_Execution;
	}

	return value;
}

//-------------------------------------------------------------------------------------------------

u8 FDC_8271::WriteResetRegister( u16 address, u8 value )
{
	//
	// Note needs to be active for >=11 clock ticks to reset
	//
}

//-------------------------------------------------------------------------------------------------

u8 FDC_8271::WriteData( u16 address, u8 value )
{
}

//-------------------------------------------------------------------------------------------------

u8 FDC_8271::ReadStatusRegister( u16 address, u8 value )
{
	//-------------------------------------------------------------------------------------------------
	//
	// Status Register Format
	//
	//-------------------------------------------------------------------------------------------------
	//
	// 0-1 Not used ( 0 )
	//   2 Non-DMA Data Request
	//   3 Interrupt Request
	//   4 Result Register Full
	//   5 Parameter Register Full
	//   6 Command Register Full
	//   7 Command Busy
	// A0 = 0
	// A1 = 0
	//
	//-------------------------------------------------------------------------------------------------
	return m_uStatusRegister;
}

//-------------------------------------------------------------------------------------------------

u8 FDC_8271::ReadResultRegister( u16 address, u8 value )
{
	//-------------------------------------------------------------------------------------------------
	//
	// Result Register Format
	//
	//-------------------------------------------------------------------------------------------------
	//
	//   0 Not used
	// 1-2 Completion Code
	// 3-4 Completion Type
	//   5 Deleted Data Found
	// 6-7 Not used ( = 0 )
	// A0 = 0
	// A1 = 1
	//
	//-------------------------------------------------------------------------------------------------

}

//-------------------------------------------------------------------------------------------------

u8 FDC_8271::ReadData( u16 address, u8 value )
{
}

//-------------------------------------------------------------------------------------------------