#pragma once
#include "myAssert.h"

//-------------------------------------------------------------------------------------------------
//
// System Clock
//
// Handles calling of auxiliary systems on other buses
//
//-------------------------------------------------------------------------------------------------

class ISystemClock
{
public:
	virtual void Tick() = 0;
	virtual int GetClockCounter() = 0;
};

//-------------------------------------------------------------------------------------------------
