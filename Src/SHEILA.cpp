//-------------------------------------------------------------------------------------------------
//
// SHEILA I/O Interface
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------

static u16 c_sheilaLivesAt = 0xFE00;
static void (*s_listenerFunction[ 256 ])( SHEILA::Write_Address, u8 );

//-------------------------------------------------------------------------------------------------

void SHEILA::Init()
{
	memset( s_listenerFunction, 0, sizeof( s_listenerFunction ) );
	mem.RegisterMemoryMappedSystem( c_sheilaLivesAt, c_sheilaLivesAt + 0xff, NotifyMemWrite );
}

//-------------------------------------------------------------------------------------------------

void SHEILA::RegisterListener( Write_Address address, void (*listenerFunction)( Write_Address, u8 ) )
{
	assert( ( s_listenerFunction[ (u8)address ] == nullptr ) ); // one listener allowed per address
	s_listenerFunction[ (u8)address ] = listenerFunction;
}

//-------------------------------------------------------------------------------------------------

void SHEILA::NotifyMemWrite( u16 address, u8 value )
{
}

//-------------------------------------------------------------------------------------------------

u8 SHEILA::Read( Write_Address address )
{
	return mem.Read( c_sheilaLivesAt + (u16)address );
}

//-------------------------------------------------------------------------------------------------

void SHEILA::Write( Read_Address address, u8 value )
{
	mem.Write( c_sheilaLivesAt + (u16)address, value );
}

//-------------------------------------------------------------------------------------------------
