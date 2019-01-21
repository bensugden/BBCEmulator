//==============================================================================

#include "stdafx.h"
#include "File.h"
#include "stdio.h"

//==============================================================================
//
//  CFile Class
//
//==============================================================================


CFile::CFile( const std::string& filename, const char* access ) : m_pFile( NULL )
{
#ifdef WIN32
	fopen_s( &m_pFile, ( std::string( APP_ROOT ) + filename ).c_str(), access );
	m_fileName = std::string( APP_ROOT ) + filename;
#else
	char temp_file_name[ 2048 ];
	ConvertFilePath( filename.c_str(), temp_file_name );
	m_pFile = fopen( temp_file_name, access );
	m_fileName = temp_file_name;
#endif
	if ( !m_pFile )
	{
		assert_msg( false, "file not found" );
	}

	m_buffer = NULL;
	m_bufferLength = 0;
} 

// ------------------------------------------------------------------------------
//
//  destructor
//

CFile::~CFile( )
{
	if ( m_pFile )
	{
		fclose( m_pFile );
	}
	delete [] m_buffer;
}

// ------------------------------------------------------------------------------

u32 CFile::GetLength()
{
	if (!m_pFile)
		return 0;

	fseek( m_pFile, 0, SEEK_END );
	u32 bufferLength = ftell(m_pFile); 
	fseek( m_pFile, 0, SEEK_SET );

	return bufferLength;
}

//-------------------------------------------------------------------------------------------------

bool CFile::IsFileOlder( const std::string& filenameIsThisOlder, const std::string& filenameThanThis )
{
#ifdef WIN32
	HANDLE hFile1;
	HANDLE hFile2;
	FILETIME ftCreate1, ftAccess1, ftWrite1;
	FILETIME ftCreate2, ftAccess2, ftWrite2;

	// Opening the existing file
	hFile1 = CreateFileA(filenameIsThisOlder.c_str(),
		GENERIC_READ, //open for reading
		FILE_SHARE_READ, //share for reading
		NULL, //default security
		OPEN_EXISTING, //existing file only
		FILE_ATTRIBUTE_NORMAL, //normal file
		NULL); //no attribute template

	if ( hFile1 == INVALID_HANDLE_VALUE )
		return true;

	hFile2 = CreateFileA(filenameThanThis.c_str(),
		GENERIC_READ, //open for reading
		FILE_SHARE_READ, //share for reading
		NULL, //default security
		OPEN_EXISTING, //existing file only
		FILE_ATTRIBUTE_NORMAL, //normal file
		NULL); //no attribute template
	
	if ( hFile2 == INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile1 );
		return true;	
	}

	GetFileTime(hFile1, &ftCreate1, &ftAccess1, &ftWrite1);
	GetFileTime(hFile2, &ftCreate2, &ftAccess2, &ftWrite2);

	CloseHandle( hFile1 );
	CloseHandle( hFile2 );

	if ( ftWrite1.dwHighDateTime > ftWrite2.dwHighDateTime )
		return false;
	if (( ftWrite1.dwHighDateTime == ftWrite2.dwHighDateTime )&&
		 ( ftWrite1.dwLowDateTime >= ftWrite2.dwLowDateTime ))
		return false;
#endif
	
	return true;
}

// ------------------------------------------------------------------------------

void CFile::Load( )
{
	//
	// make this asynchronous
	//
	m_bufferLength = GetLength();

	m_buffer = new char[ m_bufferLength + 1 ];
	fread( m_buffer, 1, m_bufferLength, m_pFile );
	m_buffer[ m_bufferLength ] = 0;
}

// ------------------------------------------------------------------------------

void* CFile::GetBuffer( )
{
	//
	// add : stall if async
	//
	if ( m_bufferLength == 0 )
	{
		Load();
	}

	return m_buffer;
}

// ------------------------------------------------------------------------------

u32 CFile::GetBufferLength( )
{
	//
	// add : stall if async
	//
	if ( m_bufferLength == 0 )
	{
		Load();
	}

	return m_bufferLength;
}

// ------------------------------------------------------------------------------

void CFile::SetPosition( u32 pos )
{
	fseek( m_pFile, pos, SEEK_SET );
}

// ------------------------------------------------------------------------------

u32 CFile::GetPosition( )
{
	return ftell(m_pFile);
}

// ------------------------------------------------------------------------------

void CFile::Read( void* dest, u32 bytes )
{
	fread( dest, bytes, 1, m_pFile );
}

// ------------------------------------------------------------------------------

void CFile::Write( const void* src, u32 bytes )
{
	fwrite( src, bytes, 1, m_pFile );
}

// ------------------------------------------------------------------------------

bool CFile::IsLoaded( )
{
	return m_bufferLength != 0;
}

// ------------------------------------------------------------------------------

void CFile::Load( u32& data )
{
	fread( &data, sizeof( u32 ), 1, m_pFile );
	CFile::UnEnd4( data );
}

// ------------------------------------------------------------------------------

void CFile::Load( s32& data )
{
	fread( &data, sizeof( s32 ), 1, m_pFile );
	CFile::UnEnd4( *((u32*)(&data)) );
}

// ------------------------------------------------------------------------------

void CFile::Load( u8& data )
{
	fread( &data, sizeof( u8 ), 1, m_pFile );
}
// ------------------------------------------------------------------------------

void CFile::Load( float& data )
{
	fread( &data, sizeof( float ), 1, m_pFile );
	CFile::UnEnd4( *((u32*)(&data)) );
}

// ------------------------------------------------------------------------------

void CFile::Load( u16& data )
{
	fread( &data, sizeof( u16 ), 1, m_pFile );
	CFile::UnEnd2( data );
}

// ------------------------------------------------------------------------------

void CFile::Load( char*& str )
{
	u32 length;
	Load( length );

	str = new char[ length + 1 ];
	fread( str, 1, length, m_pFile );
	str[ length ] = 0;
}

// ------------------------------------------------------------------------------

void CFile::Load( void* pData, u32 size, u32 uBlockSize )
{
	fread( pData, size, 1, m_pFile );
	size /= uBlockSize;
	
	if ( uBlockSize==4 )
	{
		for ( u32 u = 0; u < size; u++ )
		{
			UnEnd4( (((u32*)pData)[ u ]) );
		}
	}
	else
	if ( uBlockSize==2 )
	{
		for ( u32 u = 0; u < size; u++ )
		{
			UnEnd2( (((u16*)pData)[ u ]) );
		}
	}
}

// ------------------------------------------------------------------------------

bool CFile::FileExists( const std::string& filename)
{
#ifdef _WIN32
	u32 file_attributes = GetFileAttributesA(filename.c_str());
	if (file_attributes == 0xffffffff || // file is missing
		file_attributes == 0x10)		 // file is a directory
		return false;
#else
	assert(false);// not implemented
#endif
	return true;
}

// ------------------------------------------------------------------------------
//  @} 
