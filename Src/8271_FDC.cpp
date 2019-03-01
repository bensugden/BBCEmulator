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
	m_bufferReadOffset = 0;
	m_bufferReadOffsetTest = 0;
	m_disk[ 0 ] = nullptr;
	m_disk[ 1 ] = nullptr;

	m_nDriveStatus = 0;
	m_nTickDelay = 0;
}

//-------------------------------------------------------------------------------------------------

void FDC_8271::InsertDisk( int nDrive, FloppyDisk* disk)
{
	m_disk[ nDrive ] = disk;
	m_nDriveStatus |= ( nDrive == 0 ) ? 0x04 
									  : 0x40;

	if ( disk )
	{
		disk->SetBadTrack( 0, m_nBadTrack[ nDrive ][ 0 ] );
		disk->SetBadTrack( 1, m_nBadTrack[ nDrive ][ 1 ] );
	}
}

//-------------------------------------------------------------------------------------------------

void FDC_8271::EjectDisk( int nDrive )
{
	if ( m_disk[ nDrive ] != nullptr )
	{
		m_disk[ nDrive ]->FlushWrites();
	}
	m_nDriveStatus &= ( nDrive == 0 ) ? ~0x04 
									  : ~0x40;

	m_disk[ nDrive ] = nullptr;
}

//-------------------------------------------------------------------------------------------------
 
