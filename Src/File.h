//==============================================================================
//
//  \file       CFile.h 
//              Created: 5/15/2006
//              Author: bsugden
//
//==============================================================================

#pragma once
#define APP_ROOT ""

// ------------------------------------------------------------------------------

#include "stdio.h"

//------------------------------------------------------------------------------

class CFile
{
public:
	CFile(  const std::string& filename, const char* access );
	// delete file to close it
	~CFile();

	void 		Load( );

	void 		Load( u32& data );
	void 		Load( s32& data );
	void 		Load( u8& data );
	void 		Load( float& data );
	void 		Load( u16& data );
	void		Load( char*& str );
	void 		Load( void* pData, u32 size, u32 uBlockSize );

	u32			GetLength();

	void*		GetBuffer(); // stalls if not loaded
	u32			GetBufferLength();
	bool		IsLoaded();

	void 		Read ( void*       dest, u32 bytes );
	void 		Write( const void* src , u32 bytes );
		
	void 		SetPosition( u32 pos );
	u32			GetPosition( );

	std::string GetFilename( ) { return m_fileName; }

	static bool FileExists( const std::string& filename);
	static bool IsFileOlder( const std::string& filenameIsThisOlder, const std::string& filenameThanThis );
	//-------------------------------------------------------------------------------------------------

private:
	static void UnEnd4( u32& v )
	{
#ifdef _XBOX
		BYTE* pValue = (BYTE*)&v;
		BYTE b[4];
		b[ 0 ] = pValue[ 3 ];
		b[ 1 ] = pValue[ 2 ];
		b[ 2 ] = pValue[ 1 ];
		b[ 3 ] = pValue[ 0 ];
		pValue[ 3 ] = b[ 3 ];
		pValue[ 2 ] = b[ 2 ];	
		pValue[ 1 ] = b[ 1 ];
		pValue[ 0 ] = b[ 0 ];
#endif
	}

	static void UnEnd2( u16& v )
	{
#ifdef _XBOX
		BYTE* pValue = (BYTE*)&v;
		BYTE b[2];
		b[ 0 ] = pValue[ 1 ];
		b[ 1 ] = pValue[ 0 ];
		pValue[ 1 ] = b[ 1 ];
		pValue[ 0 ] = b[ 0 ];
#endif
	}

	// -----------------------------------------------------------------------------
private:


	FILE*						m_pFile;
	char*						m_buffer;
	u32							m_bufferLength;
	std::string					m_fileName;
};

// ------------------------------------------------------------------------------
