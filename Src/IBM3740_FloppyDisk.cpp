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
	m_nMaxModifyOffset = -1;

	u32 nSize = m_nNumSectorsPerTrack * m_nNumTracks * m_nSectorSize * m_nNumSides;
	m_data.resize( nSize );

	CFile file( m_filename, "rb" );
	u32 nDiskSize = min( nSize, file.GetLength() );
	file.Read( m_data.data(), nDiskSize );
}

//-------------------------------------------------------------------------------------------------

FloppyDisk::~FloppyDisk()
{
	CFile file( m_filename, "wb" );
	file.Write( m_data.data(), m_nMaxModifyOffset );
}

//-------------------------------------------------------------------------------------------------

u8 FloppyDisk::Read( int track, int sector, int offset, int side )
{
	int nAddress = ( ( side * m_nNumTracks + track ) * m_nNumSectorsPerTrack + sector ) * m_nSectorSize + offset;
	return m_data[ nAddress ];
}

//-------------------------------------------------------------------------------------------------

void FloppyDisk::Write( u8 value, int track, int sector, int offset, int side )
{
	int nAddress = ( ( side * m_nNumTracks + track ) * m_nNumSectorsPerTrack + sector ) * m_nSectorSize + offset;
	m_data[ nAddress ] = value;
	if ( nAddress > m_nMaxModifyOffset )
	{
		m_nMaxModifyOffset = nAddress;
	}
}

//-------------------------------------------------------------------------------------------------
