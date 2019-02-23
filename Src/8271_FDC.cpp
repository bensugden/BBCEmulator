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
	m_uResultRegister = 0;
	m_uCurrentTrack[ 0 ] = 0;
	m_uCurrentTrack[ 1 ] = 0;
	m_disk[ 0 ] = nullptr;
	m_disk[ 1 ] = nullptr;
}

//-------------------------------------------------------------------------------------------------

void FDC_8271::InsertDisk( int nDrive, FloppyDisk* disk)
{
	m_disk[ nDrive ] = disk;
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

	if ( m_currentPhase == Phase_None || m_currentPhase == Phase_Result )
	{
		m_uStatusRegister &= ~Status_Result_Register_Full;

		m_uCommandDrive = value & 0x80 ? 1 : 0;
		m_uCurrentCommand = (ECommand)(value & 0x3f);
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
		m_uStatusRegister &= ~Status_Parameter_Register_Full;

		//
		// If no parameters needed, proceed to execute command immediately
		//
		if ( m_uNumParametersRequired == 0 )
		{
			ExecuteCommand();
		}
	}
	return value;
}

//-------------------------------------------------------------------------------------------------

void FDC_8271::ExecuteCommand()
{
	m_currentPhase = Phase_Execution;

	//
	// Perform initialization of specific commands prior to reading result
	//
	m_nNumBytesToTransfer = 0;

	switch ( m_uCurrentCommand )
	{
		case Command_Initialization:
		{
			m_nStepRate			= m_uParameters[ 1 ];
			m_nHeadSettlingTime = m_uParameters[ 2 ] * m_nStepRate; 
			m_nIndexCount		= m_uParameters[ 3 ] >> 4; 
			m_nHeadLoadTime		= ( m_uParameters[ 3 ] & 0xf ) * m_nStepRate; 
			break;
		}
		case Command_Read_Drive_Status:
		case Command_Load_Bad_Tracks:
		case Command_Read_Special_Reg:
		case Command_Read_ID:
		{
			break;
		}

		case Command_Write_Special_Reg:
		{
			u8 value = m_uParameters[ 1 ];
			switch( m_uParameters[ 0 ] )
			{
				case 0x06:
					m_uScanSectorNumber = value;
					break;
				case 0x14:
					m_uScanCount &= 0xff;
					m_uScanCount |= value << 8;
					break;
				case 0x13:
					m_uScanCount &= 0xff00;
					m_uScanCount |= value;
					break;
				case 0x12:
					m_uCurrentTrack[ 0 ] = value;
					break;
				case 0x1a:
					m_uCurrentTrack[ 1 ] = value;
					break;
				case 0x17:
					m_uModeRegister = value;
					break;
				case 0x23:
					m_uDriveControlOutputPort = value;
					break;
				case 0x22:
					m_uDriveControlInputPort = value;
					break;
				case 0x10:
					m_nSurface0BadTrack1 = value;
					break;
				case 0x11:
					m_nSurface0BadTrack2 = value;
					break;
				case 0x18:
					m_nSurface1BadTrack1 = value;
					break;
				case 0x19:
					m_nSurface1BadTrack2 = value;
					break;
				default:
					break;
			}
		}
		case Command_Var_Verify_Data_And_Deleted:
		{
			break;
		}
		case Command_Seek:
		{
			break;
		}
		case Command_128byte_Read_Data:
		case Command_128byte_Read_Data_And_Deleted:
		case Command_128byte_Verify_Data_And_Deleted:
		{
			m_uStatusRegister |= Status_Result_Register_Full;
		}
		case Command_128byte_Write_Data:
		case Command_128byte_Write_Deleted:
		{
			m_nNumBytesToTransfer = 128;
			m_uCurrentTrack[ m_uCommandDrive ] = m_uParameters[ 0 ];
			m_uCurrentSector[ m_uCommandDrive ] = m_uParameters[ 1 ];

			break;
		}
		case Command_Var_Read_Data:
		case Command_Var_Read_Data_And_Deleted:
		{
			m_uStatusRegister |= Status_Result_Register_Full;
		}
		case Command_Var_Write_Data:
		case Command_Var_Write_Deleted:
		{
			m_uCurrentTrack[ m_uCommandDrive ] = m_uParameters[ 0 ];
			m_uCurrentSector[ m_uCommandDrive ] = m_uParameters[ 1 ];
			u16 uRecordSize = 128 << ( m_uParameters[ 2 ] >> 5 );
			m_nNumBytesToTransfer = uRecordSize * ( m_uParameters[ 2 ] & 0x1f );

			break;
		}
		case Command_Format:
		{
			break;
		}

		case Command_Var_Scan_Data:
		case Command_Var_Scan_Data_And_Deleted:
		{
			break;
		}
		default:
		{
			break;
		}
	}
	m_uStatusRegister |= Status_Interrupt_Request;
	m_uStatusRegister &= ~Status_Command_Busy;

	cpu.ThrowInterrupt( INTERRUPT_NMI );
	m_currentPhase = Phase_Result;
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

	//
	// All parameters submitted, ok to execute command
	//
	if ( m_uNumParametersRequired == m_uNumParameters )
	{
		ExecuteCommand();
	}

	return value;
}

