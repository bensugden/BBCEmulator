#pragma once

//-------------------------------------------------------------------------------------------------
//
// 6502 Disassembler / Code Crawler
//
//-------------------------------------------------------------------------------------------------

class Disassembler
{
public:
	Disassembler( int memorySize );
	void DisassembleFrom( int entry_point );

private:	
	struct MemReference
	{
		MemReference()
		{
			type = MEM_UNKNOWN;
		}
		enum EType
		{
			MEM_UNKNOWN = 0,
			MEM_INSTRUCTION,
			MEM_EA,
		};
		u8		data;
		EType	type;
	};
	bool IsMemoryAlreadyDisassembled( int pc, const CommandInfo& command );
	const CommandInfo& DecodeAt( int pc );
	void MarkAsDecoded( int pc, const CommandInfo& command );
	void CrawlCodeFrom( int pc );
	int GetDestAddress( int pc, const CommandInfo& command );
	std::vector<MemReference> m_memory;
};

//-------------------------------------------------------------------------------------------------
