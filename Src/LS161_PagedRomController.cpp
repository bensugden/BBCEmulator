#include "stdafx.h"

//-------------------------------------------------------------------------------------------------
//
// LS161PagedRomController
//
//-------------------------------------------------------------------------------------------------

LS161PagedRomController::LS161PagedRomController()
{
	mem.RegisterMemoryMap_Write( SHEILA::WRITE_LS161_Paged_ROM_RAM_ID, MemoryMapHandler( LS161PagedRomController::SelectROM ) );
}
//-------------------------------------------------------------------------------------------------

u8 LS161PagedRomController::SelectROM( u16 address, u8 value )
{
	mem.SelectRomForAddress( 0x8000, value );
	return value;
}

//-------------------------------------------------------------------------------------------------