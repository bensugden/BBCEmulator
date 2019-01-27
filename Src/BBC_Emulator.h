#pragma once

//-------------------------------------------------------------------------------------------------
#include <time.h>
//-------------------------------------------------------------------------------------------------

class BBC_Emulator
{
public:
	BBC_Emulator();
	~BBC_Emulator( );

	bool RunFrame( std::string* p_debugOutput );
	void ProcessInstructions( int nCount, std::string* pDebugOutput );
private:
	CPUEmulator m_cpuEmulator;
	time_t m_lastTime;
	bool bStarted = false;
	bool bPaused = false;
};

//-------------------------------------------------------------------------------------------------

