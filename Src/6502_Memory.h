#pragma once

//-------------------------------------------------------------------------------------------------
//
// RAM For 6502 emulator
//
//-------------------------------------------------------------------------------------------------

extern CPU cpu;

//-------------------------------------------------------------------------------------------------
//
// Lets you pass in a member fn pointer as a memory map handler rather than just a function ptr
//
//-------------------------------------------------------------------------------------------------

#define MemoryMapHandler( function ) std::bind( &function, this, _1, _2 )

//-------------------------------------------------------------------------------------------------

struct MemoryState
{
	MemoryState( u16 nUserMemory = 32768, u32 nAllocation = 65536 )
		: m_nEndUserMemory( nUserMemory )
		, m_maxAllocatedMemory( nAllocation )
		, m_bReadBreakpointSet( false )
		, m_bWriteBreakpointSet( false )
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

	void RegisterMemoryMappedAddress( u16 address, std::function<u8(u16,u8)> listenerFunction )
	{
		assert( m_nNumMemMappedAddresses < 255 );
		int nAllocatedSlot = -1;

		//
		// Function already registered?
		//
		for ( int i = 0; i < m_nNumMemMappedAddresses; i++ )
		{
			if ( m_pMemoryMappedCallback[ i ] = listenerFunction )
			{
				nAllocatedSlot = i;
				break;
			}
		}

		//
		// Need to allocate new function ptr slot
		//
		if ( nAllocatedSlot == -1 )
		{
			m_pMemoryMappedCallback[ ++m_nNumMemMappedAddresses ] = listenerFunction; // 0 will indicate no listener registered for this memory location
			nAllocatedSlot = m_nNumMemMappedAddresses;
		}
		m_pMemoryMapID[ address - m_nEndUserMemory ] = nAllocatedSlot;
	}
	
	//-------------------------------------------------------------------------------------------------

	inline void CheckWriteMemoryMapped( u16 nAddress, u8 value )
	{
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
		// Call system that is memory mapped to this address and update if it modifies read value
		//
		m_pMemory[ nAddress ] = m_pMemoryMappedCallback[ nMemoryMapID ]( nAddress, value );
	}

	//-------------------------------------------------------------------------------------------------

	inline void CheckReadBreakpoint( u16 address ) const
	{
		if ( !m_bReadBreakpointSet )
			return;
		if ( m_nReadBreakpoint == address )
			cpu.ThrowBreakpoint( std::string("Break on read at :") + Utils::toHex(address));
	}

	//-------------------------------------------------------------------------------------------------

	inline void CheckWriteBreakpoint( u16 address, u8 value ) const
	{
		if ( !m_bWriteBreakpointSet )
			return;
		if ( m_nWriteBreakpoint == address )
		{
			if ( ( !m_bWriteBreakpointValueSet ) || ( m_nWriteBreakpointValue == value ) )
			{
				cpu.ThrowBreakpoint( std::string("Break on write at :") +  Utils::toHex(address) +" with value :" +  Utils::toHex(value) );
			}
		}
	}

	//-------------------------------------------------------------------------------------------------

	inline u8 Read_Internal( int nAddress ) const
	{
		// No breakpoints - this is for internal (ie hw/debugger) memory access only 
		return m_pMemory[ nAddress ];
	}
	//-------------------------------------------------------------------------------------------------

	inline u8 Read( int nAddress ) const
	{
		CheckReadBreakpoint( nAddress );
		return m_pMemory[ nAddress ];
	}
	//-------------------------------------------------------------------------------------------------

	inline u16 ReadAddress( int nAddress ) const
	{
		return Utils::MakeAddress( Read( nAddress + 1 ), Read( nAddress + 0 ) );
	}
	//-------------------------------------------------------------------------------------------------

	inline void Write( u16 nAddress, u8 value )
	{
		m_pMemory[ nAddress ] = value;
		CheckWriteMemoryMapped( nAddress, value );
		CheckWriteBreakpoint( nAddress, m_pMemory[ nAddress ] );
	}

	//-------------------------------------------------------------------------------------------------

	inline void WriteHiByte( u16 nAddress, u16 value )
	{
		m_pMemory[ nAddress ] = ( value >> 8) & 0xff;
		CheckWriteMemoryMapped( nAddress, m_pMemory[ nAddress ] );
		CheckWriteBreakpoint( nAddress, m_pMemory[ nAddress ] );
	}

	//-------------------------------------------------------------------------------------------------

	inline void WriteLoByte( u16 nAddress, u16 value )
	{
		m_pMemory[ nAddress ] = value & 0xff;
		CheckWriteMemoryMapped( nAddress, m_pMemory[ nAddress ] );
		CheckWriteBreakpoint( nAddress, m_pMemory[ nAddress ] );
	}

	//-------------------------------------------------------------------------------------------------

	inline u16 ReadHiByte( u16 nAddress, u16& valueToModify ) const
	{
		u8 readValue = Read( nAddress );
		valueToModify &= (0xffff00ff); 
		valueToModify |= ( readValue & 0xff ) << 8;
		CheckReadBreakpoint( nAddress );
		return valueToModify;
	}

	//-------------------------------------------------------------------------------------------------

	inline u16 ReadLoByte( u16 nAddress, u16& valueToModify ) const
	{
		u8 readValue = Read( nAddress );
		valueToModify &= (0xffffff00); 
		valueToModify |= readValue & 0xff;
		CheckReadBreakpoint( nAddress );
		return valueToModify;
	}

	//-------------------------------------------------------------------------------------------------

	void LoadROM( string filename, u16 nAddress )
	{
		CFile file( filename, "rb" );
		file.Load( m_pMemory + nAddress, file.GetLength(), 1 );
	}

	//-------------------------------------------------------------------------------------------------

	void Clear( u16 start, u16 end )
	{
		for ( u16 i = start; i < end; i++ )
		{
			Write( i, 0 );
		}
	}

	//-------------------------------------------------------------------------------------------------

	void SetReadBreakpoint( u16 address )
	{
		m_bReadBreakpointSet = true;
		m_nReadBreakpoint = address;
	}
	//-------------------------------------------------------------------------------------------------

	void SetWriteBreakpoint( u16 address, int value )
	{
		m_bWriteBreakpointSet = true;
		m_bWriteBreakpointValueSet = (value != -1);
		m_nWriteBreakpoint = address;
		m_nWriteBreakpointValue = (u8)value;
	}

	//-------------------------------------------------------------------------------------------------

	void ClearReadBreakpoint()
	{
		m_bReadBreakpointSet = false;
	}

	//-------------------------------------------------------------------------------------------------

	void ClearWriteBreakpoint()
	{
		m_bWriteBreakpointSet = false;
	}
	//-------------------------------------------------------------------------------------------------

	void DumpMemoryToString( u16 address, int xcolumns, int yrows, string& output );

	//-------------------------------------------------------------------------------------------------

	u8*			m_pMemory;
	u16			m_nEndUserMemory;		// everything up to this point is guaranteed to NOT be memory mapped
	u32			m_maxAllocatedMemory; 
	u8*			m_pMemoryMapID;
	std::function<u8(u16,u8)> m_pMemoryMappedCallback[256];
	u8			m_nNumMemMappedAddresses;
	bool		m_bReadBreakpointSet;
	bool		m_bWriteBreakpointSet;
	bool		m_bWriteBreakpointValueSet;
	u16			m_nReadBreakpoint;
	u16			m_nWriteBreakpoint;
	u8			m_nWriteBreakpointValue;
};

//-------------------------------------------------------------------------------------------------
