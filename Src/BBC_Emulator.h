#pragma once

//-------------------------------------------------------------------------------------------------
#include <time.h>
//-------------------------------------------------------------------------------------------------

class BBC_Emulator
{
public:
	BBC_Emulator();
	~BBC_Emulator( );

	//-------------------------------------------------------------------------------------------------

	struct InstructionHistory
	{
		static const int c_nHistory = 32;
		InstructionHistory()
		{
			m_nOffset = m_nCount = 0;
		}
		int GetHistoryLength() const
		{
			return c_nHistory;
		}

		void AddStringToHistory( std::string& stringToAdd )
		{
			m_instuctions[ m_nOffset ] = stringToAdd;
			m_nOffset++;
			m_nCount++;
			if ( m_nOffset >= c_nHistory )
			{
				m_nOffset = 0;
			}
		}
		int GetCount( ) const { return m_nCount > c_nHistory ? c_nHistory : m_nCount ; }
		const std::string& GetString( int nAge ) const // -ve 0 = now, -10 = 10 instruction ago
		{
			int nOffset = m_nOffset + nAge - 1;
			if ( nOffset < 0 )
				nOffset += c_nHistory;

			return m_instuctions[ nOffset ];
		}

		void GetHistory( std::string& DisassemblyHistory ) const
		{
			DisassemblyHistory.clear();
			int cCount = GetCount();
			for ( int i = -cCount + 1; i <= 0; i++ )
			{
				DisassemblyHistory+= GetString( i );
			}
		}

		std::string m_instuctions[ c_nHistory ];
		int m_nCount;
		int m_nOffset;
	};

	//-------------------------------------------------------------------------------------------------

	bool				RunFrame( std::string* pDisassemblyString, bool bDebug );
	void				ProcessInstructions( int nCount, std::string* pDisassemblyHistory, bool bDebug );
private:
	InstructionHistory	m_history;
	CPUEmulator			m_cpuEmulator;
	VideoULA			m_videoULA;

	time_t				m_lastTime;
	bool				m_bStarted = false;
	bool				m_bPaused = false;
};

//-------------------------------------------------------------------------------------------------

