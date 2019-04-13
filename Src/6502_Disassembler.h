#pragma once

//-------------------------------------------------------------------------------------------------
//
// 6502 Disassembler / Code Crawler
//
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------

class Disassembler
{
public:
	Disassembler( );

	void				DisassembleFrom( int );
	void				GenerateCode( std::string& code );


private:	
	struct MemReference
	{
		MemReference()
		{
			flags = MEM_UNKNOWN;
		}
		enum ETypeFlags
		{
			MEM_UNKNOWN			= 0,
			MEM_INSTRUCTION		= 1<<1,
			MEM_EA				= 1<<2,
			MEM_CONFIRMED		= 1<<3
		};
		u8		data;
		u8		flags;
	};

	bool				IsMemoryAlreadyDisassembled( int pc, const CommandInfo& command );
	const CommandInfo&	DecodeAt( int pc );
	void				MarkAsDecoded( int pc, const CommandInfo& command, bool bConfirmed );
	int					GetDestAddress( int pc, const CommandInfo& command );
	void				CreateLabel( int address );
	void				RegenerateLabelNameInAddressOrder();

	std::vector<MemReference>	m_memory;
	std::map<int,std::string>	m_memToLabelMap;
	std::vector<int>			m_pcToLineMap;
	std::vector<int>			m_lineToPCMap;
	int							m_currentLabelIndex;
};

//-------------------------------------------------------------------------------------------------
// 
// NOTE: Getting disassembler requires us to lock the CPU when done via another thread, so
//		 I've wrapped this behind a little struct to lock and unlock automatically
//
//-------------------------------------------------------------------------------------------------

struct DisassemblerContext
{
	DisassemblerContext( Disassembler&, class BBC_Emulator& );
	~DisassemblerContext();
	Disassembler& GetDisassembler()
	{
		return m_disassembler;
	}
private:
	Disassembler& m_disassembler;
	BBC_Emulator& m_emulator;
};
//-------------------------------------------------------------------------------------------------
