#pragma once

//-------------------------------------------------------------------------------------------------
//
// 6522 VIA Parallel / User Ports Chip
//
//-------------------------------------------------------------------------------------------------

#include "6522_VIA.h"

class Ports_VIA_6522 : public VIA_6522
{
public:
	Ports_VIA_6522();

	//
	// Stubs
	//
	 
	virtual void	WritePortA( u8 value ) {}; 
	virtual void	WritePortB( u8 value ) {}; 

	virtual u8		ReadPortA( ) { return 0xff; }
	virtual u8		ReadPortB( ) { return 0xff; }
};

//-------------------------------------------------------------------------------------------------