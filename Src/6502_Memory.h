#pragma once

//-------------------------------------------------------------------------------------------------
//
// Here's our RAM
//
//-------------------------------------------------------------------------------------------------

struct MemoryState
{
	MemoryState( u16 nUserMemory = 32768, u32 nAllocation = 65536 )
		: m_nEndUserMemory( nUserMemory )
		, m_maxAllocatedMemory( nAllocation )
	{
		m_pMemory = new u8[ nAllocation ];
		memset( m_pMemory, 0, nAllocation );

		//
		// This is a list of IDs for memory mapped memory callbacks
		//
		m_pMemoryMapID = new u8[ nAllocation-nUserMemory ];
		memset( m_pMemoryMapID, 0, nAllocation-nUserMemory );
		m_nNumMemMappedAddresses = 0; //[0 indicates no listener]

		memset( m_pMemoryMapID, 0, sizeof( m_pMemoryMapID )) ;

		// todo - set up default memory values for 6502
	}

	//-------------------------------------------------------------------------------------------------
	void RegisterMemoryMappedAddress( u16 address, void (*listenerFunction)( u16 address, u8 value ) )
	{
		assert( m_nNumMemMappedAddresses < 255 );
		m_pMemoryMappedCallback[ ++m_nNumMemMappedAddresses ] = listenerFunction; // 0 will indicate no listener registered for this memory location
		m_pMemoryMapID[ address - m_nEndUserMemory ] = m_nNumMemMappedAddresses;
	}

	//-------------------------------------------------------------------------------------------------

	inline u8 Read( int nAddress ) const
	{
		return m_pMemory[ nAddress ];
	}

	//-------------------------------------------------------------------------------------------------

	inline void CheckWriteMemoryMapped( u16 nAddress, u8 value )
	{
		assert( nAddress != 0x355 );
		//
		// Is this write in user memory ? If so, not memory mapped.
		//
		if ( nAddress < m_nEndUserMemory )
			return;
		//
		// Check if a system registered this address as a memory mapped address
		//
		u8 nMemoryMapID = m_pMemoryMapID[ nAddress - m_nEndUserMemory ];
		if ( nMemoryMapID == 0 )
			return;

		// 
		// Call system that is memory mapped to this address
		//
		m_pMemoryMappedCallback[ nMemoryMapID ]( nAddress, value );
	}

	//-------------------------------------------------------------------------------------------------

	inline void Write( u16 nAddress, u8 value )
	{
		m_pMemory[ nAddress ] = value;
		CheckWriteMemoryMapped( nAddress, value );
	}

	//-------------------------------------------------------------------------------------------------

	inline void WriteHiByte( u16 nAddress, u16 value )
	{
		m_pMemory[ nAddress ] = ( value >> 8) & 0xff;
		CheckWriteMemoryMapped( nAddress, m_pMemory[ nAddress ] );
	}

	//-------------------------------------------------------------------------------------------------

	inline void WriteLoByte( u16 nAddress, u16 value )
	{
		m_pMemory[ nAddress ] = value & 0xff;
		CheckWriteMemoryMapped( nAddress, m_pMemory[ nAddress ] );
	}

	//-------------------------------------------------------------------------------------------------

	inline u16 ReadHiByte( u16 nAddress, u16& valueToModify ) const
	{
		u8 readValue = Read( nAddress );
		valueToModify &= (0xffff00ff); 
		valueToModify |= ( readValue & 0xff ) << 8;
		return valueToModify;
	}

	//-------------------------------------------------------------------------------------------------

	inline u16 ReadLoByte( u16 nAddress, u16& valueToModify ) const
	{
		u8 readValue = Read( nAddress );
		valueToModify &= (0xffffff00); 
		valueToModify |= readValue & 0xff;
		return valueToModify;
	}

	//-------------------------------------------------------------------------------------------------

	void LoadROM( string filename, u16 nAddress )
	{
		CFile file( filename, "rb" );
		file.Load( m_pMemory + nAddress, file.GetLength(), 1 );
	}

	//-------------------------------------------------------------------------------------------------

	u8*			m_pMemory;
	u16			m_nEndUserMemory;		// everything up to this point is guaranteed to NOT be memory mapped
	u32			m_maxAllocatedMemory; 
	u8*			m_pMemoryMapID;
	void		(*m_pMemoryMappedCallback[256])( u16 address, u8 value );
	u8			m_nNumMemMappedAddresses;
};

//-------------------------------------------------------------------------------------------------
