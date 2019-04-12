#pragma once

//-------------------------------------------------------------------------------------------------
#include <time.h>

extern CPU cpu;

//-------------------------------------------------------------------------------------------------
class BBC_Emulator : public ISystemClock
{
public:
	BBC_Emulator();
	~BBC_Emulator( );

	virtual void Tick();
	virtual void PollChips();
	virtual u64 GetClockCounter() { return m_nClockCounter; };

	//-------------------------------------------------------------------------------------------------
	//
	// Common Interface
	//
	//-------------------------------------------------------------------------------------------------

	void	Reset();
	void	RefreshDisplay();
	bool	RunFrame( std::string* pDisassemblyString, bool bDebug );
	bool	ProcessInstructions( int nCount, std::string* pDisassemblyHistory, bool bDebug, bool bForceDebugPC = false, bool bAlwaysSpewToOutputWindow = false );
	void	SetBreakpoint( u16 address );

	//
	// These block until critical section is free ( will wait for a frame boundary if in run mode )
	//
	void	EnterEmulatorCriticalSection();
	void	ExitEmulatorCriticalSection();

	void	InsertDisk( int drive, const std::string& filename );
	void	EjectDisk( int drive );
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
			Clear();
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

		void GetLastInstruction( std::string& disassemble ) const
		{
			int cCount = GetCount();
			const State& state = GetStateAtTime( 0 );
			cpu.DisassembleAtCPUState( state.reg_state, disassemble, nullptr );
		}

		void GetHistory( std::string& DisassemblyHistory ) const
		{
			DisassemblyHistory.clear();
			int cCount = GetCount();
			for ( int i = -cCount + 1; i <= 0; i++ )
			{
				const State& state = GetStateAtTime( i );
				string disassemble;
				cpu.DisassembleAtCPUState( state.reg_state, disassemble, nullptr );
				DisassemblyHistory += disassemble;
			}
		}

		void Clear()
		{
			m_nCount = m_nOffset = 0;
		}
		bool IsEmpty()
		{
			return m_nCount == 0;
		}

		State m_instructions[ c_nHistory ];
		int m_nCount;
		int m_nOffset;
	};

	//-------------------------------------------------------------------------------------------------

	void SetKeyDown( u8 key, bool bIsVKey )		{ m_keyboard.SetKeyDown( key, bIsVKey ); }
	void SetKeyUp( u8 key, bool bIsVKey )		{ m_keyboard.SetKeyUp( key, bIsVKey ); }

	//-------------------------------------------------------------------------------------------------

private:
	void						DebugDecodeNextInstruction();

	VideoULA					m_videoULA;
	System_VIA_6522				m_systemVIA;
	Ports_VIA_6522				m_portsVIA;
	TI_76489					m_ti76489;
	LS161PagedRomController		m_pagedRomController;
	SAA5050						m_teletext;
	CRTC_6845					m_crtc;
	I8271_FDC					m_fdc;
	BBC_Keyboard				m_keyboard;
	XAudio2						m_audioPlayer;
	DebugServer					m_debugServer;
	u64							m_nClockCounter = 0;
	u64							m_nLastClockCounter = 0;
	CPUStateHistory				m_history;

	LARGE_INTEGER				m_lastTime;
	LARGE_INTEGER				m_timerFreq;
	double						m_lastTimeInSeconds;
	CRITICAL_SECTION			m_csForBreak;

	FloppyDisk*					m_floppies[2];
	bool						m_bStarted = false;
	bool						m_bPaused = false;
};

//-------------------------------------------------------------------------------------------------

