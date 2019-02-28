#pragma once

//-------------------------------------------------------------------------------------------------
//
// Floppy Disk
//
//-------------------------------------------------------------------------------------------------

class FloppyDisk
{
public:
	FloppyDisk( const std::string& filename );
	~FloppyDisk();

	int					m_nNumTracks;
	int					m_nNumSectorsPerTrack;
	int					m_nSectorSize;

	void				Read( u8* buffer, int track, int sector, int bytes );
	void				Write( u8 value, int track, int sector );
	void				FlushWrites();
	void				SetBadTrack( int index, int track );
private:
	int					GetPhysicalTrack( int track );
	int					m_nSize;
	std::string			m_filename;
	std::vector< u8 >	m_data;
	int					m_badTrack[2];
	bool				m_bNeedsFlush;
	char				m_volumeTitle[12];
};

//-------------------------------------------------------------------------------------------------
