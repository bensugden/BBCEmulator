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

struct MemoryMap
{
	MemoryMap( u32 memoryMapSize, u16 startAddress )
		: m_memoryMapSize( memoryMapSize )
		, m_startAddress( startAddress )
	{
		//
		// This is a list of IDs for memory mapped memory callbacks
		//
		m_pMemoryMapID = new u8[ memoryMapSize ];
		memset( m_pMemoryMapID, 0, memoryMapSize );
		m_nNumMemMappedAddresses = 0; //[0 indicates no listener]
	}
	
	//-------------------------------------------------------------------------------------------------

	~MemoryMap()
	{
		delete [] m_pMemoryMapID;
	}

	//-------------------------------------------------------------------------------------------------

	void RegisterMemoryMap_Write( u16 address, std::function<u8(u16,u8)> listenerFunction )
	{
		assert( m_nNumMemMappedAddresses < 255 );
		//
		// Need to allocate new function ptr slot
		//
		u8 nID = m_pMemoryMapID[ address - m_startAddress ];

		if ( nID == 0 )	// if ID != 0 then we already registered handler for this address.
		{ 
			nID = ++m_nNumMemMappedAddresses;
		}

		m_pMemoryMappedCallback[ nID ] = listenerFunction; // 0 will indicate no listener registered for this memory location
		m_pMemoryMapID[ address - m_startAddress ] = nID;
	}

	//-------------------------------------------------------------------------------------------------

	inline u8 CheckMemoryMapped( u16 nAddress, u8 value )
	{
		//
		// Is this write in user memory ? If so, not memory mapped.
		//
		if ( nAddress < m_startAddress )
			return value;
		//
		// Check if a system registered this address as a memory mapped address
		//
		u8 nMemoryMapID = m_pMemoryMapID[ nAddress - m_startAddress ];
		if ( nMemoryMapID == 0 )
			return value;

		// 
		// Call system that is memory mapped to this address and update if it modifies read value
		//
		return m_pMemoryMappedCallback[ nMemoryMapID ]( nAddress, value );
	}

	//-------------------------------------------------------------------------------------------------

	u8*			m_pMemoryMapID;
	std::function<u8(u16,u8)> m_pMemoryMappedCallback[256];
	u8			m_nNumMemMappedAddresses;
	u32			m_memoryMapSize;
	u16			m_startAddress;
};

//-------------------------------------------------------------------------------------------------

struct MemoryState
{
	MemoryState( u16 nUserMemory = 32768, u32 nAllocation = 65536, u16 pageSize = 16384, u16 maxPagedROMS = 16 )
		: m_nEndUserMemory( nUserMemory )
		, m_maxAllocatedMemory( nAllocation )
		, m_bReadBreakpointSet( false )
		, m_bWriteBreakpointSet( false )
		, m_writeMemoryMap( nAllocation -  nUserMemory, nUserMemory )
		, m_readMemoryMap( nAllocation -  nUserMemory, nUserMemory )
		, m_nMaxPagedRoms( maxPagedROMS )
		, m_nPageSize( pageSize )
	{
		//
		// Pre-allocate 1 page of memory per page. Future ones can be allocated ad-hoc
		//
		u32 nNumPages = nAllocation / pageSize;
		m_pPageAddresses = new u8**[ nNumPages ];
		m_pActivePageAddress = new u8*[ nNumPages ];

		for ( u32 page = 0; page < nNumPages; page++ )
		{ 
			m_pPageAddresses[ page ] = new u8*[ maxPagedROMS ];
			
			m_pPageAddresses[ page ][ 0 ] = new u8[ pageSize ];
			m_pActivePageAddress[ page ] = m_pPageAddresses[ page ][ 0 ];

			memset( m_pPageAddresses[ page ][ 0 ], 0, pageSize );

			for ( u32 rom = 1; rom < maxPagedROMS; rom++ )
			{
				m_pPageAddresses[ page ][ rom ] = nullptr;
			}
		}

		m_nPageSizeShift = 0;
		while ( ( 1 << m_nPageSizeShift ) < pageSize )
		{
			m_nPageSizeShift ++;
		}

		// todo - set up default memory values for 6502
	}
	//-------------------------------------------------------------------------------------------------

	~MemoryState()
	{
		u32 nNumPages = m_maxAllocatedMemory / m_nPageSize;
		for ( u32 page = 0; page < nNumPages; page++ )
		{ 
			for ( u32 rom = 0; rom < m_nMaxPagedRoms; rom++ )
			{
				delete [] m_pPageAddresses[ page ][ rom ] ;
			}
			delete [] m_pPageAddresses[ page ];
		}
		delete [] m_pPageAddresses;
	}

	//-------------------------------------------------------------------------------------------------

	void EnsureMemoryAllocated( u16 address, u8 romID )
	{
		u16 page = address >> m_nPageSizeShift;
		if ( m_pPageAddresses[ page ][ romID ] != nullptr )
			return;
		m_pPageAddresses[ page ][ romID ] = new u8[ m_nPageSize ];
		memset( m_pPageAddresses[ page ][ romID ], 0, m_nPageSize );
	}

	//-------------------------------------------------------------------------------------------------

	void SelectRomForAddress( u16 address, u8 romID )
	{
		EnsureMemoryAllocated( address, romID );
		u16 page = address >> m_nPageSizeShift;
		m_pActivePageAddress[ page ] = m_pPageAddresses[ page ][ romID ];
		assert( m_pActivePageAddress[ page ] );
	}

