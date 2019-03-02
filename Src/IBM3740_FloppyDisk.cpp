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

	m_data.resize( m_nNumSectorsPerTrack * m_nNumTracks * m_nSectorSize );

	m_bNeedsFlush = false;
	CFile file( m_filename, "rb" );
	m_nSize = min( (int)m_data.size(), file.GetLength() );
	file.Read( m_data.data(), m_nSize );

	m_badTrack[ 0 ] = 0xff;
	m_badTrack[ 1 ] = 0xff;
}

//-------------------------------------------------------------------------------------------------

FloppyDisk::~FloppyDisk()
{
	FlushWrites();
}

//-------------------------------------------------------------------------------------------------

void FloppyDisk::SetBadTrack( int index, int track )
{
	m_badTrack[ index ] = track;
}

//-------------------------------------------------------------------------------------------------

int FloppyDisk::GetPhysicalTrack( int track )
{
  int offset=0;

  if (m_badTrack[ 0 ] <= track) 
	  offset++;
  if (m_badTrack[ 1 ] <= track) 
	  offset++;
    
  return ( track + offset );
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

void FloppyDisk::Read( u8* buffer, int track, int sector, int numBytes )
{
	track = GetPhysicalTrack( track );
	int nAddress = ( track * m_nNumSectorsPerTrack + sector ) * m_nSectorSize;
	assert( nAddress >=0 && nAddress < (int)m_data.size() );
	memcpy( buffer, &m_data[ nAddress ], numBytes );
}

//-------------------------------------------------------------------------------------------------

void FloppyDisk::Write( u8 value, int track, int sector )
{
	track = GetPhysicalTrack( track );
	int nAddress = ( track * m_nNumSectorsPerTrack + sector ) * m_nSectorSize;
	m_data[ nAddress ] = value;
	if ( nAddress > m_nSize )
	{
		m_nSize = nAddress;
	}
	assert( nAddress >=0 && nAddress < (int)m_data.size() );

	m_bNeedsFlush = true;
}

//-------------------------------------------------------------------------------------------------
