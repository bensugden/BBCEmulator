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
	void CrawlCodeFrom( int pc );

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
	int GetDestAddress( int pc, const CommandInfo& command, u8 lo, u8 hi );
	std::vector<MemReference> m_memory;
};

//-------------------------------------------------------------------------------------------------