void FDC_8271::Tick( int nCPUClockTicks )
{
	if ( m_nTickDelay > 0 )
	{
		m_nTickDelay -= nCPUClockTicks;

		if ( m_nTickDelay <= 0 )
		{
			switch ( m_uCurrentCommand )
			{
				case Command_128byte_Read_Data:
				case Command_128byte_Read_Data_And_Deleted:
				case Command_128byte_Verify_Data_And_Deleted:
				case Command_Var_Read_Data:
				case Command_Var_Read_Data_And_Deleted:
				case Command_Var_Verify_Data_And_Deleted:
				{
					m_uStatusRegister |= Status_Interrupt_Request;
					cpu.ThrowInterrupt( INTERRUPT_NMI );

					m_uResultRegister = 0;
					if ( m_nNumSectorsToTransfer < 0 )
					{
						m_uStatusRegister= Status_Result_Register_Full | Status_Interrupt_Request;
						cpu.ThrowInterrupt( INTERRUPT_NMI );
						m_currentPhase = Phase_None;
						break;
					}
					if ( m_bufferReadOffset > m_bufferReadOffsetTest )
					{
						m_nTickDelay = 160;
						return;
					}
					m_nDataRegister = m_buffer[ m_bufferReadOffset++ ];
					bool bLastByte = false;
					if ( m_bufferReadOffset >= m_nSectorSize ) 
					{
						m_bufferReadOffset = 0;
						if ( --m_nNumSectorsToTransfer ) 
						{
							m_uCurrentSector[ m_uCommandDrive ]++;
							m_disk[ m_uCommandDrive ]->Read( m_buffer, m_uCurrentTrack[ m_uCommandDrive ], m_uCurrentSector[ m_uCommandDrive ], m_nSectorSize );
					    } 
						else 
						{
							// Last sector done
							m_uStatusRegister = Status_Command_Busy | Status_Interrupt_Request | Status_Non_DMA_Data_Request | Status_Result_Register_Full;
							cpu.ThrowInterrupt( INTERRUPT_NMI );
							bLastByte = true;
							m_nNumSectorsToTransfer = -1;
							m_nTickDelay = 160;
						}
					}
  
					if (!bLastByte) 
					{		
						m_uStatusRegister= Status_Command_Busy | Status_Non_DMA_Data_Request | Status_Interrupt_Request; 
						cpu.ThrowInterrupt( INTERRUPT_NMI );
						m_nTickDelay = 160;
					}

					break;
				}
				case Command_Seek:
				{
					m_uStatusRegister = Status_Result_Register_Full | Status_Interrupt_Request;
					m_uResultRegister = 0;
					cpu.ThrowInterrupt( INTERRUPT_NMI );	
					m_currentPhase = Phase_None;
					break;
				}
				case Command_Read_Special_Reg:
				case Command_Read_Drive_Status:
				case Command_Specify:
				case Command_Read_ID:
				case Command_Format:
				case Command_Var_Scan_Data:
				case Command_Var_Scan_Data_And_Deleted:
				default:
				{
					break;
				}
			}
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
			case Command_Specify:
			{
				m_uNumParametersRequired = 4;
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

	m_uParameters[ m_uNumParameters++ ] = value;

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
	//   2 Non-DMA Data Request		// 0x04
	//   3 Interrupt Request		// 0x08
	//   4 Result Register Full		// 0x10
	//   5 Parameter Register Full	// 0x20
	//   6 Command Register Full	// 0x40
	//   7 Command Busy				// 0x80
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
	m_uStatusRegister &= ~( Status_Result_Register_Full | 
							Status_Interrupt_Request );

	return m_uResultRegister; 
}

//-------------------------------------------------------------------------------------------------

void FDC_8271::ExecuteCommand()
{
	m_currentPhase = Phase_Execution;

	//
	// Perform initialization of specific commands prior to reading result
	//
	m_uResultRegister = 0;
	m_uStatusRegister &= ~( Status_Result_Register_Full | Status_Interrupt_Request );
	m_currentPhase = Phase_Result;

	switch ( m_uCurrentCommand )
	{
		case Command_Specify:
		{
			//
			// Choose mode based on 1st parameter & modify parameters / drive accordingly
			//
			if ( m_uParameters[ 0 ] == 0xD )
			{
				//
				// Initialization
				//
				m_nStepRate			= m_uParameters[ 1 ];
				m_nHeadSettlingTime = m_uParameters[ 2 ] * m_nStepRate; 
				m_nIndexCount		= m_uParameters[ 3 ] >> 4; 
				m_nHeadLoadTime		= ( m_uParameters[ 3 ] & 0xf ) * m_nStepRate; 

				m_nBadTrack[0][0] = 0xff;
				m_nBadTrack[0][1] = 0xff;
				m_nBadTrack[1][0] = 0xff;
				m_nBadTrack[1][1] = 0xff;
				if ( m_disk[ 0 ] )
				{
					m_disk[ 0 ]->SetBadTrack( 0, m_nBadTrack[ 0 ][ 0 ] );
					m_disk[ 0 ]->SetBadTrack( 1, m_nBadTrack[ 0 ][ 1 ] );
				}
				if ( m_disk[ 1 ] )
				{
					m_disk[ 1 ]->SetBadTrack( 0, m_nBadTrack[ 1 ][ 0 ] );
					m_disk[ 1 ]->SetBadTrack( 1, m_nBadTrack[ 1 ][ 1 ] );
				}

				m_uCurrentTrack[0] = 0xff;
				m_uCurrentTrack[1] = 0xff;
				m_uStatusRegister &= ~Status_Command_Busy;
				m_currentPhase = Phase_None;
			}
			else
			if ((  m_uParameters[ 0 ] == 0x10 )||(  m_uParameters[ 0 ] == 0x18 ))
			{
				//
				// Load Bad Tracks
				//
				m_uCommandDrive = ( m_uParameters[ 0 ] &8)>>3;
				m_nBadTrack[ m_uCommandDrive ][ 0 ] = m_uParameters[ 1 ];
				m_nBadTrack[ m_uCommandDrive ][ 1 ] = m_uParameters[ 2 ];
				m_uCurrentTrack[ m_uCommandDrive ] = m_uParameters[ 3 ];
				m_uStatusRegister &= ~Status_Command_Busy;
				m_currentPhase = Phase_None;

				if ( m_disk[ m_uCommandDrive ] )
				{
					m_disk[ m_uCommandDrive ]->SetBadTrack( 0, m_nBadTrack[ m_uCommandDrive ][ 0 ] );
					m_disk[ m_uCommandDrive ]->SetBadTrack( 1, m_nBadTrack[ m_uCommandDrive ][ 1 ] );
				}
			}
			break;
		}
		case Command_Read_ID:
		{
			m_uStatusRegister |= Status_Interrupt_Request;
			m_uStatusRegister |= Status_Result_Register_Full;
			m_uStatusRegister &= ~Status_Command_Busy;
			m_currentPhase = Phase_Result;
			break;
		}
		case Command_Read_Drive_Status:
		{
			m_uResultRegister = 0x80 | m_nDriveStatus ;
			m_uStatusRegister |= Status_Result_Register_Full;
			m_uStatusRegister &= ~Status_Command_Busy;
			m_currentPhase = Phase_Result;
			break;
		}
		case Command_Read_Special_Reg:
		{
			m_uResultRegister = ReadSpecialRegister( m_uParameters[ 0 ] );
			m_uStatusRegister |= Status_Result_Register_Full;
			m_uStatusRegister &= ~Status_Command_Busy;
			m_currentPhase = Phase_Result;
			break;
		}
		case Command_Write_Special_Reg:
		{
			WriteSpecialRegister( m_uParameters[ 0 ], m_uParameters[ 1 ] );
			m_uStatusRegister &= ~Status_Command_Busy;
			m_currentPhase = Phase_None;
			break;
		}
		case Command_Seek:
		{
			m_uCurrentTrack[ m_uCommandDrive ] = m_uParameters[ 0 ];
			m_uStatusRegister = Status_Command_Busy;
			m_currentPhase = Phase_Execution;
			m_nTickDelay = 100; // need variable time on scan
			break;
		}
		case Command_128byte_Read_Data:
		case Command_128byte_Read_Data_And_Deleted:
		case Command_128byte_Write_Data:
		case Command_128byte_Write_Deleted:
		case Command_128byte_Verify_Data_And_Deleted:
		{
			m_nSectorSize = 128;
			m_nNumSectorsToTransfer = 1;
			m_uCurrentTrack[ m_uCommandDrive ] = m_uParameters[ 0 ];
			m_uCurrentSector[ m_uCommandDrive ] = m_uParameters[ 1 ];
			m_currentPhase = Phase_Execution;
			m_bufferReadOffset = 0;
			m_bufferReadOffsetTest = 0;
			m_nTickDelay = 200;
			break;
		}
		case Command_Var_Read_Data:
		case Command_Var_Read_Data_And_Deleted:
		case Command_Var_Write_Data:
		case Command_Var_Write_Deleted:
		case Command_Var_Verify_Data_And_Deleted:
		{
			m_uCurrentTrack[ m_uCommandDrive ] = m_uParameters[ 0 ];
			m_uCurrentSector[ m_uCommandDrive ] = m_uParameters[ 1 ];
			m_nSectorSize = 128 << ( m_uParameters[ 2 ] >> 5 );
			m_nNumSectorsToTransfer = ( m_uParameters[ 2 ] & 0x1f );
			m_currentPhase = Phase_Execution;
			
			m_disk[ m_uCommandDrive ]->Read( m_buffer, m_uCurrentTrack[ m_uCommandDrive ], m_uCurrentSector[ m_uCommandDrive ], m_nSectorSize );
			m_bufferReadOffset = 0;
			m_bufferReadOffsetTest = 0;
			m_uStatusRegister = Status_Command_Busy;

			m_nTickDelay = 160;

			break;
		}
		case Command_Format:
		{
			m_uStatusRegister &= ~Status_Command_Busy;
			m_currentPhase = Phase_Execution;
			m_nTickDelay = 200;
			break;
		}

		case Command_Var_Scan_Data:
		case Command_Var_Scan_Data_And_Deleted:
		{
			m_uStatusRegister &= ~Status_Command_Busy;
			m_currentPhase = Phase_Execution;
			m_nTickDelay = 200;
			break;
		}
		default:
		{
			break;
		}
	}

	if ( m_uStatusRegister & Status_Interrupt_Request )
	{
		cpu.ThrowInterrupt( INTERRUPT_NMI );
	}

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
			//value = m_disk[ m_uCommandDrive ]->Read( m_uCurrentTrack[ m_uCommandDrive ], m_uCurrentSector[ m_uCommandDrive ], m_nReadWriteOffset++, 0 );
			//if ( m_nReadWriteOffset == m_nNumBytesToTransfer )
			//{
			//	m_disk[ m_uCommandDrive ]->FlushWrites();
			//	m_currentPhase = Phase_None;
			//	m_uStatusRegister &= ~Status_Command_Busy;
			//	m_nNMITickDelay = 200;
			//}
		}

		case Command_Read_Special_Reg:
		case Command_Read_Drive_Status:
		case Command_Specify:
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

u8 FDC_8271::ReadSpecialRegister( u8 parameter ) const
{
	switch( parameter )
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
			return m_nBadTrack[ 0 ][ 0 ];
		case 0x11:
			return m_nBadTrack[ 0 ][ 1 ];
		case 0x18:
			return m_nBadTrack[ 1 ][ 0 ];
		case 0x19:
			return m_nBadTrack[ 1 ][ 1 ];
		default:
			return 0;
	}
}

//-------------------------------------------------------------------------------------------------

void FDC_8271::WriteSpecialRegister( u8 parameter, u8 value )
{
	switch( parameter )
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
			m_nBadTrack[ 0 ][ 0 ] = value;
		case 0x11:
			m_nBadTrack[ 0 ][ 1 ] = value;
		case 0x18:
			m_nBadTrack[ 1 ][ 0 ] = value;
		case 0x19:
			m_nBadTrack[ 1 ][ 1 ] = value;
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------------------

u8 FDC_8271::ReadData( u16 address, u8 value )
{
    m_uStatusRegister &= ~( Status_Non_DMA_Data_Request | Status_Interrupt_Request );

	switch ( m_uCurrentCommand )
	{
		case Command_Read_Special_Reg:
		{
			return ReadSpecialRegister( m_uParameters[ 0 ] );
		}
		case Command_128byte_Read_Data:
		case Command_128byte_Read_Data_And_Deleted:
		case Command_128byte_Verify_Data_And_Deleted:
		case Command_Var_Read_Data:
		case Command_Var_Read_Data_And_Deleted:
		case Command_Var_Verify_Data_And_Deleted:
		{
			assert( m_bufferReadOffset != m_bufferReadOffsetTest );
			value = m_nDataRegister;
			m_bufferReadOffsetTest++;
			if ( m_bufferReadOffsetTest >= m_nSectorSize ) 
				m_bufferReadOffsetTest =0;
			m_uStatusRegister &= ~( Status_Non_DMA_Data_Request | Status_Interrupt_Request );
		}
		case Command_Read_Drive_Status:
		case Command_Specify:
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