//-------------------------------------------------------------------------------------------------

u8 FDC_8271::WriteResetRegister( u16 address, u8 value )
{
	//
	// Note needs to be active for >=11 clock ticks to reset
	//
	return value;
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

	return m_uResultRegister; // return 0 always for now
}

//-------------------------------------------------------------------------------------------------

u8 FDC_8271::WriteData( u16 address, u8 value )
{
	switch ( m_uCurrentCommand )
	{
		case Command_128byte_Write_Data:
		case Command_128byte_Write_Deleted:
		case Command_128byte_Verify_Data_And_Deleted:
		case Command_Var_Write_Data:
		case Command_Var_Write_Deleted:
		{
			value = m_disk[ m_uCommandDrive ]->Read( m_uCurrentTrack[ m_uCommandDrive ], m_uCurrentSector[ m_uCommandDrive ], m_nReadWriteOffset++, 0 );
			if ( m_nReadWriteOffset == m_nNumBytesToTransfer )
			{
				m_currentPhase = Phase_None;
			}
		}

		case Command_Read_Special_Reg:
		case Command_Read_Drive_Status:
		case Command_Initialization:
		case Command_Load_Bad_Tracks:
		case Command_Read_ID:
		case Command_Write_Special_Reg:
		case Command_Seek:
		case Command_Format:
		case Command_Var_Scan_Data:
		case Command_Var_Scan_Data_And_Deleted:
		default:
		{
			break;
		}
	}
	return value;
}
//-------------------------------------------------------------------------------------------------

u8 FDC_8271::ReadData( u16 address, u8 value )
{
	switch ( m_uCurrentCommand )
	{
		case Command_Read_Special_Reg:
		{
			switch( m_uParameters[ 0 ] )
			{
				case 0x06:
					return m_uScanSectorNumber;
				case 0x14:
					return m_uScanCount >> 8;
				case 0x13:
					return m_uScanCount & 0xff;
				case 0x12:
					return m_uCurrentTrack[ 0 ];
				case 0x1a:
					return m_uCurrentTrack[ 1 ];
				case 0x17:
					return m_uModeRegister;
				case 0x23:
					return m_uDriveControlOutputPort;
				case 0x22:
					return m_uDriveControlInputPort;
				case 0x10:
					return m_nSurface0BadTrack1;
				case 0x11:
					return m_nSurface0BadTrack2;
				case 0x18:
					return m_nSurface1BadTrack1;
				case 0x19:
					return m_nSurface1BadTrack2;
				default:
					return 0;
			}
		}
		case Command_128byte_Read_Data:
		case Command_128byte_Read_Data_And_Deleted:
		case Command_128byte_Verify_Data_And_Deleted:
		case Command_Var_Read_Data:
		case Command_Var_Read_Data_And_Deleted:
		case Command_Var_Verify_Data_And_Deleted:
		{
			value = m_disk[ m_uCommandDrive ]->Read( m_uCurrentTrack[ m_uCommandDrive ], m_uCurrentSector[ m_uCommandDrive ], m_nReadWriteOffset++, 0 );
			if ( m_nReadWriteOffset == m_nNumBytesToTransfer )
			{
				m_uStatusRegister &= ~Status_Result_Register_Full;
				m_currentPhase = Phase_None;
			}
		}
		case Command_Read_Drive_Status:
		case Command_Initialization:
		case Command_Load_Bad_Tracks:
		case Command_Read_ID:
		case Command_Seek:
		case Command_Format:
		case Command_Var_Scan_Data:
		case Command_Var_Scan_Data_And_Deleted:
		default:
		{
			break;
		}
	}
	return value;
}

//-------------------------------------------------------------------------------------------------