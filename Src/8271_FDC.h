#pragma once

//-------------------------------------------------------------------------------------------------
//
// 8271 Floppy Disk Controller
//
//-------------------------------------------------------------------------------------------------

class FDC_8271
{
public:
	FDC_8271();
	void InsertDisk( int nDrive, FloppyDisk* disk );

private:
	//-------------------------------------------------------------------------------------------------
	//
	// Memory Map Handlers
	//
	//-------------------------------------------------------------------------------------------------

	u8 WriteCommandRegister		( u16 address, u8 value );
	u8 WriteParameterRegister	( u16 address, u8 value );
	u8 WriteResetRegister		( u16 address, u8 value );
	u8 WriteData				( u16 address, u8 value );

	u8 ReadStatusRegister		( u16 address, u8 value );
	u8 ReadResultRegister		( u16 address, u8 value );
	u8 ReadData					( u16 address, u8 value );

	//-------------------------------------------------------------------------------------------------
	enum EPhase
	{
		Phase_None,
		Phase_Command,
		Phase_Execution,
		Phase_Result
	};

	//-------------------------------------------------------------------------------------------------

	enum EStatusFlag
	{
		Status_Non_DMA_Data_Request		= 1<<2 , // 0x04
		Status_Interrupt_Request		= 1<<3 , // 0x08
		Status_Result_Register_Full		= 1<<4 , // 0x10
		Status_Parameter_Register_Full	= 1<<5 , // 0x20
		Status_Command_Register_Full	= 1<<6 , // 0x40
		Status_Command_Busy				= 1<<7 , // 0x80
	};

	//-------------------------------------------------------------------------------------------------

	enum ECommand
	{
		Command_Specify							= 0x35,
		Command_Seek							= 0x29,
		Command_Read_Drive_Status				= 0x2c,
		Command_Read_Special_Reg				= 0x3D,
		Command_Write_Special_Reg				= 0x3A,

		Command_Format							= 0x23,
		Command_Read_ID							= 0x1B,

		Command_128byte_Read_Data				= 0x12,
		Command_128byte_Read_Data_And_Deleted	= 0x16,
		Command_128byte_Write_Data				= 0x0a,
		Command_128byte_Write_Deleted			= 0x0e,
		Command_128byte_Verify_Data_And_Deleted	= 0x1e,

		Command_Var_Read_Data					= 0x13,
		Command_Var_Read_Data_And_Deleted		= 0x17,
		Command_Var_Write_Data					= 0x0b,
		Command_Var_Write_Deleted				= 0x0f,
		Command_Var_Verify_Data_And_Deleted		= 0x1f,

		Command_Var_Scan_Data					= 0x00,
		Command_Var_Scan_Data_And_Deleted		= 0x04,

		Command_Initialization					= 0x100,
		Command_Load_Bad_Tracks					= 0x101,
	};

	void Tick();
	void ExecuteCommand();

	//-------------------------------------------------------------------------------------------------
	EPhase m_currentPhase;

	ECommand m_uCurrentCommand;
	u8 m_uCommandDrive;
	u8 m_uStatusRegister;
	u8 m_uResultRegister;
	u8 m_uParameterRegister;
	u8 m_uParameters[8];
	u8 m_uNumParameters;
	u8 m_uNumParametersRequired;
	u8 m_uCurrentSector[2];
	u32 m_nNumBytesToTransfer;
	FloppyDisk* m_disk[ 2 ];
	u32 m_nReadWriteOffset;

	//
	// Initialization Params
	//
	int m_nStepRate			;
	int m_nHeadSettlingTime ;
	int m_nIndexCount		;
	int m_nHeadLoadTime		;

	//
	// Special Registers
	//
	u8 m_uCurrentTrack[2];
	u8 m_uScanSectorNumber;
	u16 m_uScanCount;
	u8 m_uModeRegister;
	u8 m_uDriveControlOutputPort;
	u8 m_uDriveControlInputPort;
	u8 m_nSurface0BadTrack1;
	u8 m_nSurface0BadTrack2;
	u8 m_nSurface1BadTrack1;
	u8 m_nSurface1BadTrack2;
};

//-------------------------------------------------------------------------------------------------
