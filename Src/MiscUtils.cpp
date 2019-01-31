//-------------------------------------------------------------------------------------------------
//
// A few common utility functions
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace Utils;

//-------------------------------------------------------------------------------------------------
std::string Utils::toHex( u8 i, bool bPrefix )
{
	char buffer[256];
	_itoa( i, buffer, 16 );
	string outp = string(buffer);
	while ( outp.length() < 2 )	
	{
		outp = "0" + outp;
	}
	for ( u32 i = 0 ; i < outp.length(); i++ )
	{
		if ( outp[i]>='a'&&outp[i]<='z')
			outp[i]+='A'-'a';
	}

	if ( bPrefix )
		return "$"+outp;
	return outp;
}
//-------------------------------------------------------------------------------------------------
std::string Utils::toHex( u16 i, bool bPrefix )
{
	char buffer[256];
	_itoa( i, buffer, 16 );
	string outp = string(buffer);
	while ( outp.length() < 4 )	
	{
		outp = "0" + outp;
	}
	for ( u32 i = 0 ; i < outp.length(); i++ )
	{
		if ( outp[i]>='a'&&outp[i]<='z')
			outp[i]+='A'-'a';
	}

	if ( bPrefix )
		return "$"+outp;
	return outp;
}