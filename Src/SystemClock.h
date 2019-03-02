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
	virtual void PollChips() = 0;
	virtual u64 GetClockCounter() = 0;
};

//-------------------------------------------------------------------------------------------------
