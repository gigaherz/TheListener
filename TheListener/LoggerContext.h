#pragma once

#include "LogItem.h"
#include "NamedPipe.h"

// Forward declaration
class CLogWindow;

#define BUFSIZE 1024

class CLoggerContext
{
public:

	CLoggerContext(void);

	virtual ~CLoggerContext(void);

	void SetPipeName(LPCTSTR pipeName) { m_PipeName = std::wstring(pipeName); }
	LPCTSTR GetPipeName() { return m_PipeName.c_str(); }
	
	std::vector<CLogItem*> m_Items;

	CLogWindow *m_LogWindow;

private:
	std::wstring m_PipeName;

	HANDLE m_EventRead;
	HANDLE m_EventWrite;
	HANDLE m_EventQuit;
	
	HANDLE m_hPipeInst;

	struct _ASYNC_CONTEXT {	
	    OVERLAPPED oOverlap; 
		CLoggerContext *context;
	} m_AsyncContext;

	bool m_isConnecting;
	bool m_isReading;
	bool m_isWriting;

	HANDLE thread;

	CHAR m_ReadBuffer[1024];
	DWORD m_ReadCount;
	TCHAR m_Text[1025];

	std::wstring m_TextBuffer;

	CFile m_FileLog;

	void NewConnection();
	void BeginRead();
	void EndRead();
	void AddItem();

	int ThreadProc();
	
	static DWORD WINAPI StaticThreadProc(PVOID object);
};

