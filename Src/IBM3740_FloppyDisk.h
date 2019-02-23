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
	int					m_nNumSides;

	u8					Read( int track, int sector, int offset, int side );
	void				Write( u8 value, int track, int sector, int offset, int side );
	void				FlushWrites();
private:
	int					m_nSize;
	std::string			m_filename;
	std::vector< u8 >	m_data;
	bool				m_bNeedsFlush;
	char				m_volumeTitle[12];
};

//-------------------------------------------------------------------------------------------------
