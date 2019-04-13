//-------------------------------------------------------------------------------------------------
//
// Debug Server
// Using Winsock
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

using namespace std;

//-------------------------------------------------------------------------------------------------
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

//-------------------------------------------------------------------------------------------------
const DWORD MS_VC_EXCEPTION = 0x406D1388;  
#pragma pack(push,8)  
typedef struct tagTHREADNAME_INFO  
{  
    DWORD dwType; // Must be 0x1000.  
    LPCSTR szName; // Pointer to name (in user addr space).  
    DWORD dwThreadID; // Thread ID (-1=caller thread).  
    DWORD dwFlags; // Reserved for future use, must be zero.  
 } THREADNAME_INFO;  
#pragma pack(pop) 
static void SetThreadName(uint32_t dwThreadID, const char* threadName)
{

  // DWORD dwThreadID = ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );

   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}
//-------------------------------------------------------------------------------------------------

DebugServer::DebugServer( BBC_Emulator& emulator )
	: m_emulator( emulator )
{
	m_pThread = new std::thread( RunDispatch, this );
	
	DWORD threadId = ::GetThreadId( static_cast<HANDLE>( m_pThread->native_handle() ) );
    SetThreadName( threadId ,"DebugServer");

	m_pThread->detach();

	RegisterEventHandlers();
}

//-------------------------------------------------------------------------------------------------

void DebugServer::RunDispatch(DebugServer* server)
{
	s32 res = server->Run();
}

//-------------------------------------------------------------------------------------------------

DebugServer::~DebugServer()
{
	ExitWinsock();
	delete m_pThread;
}

//-------------------------------------------------------------------------------------------------

bool DebugServer::OnStep( const string& data )
{
	// Echo the buffer back to the sender
	GetEmulatorStatus();

	return true;
}

//-------------------------------------------------------------------------------------------------

bool DebugServer::Send( const string& data )
{
	int iSendResult = send( m_ClientSocket, data.c_str(), data.length() + 1, 0 );
	if (iSendResult == SOCKET_ERROR) 
	{
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------------------

void DebugServer::RegisterEventHandlers()
{
	AddEventHandler( &DebugServer::OnStep, "step" );
}

//-------------------------------------------------------------------------------------------------

void DebugServer::AddEventHandler( EventHandler fn, const string& handle )
{
	m_eventHandlers.insert( pair<string, EventHandler>( handle, fn ) );
}

//-------------------------------------------------------------------------------------------------

static void split(const string& str, const string& delim, vector<string>& parts) 
{
	size_t start, end = 0;
	while (end < str.size()) 
	{
		start = end;
		while (start < str.size() && (delim.find(str[start]) != string::npos))
		{
			start++;  // skip initial whitespace
		}
		end = start;
		while (end < str.size() && (delim.find(str[end]) == string::npos)) 
		{
			end++; // skip to end of word
		}
		if (end-start != 0) 
		{  // just ignore zero-length strings.
			parts.push_back(string(str, start, end-start));
		}
	}
}

//-------------------------------------------------------------------------------------------------

bool DebugServer::HandleEvent( const string& packet )
{
	//
	// Strip event name
	//
	vector<string> parts;
	split( packet, " ", parts );

	auto it = m_eventHandlers.find( parts[0] );
	if ( it == m_eventHandlers.end() )
		return false;
	//
	// Is this the worlds worst syntax, or what?
	//
	return (this->*((*it).second))( parts.size() > 1 ? parts[1] : "" );
}

//-------------------------------------------------------------------------------------------------

void DebugServer::GetEmulatorStatus()
{
	DisassemblerContext dc = m_emulator.GetDisassemblerContext();
	dc.GetDisassembler().DisassembleFrom( cpu.reg.PC );

	string code;
	dc.GetDisassembler().GenerateCode( code );
	FILE* fp = fopen( "D:\\test.txt", "w" );
	fwrite( code.c_str(), code.length() + 1, 1, fp );
	fclose( fp );
}

//-------------------------------------------------------------------------------------------------

int DebugServer::Run()
{
	InitWinsock();

	while ( true )
	{
		int  iResult = listen(m_ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) 
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(m_ListenSocket);
			WSACleanup();
			return 1;
		}

		//
		// Accept a client socket
		//
		m_ClientSocket = accept(m_ListenSocket, NULL, NULL);
		if (m_ClientSocket == INVALID_SOCKET) 
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(m_ListenSocket);
			WSACleanup();
			return 1;
		}


		//
		// Receive until the peer shuts down the connection
		//
		do 
		{
			char recvbuf[DEFAULT_BUFLEN];
			int recvbuflen = DEFAULT_BUFLEN;
			iResult = recv(m_ClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) 
			{
				HandleEvent( std::string( recvbuf ) );
			}
			else if (iResult == 0)
			{
				// connection closed
				break;
			}
			else  
			{
				goto exit;
			}

		} while (iResult > 0);
	};
exit:
	ExitWinsock();

	return 0;
}

//-------------------------------------------------------------------------------------------------

int DebugServer::InitWinsock()
{

	WSADATA wsaData;
    int iResult;

	m_ListenSocket = INVALID_SOCKET;
	m_ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

	//
    // Initialize Winsock
	//
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) 
	{
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

	//
    // Resolve the server address and port
	//
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) 
	{
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

	//
    // Create a SOCKET for connecting to server
	//
    m_ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_ListenSocket == INVALID_SOCKET) 
	{
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

	//
    // Setup the TCP listening socket
	//
    iResult = ::bind( m_ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) 
	{
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(m_ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);
	return 0;
}

//-------------------------------------------------------------------------------------------------

int DebugServer::ExitWinsock()
{
	//
    // No longer need server socket
	//
    closesocket(m_ListenSocket);
	   
	int iResult;

	//
    // Shutdown the connection since we're done
	//
    iResult = shutdown(m_ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) 
	{
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(m_ClientSocket);
        WSACleanup();
        return 1;
    }

	//
    // Cleanup
	//
    closesocket(m_ClientSocket);
    WSACleanup();
	return 0;
 }