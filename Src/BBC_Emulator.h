#pragma once

//-------------------------------------------------------------------------------------------------
#include <time.h>

extern CPU cpu;

//-------------------------------------------------------------------------------------------------
class BBC_Emulator
{
public:
	BBC_Emulator();
	~BBC_Emulator( );

	//-------------------------------------------------------------------------------------------------

	struct CPUStateHistory
	{
		struct State
		{
			CPU::Registers reg_state;
			u8 pc_state[3];
		};
		static const int c_nHistory = 32;
		CPUStateHistory()
		{
			m_nOffset = m_nCount = 0;
		}
		int GetHistoryLength() const
		{
			return c_nHistory;
		}

		void RecordCPUState( CPU::Registers reg_state, u8* pc_state )
		{
			m_instructions[ m_nOffset ].pc_state[0] = pc_state[0];
			m_instructions[ m_nOffset ].pc_state[1] = pc_state[1];
			m_instructions[ m_nOffset ].pc_state[2] = pc_state[2];
			m_instructions[ m_nOffset ].reg_state = reg_state;
			m_nOffset++;
			m_nCount++;
			if ( m_nOffset >= c_nHistory )
			{
				m_nOffset = 0;
			}
		}
		int GetCount( ) const { return m_nCount > c_nHistory ? c_nHistory : m_nCount ; }
		const State& GetStateAtTime( int nAge ) const // -ve 0 = now, -10 = 10 instruction ago
		{
			int nOffset = m_nOffset + nAge - 1;
			if ( nOffset < 0 )
				nOffset += c_nHistory;

			return m_instructions[ nOffset ];
		}

		void GetHistory( std::string& DisassemblyHistory ) const
		{
			DisassemblyHistory.clear();
			int cCount = GetCount();
			for ( int i = -cCount + 1; i <= 0; i++ )
			{
				const State& state = GetStateAtTime( i );
				string disassemble;
				cpu.Disassemble( state.reg_state, state.pc_state, disassemble, nullptr );
				DisassemblyHistory += disassemble;
			}
		}

		State m_instructions[ c_nHistory ];
		int m_nCount;
		int m_nOffset;
	};

	//-------------------------------------------------------------------------------------------------

	bool						RunFrame( std::string* pDisassemblyString, bool bDebug );
	bool						ProcessInstructions( int nCount, std::string* pDisassemblyHistory, bool bDebug );
	void						SetBreakpoint( u16 address );

private:

	VideoULA					m_videoULA;

	CPUStateHistory				m_history;
	time_t						m_lastTime;
	bool						m_bStarted = false;
	bool						m_bPaused = false;
};

//-------------------------------------------------------------------------------------------------

