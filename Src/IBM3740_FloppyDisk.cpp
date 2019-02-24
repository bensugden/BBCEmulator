//-------------------------------------------------------------------------------------------------
//
// IBM 3740 Format Floppy
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------

FloppyDisk::FloppyDisk( const std::string& filename )
	: m_filename( filename )
{
	m_nNumSectorsPerTrack = 10;
	m_nNumTracks = 80;
	m_nSectorSize = 256;
	m_nNumSides = 1;

	m_data.resize( m_nNumSectorsPerTrack * m_nNumTracks * m_nSectorSize * m_nNumSides );

	m_bNeedsFlush = false;
	CFile file( m_filename, "rb" );
	m_nSize = min( m_data.size(), file.GetLength() );
	file.Read( m_data.data(), m_nSize );
}

//-------------------------------------------------------------------------------------------------

FloppyDisk::~FloppyDisk()
{
	FlushWrites();
}

//-------------------------------------------------------------------------------------------------

void FloppyDisk::FlushWrites()
{
	if ( m_bNeedsFlush )
	{
		CFile file( m_filename, "wb" );
		file.Write( m_data.data(), m_nSize );
		m_bNeedsFlush = false;
	}
}

//-------------------------------------------------------------------------------------------------

u8 FloppyDisk::Read( int track, int sector, int offset, int side )
{
	int nAddress = ( ( side * m_nNumTracks + track ) * m_nNumSectorsPerTrack + sector ) * m_nSectorSize + offset;
	assert( nAddress >=0 && nAddress < (int)m_data.size() );
	return m_data[ nAddress ];
}

//-------------------------------------------------------------------------------------------------

void FloppyDisk::Write( u8 value, int track, int sector, int offset, int side )
{
	int nAddress = ( ( side * m_nNumTracks + track ) * m_nNumSectorsPerTrack + sector ) * m_nSectorSize + offset;
	m_data[ nAddress ] = value;
	if ( nAddress > m_nSize )
	{
		m_nSize = nAddress;
	}
	assert( nAddress >=0 && nAddress < (int)m_data.size() );

	m_bNeedsFlush = true;
}

//-------------------------------------------------------------------------------------------------
