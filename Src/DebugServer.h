#pragma once
//-------------------------------------------------------------------------------------------------
//
// Network Debug Server
// For debugging with VS Code / external IDE
//
//-------------------------------------------------------------------------------------------------

class DebugServer
{
public:
	DebugServer( class BBC_Emulator& emulator );
	~DebugServer();
private:
	int									Run();
	static void							RunDispatch(DebugServer* server);
	
	//
	// Network Stuff
	//
	int									InitWinsock();
	int									ExitWinsock();
	bool								Send( const std::string& data );

	std::thread*						m_pThread;
	SOCKET								m_ListenSocket;
	SOCKET								m_ClientSocket;

	//
	// Emulator / Disassembler Etc.
	//

	void								GetEmulatorStatus();
	class BBC_Emulator&					m_emulator;

	//
	// Event Handling
	//
	typedef bool (DebugServer::*EventHandler) (const std::string&);
	void								RegisterEventHandlers();
	bool								HandleEvent( const string& packet );
	void								AddEventHandler( EventHandler, const std::string& );

	//
	// Event Handlers
	//
	bool								OnStep( const std::string& data );

	std::map<std::string, EventHandler>	m_eventHandlers;

};

//-------------------------------------------------------------------------------------------------
