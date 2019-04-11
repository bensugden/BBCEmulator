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
	DebugServer();
	~DebugServer();
private:
	int						InitWinsock();
	int						ExitWinsock();
	int						Run();
	static void				RunDispatch(DebugServer* server);

	std::thread*			m_pThread;
    SOCKET					m_ListenSocket;
	SOCKET					m_ClientSocket;

};

//-------------------------------------------------------------------------------------------------
