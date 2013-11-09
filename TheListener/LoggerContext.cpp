#include "stdafx.h"
#include "LoggerContext.h"
#include "LogWindow.h"

#define PIPE_TIMEOUT 5000
 
CLoggerContext::CLoggerContext(void)
{
	m_hPipeInst = INVALID_HANDLE_VALUE;
	m_EventRead = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_EventWrite= CreateEvent(NULL, TRUE, FALSE, NULL);
	m_EventQuit = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_LogWindow = NULL;

	memset(&(m_AsyncContext.oOverlap), 0, sizeof(OVERLAPPED));
	m_AsyncContext.oOverlap.hEvent = m_EventRead;
	m_AsyncContext.context = this;

	thread = ::CreateThread(NULL, 0, CLoggerContext::StaticThreadProc, this, CREATE_SUSPENDED, NULL);
	::ResumeThread(thread);
}

CLoggerContext::~CLoggerContext(void)
{
	DeleteObject(m_EventRead);
	DeleteObject(m_EventWrite);
	DeleteObject(m_EventQuit);
}

void CLoggerContext::NewConnection()
{
	if (m_hPipeInst != INVALID_HANDLE_VALUE && !DisconnectNamedPipe(m_hPipeInst))
	{
		printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
	}
 
	m_isConnecting = false;
	m_isReading = false;
	m_isWriting = false;
 
	// Start an overlapped connection for this pipe instance. 
	BOOL fConnected = ConnectNamedPipe(m_hPipeInst, &m_AsyncContext.oOverlap); 
 
	// Overlapped ConnectNamedPipe should return zero. 
	if (fConnected) 
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError()); 
		//return 0;
		throw new std::exception();
	}
 
	switch (GetLastError()) 
	{ 
	// The overlapped connection in progress. 
		case ERROR_IO_PENDING: 
			m_isConnecting = TRUE; 
			break; 
 
	// Client is already connected, so signal an event.  
		case ERROR_PIPE_CONNECTED: 
			if (SetEvent(m_AsyncContext.oOverlap.hEvent)) 
			break; 
 
	// If an error occurs during the connect operation... 
		default: 
		{
			printf("ConnectNamedPipe failed with %d.\n", GetLastError());
			//return 0;
			throw new std::exception();
		}
	}
}

void CLoggerContext::BeginRead()
{
	if(!ReadFile(m_hPipeInst, m_ReadBuffer, sizeof(m_ReadBuffer), &m_ReadCount, &m_AsyncContext.oOverlap))
	{
		if(GetLastError() == ERROR_IO_PENDING)
			m_isReading = true;
		else
			NewConnection();
	}
	else
	{
		EndRead();
		SetEvent(m_AsyncContext.oOverlap.hEvent);
	}
}

void CLoggerContext::EndRead()
{
	int unicodeLength = MultiByteToWideChar(CP_UTF8, 0, m_ReadBuffer, m_ReadCount, m_Text, sizeof(m_Text));

	for(int i=0; i< unicodeLength;i++)
	{
		TCHAR cc = m_Text[i];
		if(cc == 10 || cc == 13)
		{
			if(m_TextBuffer.size() > 0)
				AddItem();
			m_TextBuffer.clear();
		}
		else
		{
			m_TextBuffer.append(1, cc);
		}
	}
}

void CLoggerContext::AddItem()
{
	CLogItem *item = new CLogItem(m_TextBuffer.c_str(), _T(""));
	m_Items.push_back(item);
	if(m_LogWindow)
		m_LogWindow->ItemAdded();
}

int CLoggerContext::ThreadProc()
{
	HANDLE handles[] = { m_EventRead, m_EventWrite, m_EventQuit };
		
	DWORD cbRet; 
	BOOL fSuccess; 

	m_hPipeInst = CreateNamedPipe( 
		m_PipeName.c_str(),            // pipe name 
		PIPE_ACCESS_DUPLEX |     // read/write access 
		FILE_FLAG_OVERLAPPED,    // overlapped mode 
		PIPE_TYPE_BYTE |      // message-type pipe 
		PIPE_READMODE_BYTE |  // message-read mode 
		PIPE_WAIT,               // blocking mode 
		1,						  // number of instances 
		BUFSIZE*sizeof(TCHAR),   // output buffer size 
		BUFSIZE*sizeof(TCHAR),   // input buffer size 
		PIPE_TIMEOUT,            // client time-out 
		NULL);                   // default security attributes 

	if (m_hPipeInst == INVALID_HANDLE_VALUE) 
	{
		printf("CreateNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}
 
	NewConnection();
 
	while (true) 
	{  
		int which = WaitForMultipleObjects(3, handles, FALSE, INFINITE) - WAIT_OBJECT_0;
		
		switch(which)
		{
		case 0:
			ResetEvent(handles[0]);
			fSuccess = GetOverlappedResult(m_hPipeInst, &m_AsyncContext.oOverlap, &cbRet, FALSE);
			if (m_isConnecting) 
			{
				m_isConnecting = false;
				if (! fSuccess) 
				{
					printf("Error %d.\n", GetLastError()); 
					return 0;
				}
			}
			else if(m_isReading)
			{
				m_isReading = false;
				if (!fSuccess) 
				{ 
					NewConnection(); 
					continue; 
				}
				m_ReadCount = cbRet;
				EndRead();
			}
			BeginRead();
			break;
		case 1:
			ResetEvent(handles[1]);
			// TODO: WRITE
			break;
		default:
			return 0;
		}
	}
}

DWORD WINAPI CLoggerContext::StaticThreadProc(PVOID object)
{
	return ((CLoggerContext*)object)->ThreadProc();
}
