//-------------------------------------------------------------------------------------------------
//
// Debug Server
// Using Winsock
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

//-------------------------------------------------------------------------------------------------

DebugServer::DebugServer()
{
	m_pThread = new std::thread( RunDispatch, this );
	m_pThread->detach();
}

//-------------------------------------------------------------------------------------------------

void DebugServer::RunDispatch(DebugServer* server)
{
	s32 res = server->Run();
}

//-------------------------------------------------------------------------------------------------

DebugServer::~DebugServer()
{
	delete m_pThread;
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

		// Accept a client socket
		m_ClientSocket = accept(m_ListenSocket, NULL, NULL);
		if (m_ClientSocket == INVALID_SOCKET) 
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(m_ListenSocket);
			WSACleanup();
			return 1;
		}
		SleepEx(1,false);
	};

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

   
    // Initialize Winsock
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

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) 
	{
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    m_ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_ListenSocket == INVALID_SOCKET) 
	{
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
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
    // No longer need server socket
    closesocket(m_ListenSocket);

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
	int iResult;
    // Receive until the peer shuts down the connection
    do 
	{
        iResult = recv(m_ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

        // Echo the buffer back to the sender
            iSendResult = send( m_ClientSocket, recvbuf, iResult, 0 );
            if (iSendResult == SOCKET_ERROR) 
			{
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(m_ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(m_ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(m_ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) 
	{
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(m_ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(m_ClientSocket);
    WSACleanup();
	return 0;
 }