	//-------------------------------------------------------------------------------------------------

	inline u8* TranslateAddress( u16 address, u8 romID ) const
	{
		u16 inPageAddress = address & ( m_nPageSize - 1 );
		u16 page = address >> m_nPageSizeShift;
		return m_pPageAddresses[ page ][ romID ] + inPageAddress ;
	}

	//-------------------------------------------------------------------------------------------------

	inline u8& Memory( u16 address ) const
	{
		u16 inPageAddress = address & ( m_nPageSize - 1 );
		u16 page = address >> m_nPageSizeShift;
		return m_pActivePageAddress[ page ][ inPageAddress ];
	}

	//-------------------------------------------------------------------------------------------------

	void RegisterMemoryMap_Write( u16 address, std::function<u8(u16,u8)> listenerFunction )
	{
		m_writeMemoryMap.RegisterMemoryMap_Write( address, listenerFunction );
	}

	//-------------------------------------------------------------------------------------------------

	void RegisterMemoryMap_Read( u16 address, std::function<u8(u16,u8)> listenerFunction )
	{
		m_readMemoryMap.RegisterMemoryMap_Write( address, listenerFunction );
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
		return Memory( nAddress );
	}
	//-------------------------------------------------------------------------------------------------

	inline u8 Read( int nAddress )
	{
		CheckReadBreakpoint( nAddress );
		return m_readMemoryMap.CheckMemoryMapped( nAddress, Memory( nAddress ) );

	}
	//-------------------------------------------------------------------------------------------------

	inline u16 ReadAddress( int nAddress )
	{
		return Utils::MakeAddress( Read( nAddress + 1 ), Read( nAddress + 0 ) );
	}
	//-------------------------------------------------------------------------------------------------

	inline void Write( u16 nAddress, u8 value )
	{
		Memory( nAddress ) = m_writeMemoryMap.CheckMemoryMapped( nAddress, value );
		CheckWriteBreakpoint( nAddress, Memory( nAddress ) );
	}

		//-------------------------------------------------------------------------------------------------

	inline void Write_Internal( u16 nAddress, u8 value )
	{
		// Do not check mem maps etc. - this is for internal (ie hw/debugger) memory access only
		Memory( nAddress ) = value;
		CheckWriteBreakpoint( nAddress, Memory( nAddress ) );
	}

	//-------------------------------------------------------------------------------------------------

	inline void WriteHiByte( u16 nAddress, u16 value )
	{
		Memory( nAddress ) = m_writeMemoryMap.CheckMemoryMapped( nAddress, ( value >> 8) & 0xff );
		CheckWriteBreakpoint( nAddress, Memory( nAddress ) );
	}

	//-------------------------------------------------------------------------------------------------

	inline void WriteLoByte( u16 nAddress, u16 value )
	{
		Memory( nAddress ) = m_writeMemoryMap.CheckMemoryMapped( nAddress, value & 0xff );
		CheckWriteBreakpoint( nAddress, Memory( nAddress ) );
	}

	//-------------------------------------------------------------------------------------------------

	inline u16 ReadHiByte( u16 nAddress, u16& valueToModify )
	{
		u8 readValue = Read( nAddress );
		valueToModify &= (0xffff00ff); 
		valueToModify |= ( readValue & 0xff ) << 8;
		CheckReadBreakpoint( nAddress );
		return valueToModify;
	}

	//-------------------------------------------------------------------------------------------------

	inline u16 ReadLoByte( u16 nAddress, u16& valueToModify )
	{
		u8 readValue = Read( nAddress );
		valueToModify &= (0xffffff00); 
		valueToModify |= readValue & 0xff;
		CheckReadBreakpoint( nAddress );
		return valueToModify;
	}

	//-------------------------------------------------------------------------------------------------

	void LoadROM( string filename, u16 nAddress, u8 romID = 0 )
	{
		EnsureMemoryAllocated( nAddress, romID );
		CFile file( filename, "rb" );
		assert( file.GetLength() <= m_nPageSize ); // not supported > page size loading
		file.Load( TranslateAddress( nAddress, romID ), file.GetLength(), 1 );
	}

	//-------------------------------------------------------------------------------------------------

	void Clear( u16 start, u16 end )
	{
		for ( u16 i = start; i < end; i++ )
		{
			Write_Internal( i, 0 );
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

	u32	GetAllocatedMemorySize() const { return m_maxAllocatedMemory; }

	//-------------------------------------------------------------------------------------------------

	u16			m_nEndUserMemory;		// everything up to this point is guaranteed to NOT be memory mapped
	u32			m_maxAllocatedMemory;
	MemoryMap	m_writeMemoryMap;
	MemoryMap	m_readMemoryMap;
	bool		m_bReadBreakpointSet;
	bool		m_bWriteBreakpointSet;
	bool		m_bWriteBreakpointValueSet;
	u16			m_nReadBreakpoint;
	u16			m_nWriteBreakpoint;
	u8			m_nWriteBreakpointValue;
	u16			m_nMaxPagedRoms;
	u8***		m_pPageAddresses;
	u8**		m_pActivePageAddress;
	u16			m_nPageSizeShift;
	u16			m_nPageSize;
};

//-------------------------------------------------------------------------------------------------
