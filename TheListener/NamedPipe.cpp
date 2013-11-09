/*
Module : NPIPE.CPP
Purpose: Defines the implementation for an MFC wrapper class
         for Win32 named Pipes
Created: PJN / 02-08-1998
History: PJN / 21-02-2002 1. Updated copyright message in source code and documentation
                          2. Fixed a bug in Close method where the handle value was not being reset to INVALID_HANDLE_VALUE.
                          3. Tidied up the TRACE code
                          4. Tidied up build configurations for sample apps
         PJN / 28-07-2002 1. Updated sample server app to do Flush of the pipe before we disconnect the client. Thanks to 
                          "Martin" for spotting this problem.
         PJN / 09-11-2002 1. ConnectClient now returns TRUE if the last error returns ERROR_PIPE_CONNECTED which indicates 
                          that a client is already connected before we make the call. Thanks to Metrich Frederic for 
                          reporting this.
         PJN / 05-03-2003 1. Changed the class to use exceptions rather than SDK style return values
         PJN / 12-11-2003 1. Attach now includes an AutoClose parameter. This allows control over whether the pipe handle
                          should be closed when the pipe object goes out of scope or CNamedPipe::Close is called. Thanks
                          to Metrich Frederic for reporting this issue.
         PJN / 19-12-2003 1. Fixed ASSERT's at the start of most CNamedPipe functions which verify that the pipe handle 
                          is valid. Thanks to Metrich Frederic for reporting this issue.
         PJN / 15-07-2006 1. Updated copyright details.
                          2. Renamed AfxThrowNamedPipeException to ThrowNamedPipeException and made it part of the 
                          CNamedPipe class.
                          3. CNamedPipe is no longer derived from CObject as it was not really required.
                          4. Optimized CNamedPipe constructor code.
                          5. Code now uses new C++ style casts rather than old style C casts where necessary. 
                          6. Optimized CNamedPipeException constructor code
                          7. Removed the unnecessary CNamedPipeException destructor
                          8. Removed some unreferenced variables in the sample app.
                          9. Updated the code to clean compile on VC 2005
                          10. Updated documentation to use the same style as the web site.
                          11. Addition of a CNAMEDPIPE_EXT_CLASS macro to allow the classes to be easily added to an 
                          extension dll.
         PJN / 28-12-2007 1. Updated copyright details.
                          2. Updated the sample apps to clean compile on VC 2005
                          3. Sample client app now defaults to "." (meaning the current machine) as the server to connect to.
                          4. CNamedPipeException::GetErrorMessage now uses the FORMAT_MESSAGE_IGNORE_INSERTS flag. For more 
                          information please see Raymond Chen's blog at 
                          http://blogs.msdn.com/oldnewthing/archive/2007/11/28/6564257.aspx. Thanks to Alexey Kuznetsov for 
                          reporting this issue.
                          5. CAppSettingsException::GetErrorMessage now uses Checked::tcsncpy_s if compiled using VC 2005 or 
                          later.
                          6. Provision of new overloaded versions of the Peek, Write and Read methods which allows the 
                          dwBytesRead/dwBytesWritten parameters to be returned as an output parameter as opposed to the return 
                          value of the method. This helps resolve a situation where the underlying WriteFile / ReadFile call 
                          fails but some data has actually been written / read from the pipe. Thanks to Gintautas Kisonas for 
                          reporting this issue.
                          7. dwBytesRead, dwTotalBytesAvail and dwBytesLeftThisMessage parameters to Peek are now pointers rather
                          than references. Thanks to Gintautas Kisonas for reporting this issue.
         PJN / 30-12-2007 1. Updated the sample apps to clean compile on VC 2008
         PJN / 12-07-2008 1. Updated copyright details.
                          2. Updated sample app to clean compile on VC 2008
                          3. The code has now been updated to support VC 2005 or later only. 
                          4. Code now compiles cleanly using Code Analysis (/analyze)
                          5. Removed the m_bAutoClose member variable and concept from class

Copyright (c) 1998 - 2007 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/

#include "stdafx.h"
#include "NamedPipe.h"
#include <cassert>

CNamedPipe::CNamedPipe() : m_hPipe(INVALID_HANDLE_VALUE)
{
}

CNamedPipe::~CNamedPipe()
{
  Close();
}

void CNamedPipe::Create(LPCTSTR lpszName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD dwMaxInstances, DWORD dwOutBufferSize, 
                        DWORD dwInBufferSize, DWORD dwDefaultTimeOut, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
  //Validate our parameters
  assert(!IsOpen());
  assert(_tcslen(lpszName));

  //the class encapsulates creating the pipe name, all that is required is
  //a simple name for the pipe e.g. lpName = PJPIPE will create the pipe
  //name "\\.\PIPE\PJPIPE"
  CString sPipeName;
  sPipeName.Format(_T("\\\\.\\pipe\\%s"), lpszName);

  m_hPipe = ::CreateNamedPipe(sPipeName, dwOpenMode, dwPipeMode, dwMaxInstances, dwOutBufferSize, dwInBufferSize, dwDefaultTimeOut, lpSecurityAttributes);
  if (m_hPipe == INVALID_HANDLE_VALUE)
    ThrowNamedPipeException();
}

void CNamedPipe::Open(LPCTSTR lpszServerName, LPCTSTR lpszPipeName, DWORD dwDesiredAccess, DWORD dwShareMode, 
                      LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwFlagsAndAttributes)
{
  //Validate our parameters
  assert(!IsOpen());
  assert(_tcslen(lpszServerName));
  assert(_tcslen(lpszPipeName));

  //Construct the canonical pipe name
  CString sPipeName;
  sPipeName.Format(_T("\\\\%s\\pipe\\%s"), lpszServerName, lpszPipeName);

  m_hPipe = CreateFile(sPipeName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, OPEN_EXISTING, dwFlagsAndAttributes, NULL);
  if (m_hPipe == INVALID_HANDLE_VALUE)
    ThrowNamedPipeException();
}

void CNamedPipe::Attach(HANDLE hPipe)
{
  Close();
  m_hPipe = hPipe;
}

HANDLE CNamedPipe::Detach()
{
  HANDLE hReturn = m_hPipe;
  m_hPipe = INVALID_HANDLE_VALUE;
  return hReturn;
}

void CNamedPipe::Close()
{
  if (IsOpen())
  {
    ::CloseHandle(m_hPipe);
    m_hPipe = INVALID_HANDLE_VALUE;
  }
}

void CNamedPipe::ThrowNamedPipeException(DWORD dwError)
{
	if (dwError == 0)
		dwError = ::GetLastError();

	if(dwError == ERROR_IO_PENDING)
		return;

	std::stringstream strm;

	strm << "Error " << dwError;
	
	std::string str = strm.str();

	std::exception* pException = new std::exception(str.c_str());

	TCHAR buffer[4096];

	wsprintf(buffer, _T("Warning: throwing CNamedPipeException for error %d\n"), dwError);

	TRACE(buffer);

	throw pException;
}

void CNamedPipe::ConnectClient(LPOVERLAPPED lpOverlapped)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open
  assert(IsServerPipe()); //Must be called from the server side

  if (!::ConnectNamedPipe(m_hPipe, lpOverlapped))
    ThrowNamedPipeException();
}

void CNamedPipe::DisconnectClient()
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open
  assert(IsServerPipe()); //Must be called from the server side

  if (!::DisconnectNamedPipe(m_hPipe))
    ThrowNamedPipeException();
}

void CNamedPipe::Flush()
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  if (!::FlushFileBuffers(m_hPipe))
    ThrowNamedPipeException();
}

DWORD CNamedPipe::Write(LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwNumberOfBytesWritten = 0;
  if (!::WriteFile(m_hPipe, lpBuffer, dwNumberOfBytesToWrite, &dwNumberOfBytesWritten, lpOverlapped))
    ThrowNamedPipeException();

  return dwNumberOfBytesWritten;
}

void CNamedPipe::Write(LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite, DWORD& dwNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  if (!::WriteFile(m_hPipe, lpBuffer, dwNumberOfBytesToWrite, &dwNumberOfBytesWritten, lpOverlapped))
    ThrowNamedPipeException();
}

void CNamedPipe::Write(LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  if (!::WriteFileEx(m_hPipe, lpBuffer, dwNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine))
    ThrowNamedPipeException();
}

DWORD CNamedPipe::Read(LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, LPOVERLAPPED lpOverlapped)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwNumberOfBytesRead = 0;
  if (!::ReadFile(m_hPipe, lpBuffer, dwNumberOfBytesToRead, &dwNumberOfBytesRead, lpOverlapped))
    ThrowNamedPipeException();

  return dwNumberOfBytesRead;
}

void CNamedPipe::Read(LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, DWORD& dwNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  if (!::ReadFile(m_hPipe, lpBuffer, dwNumberOfBytesToRead, &dwNumberOfBytesRead, lpOverlapped))
    ThrowNamedPipeException();
}

void CNamedPipe::Read(LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  if (!::ReadFileEx(m_hPipe, lpBuffer, dwNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine))
    ThrowNamedPipeException();
}

DWORD CNamedPipe::Peek(LPVOID lpBuffer, DWORD dwBufferSize, DWORD* lpdwTotalBytesAvail, DWORD* lpdwBytesLeftThisMessage)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwBytesRead = 0;
  if (!::PeekNamedPipe(m_hPipe, lpBuffer, dwBufferSize, &dwBytesRead, lpdwTotalBytesAvail, lpdwBytesLeftThisMessage))
    ThrowNamedPipeException();

  return dwBytesRead;
}

void CNamedPipe::Peek(LPVOID lpBuffer, DWORD dwBufferSize, DWORD* lpdwBytesRead, DWORD* lpdwTotalBytesAvail, DWORD* lpdwBytesLeftThisMessage)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  if (!::PeekNamedPipe(m_hPipe, lpBuffer, dwBufferSize, lpdwBytesRead, lpdwTotalBytesAvail, lpdwBytesLeftThisMessage))
    ThrowNamedPipeException();
}

void CNamedPipe::Transact(LPVOID lpInBuffer, DWORD dwInBufferSize, LPVOID lpOutBuffer, DWORD dwOutBufferSize, DWORD& dwBytesRead, LPOVERLAPPED lpOverlapped)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  if (!::TransactNamedPipe(m_hPipe, lpInBuffer, dwInBufferSize, lpOutBuffer, dwOutBufferSize, &dwBytesRead, lpOverlapped))
    ThrowNamedPipeException();
}

BOOL CNamedPipe::IsBlockingPipe() const
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwState = 0;
  if (!::GetNamedPipeHandleState(m_hPipe, &dwState, NULL, NULL, NULL, NULL, 0))
    ThrowNamedPipeException();
                                      
  return ((dwState & PIPE_NOWAIT) == 0);
}

BOOL CNamedPipe::IsClientPipe() const
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwFlags = 0;
  if (!::GetNamedPipeInfo(m_hPipe, &dwFlags, NULL, NULL, NULL))
    ThrowNamedPipeException();
                                      
  return ((dwFlags & PIPE_CLIENT_END) != 0);
}

BOOL CNamedPipe::IsServerPipe() const
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwFlags = 0;
  if (!::GetNamedPipeInfo(m_hPipe, &dwFlags, NULL, NULL, NULL))
    ThrowNamedPipeException();
                                      
  return ((dwFlags & PIPE_SERVER_END) != 0);
}

BOOL CNamedPipe::IsMessagePipe() const
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwState = 0;
  if (!::GetNamedPipeHandleState(m_hPipe, &dwState, NULL, NULL, NULL, NULL, 0))
    ThrowNamedPipeException();
                                      
  return ((dwState & PIPE_READMODE_MESSAGE) != 0);
}

DWORD CNamedPipe::GetCurrentInstances() const
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwCurInstances = 0;
  if (!::GetNamedPipeHandleState(m_hPipe, NULL, &dwCurInstances, NULL, NULL, NULL, 0))
    ThrowNamedPipeException();
                                      
  return dwCurInstances;
}

DWORD CNamedPipe::GetMaxCollectionCount() const
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open
  assert(IsClientPipe()); //Must be called from the client side

  DWORD dwMaxCollectionCount = 0;
  if (!::GetNamedPipeHandleState(m_hPipe, NULL, NULL, &dwMaxCollectionCount, NULL, NULL, 0))
    ThrowNamedPipeException();
                                      
  return dwMaxCollectionCount;
}

DWORD CNamedPipe::GetCollectionTimeout() const
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open
  assert(IsClientPipe()); //Must be called from the client side

  DWORD dwCollectDataTimeout = 0;
  if (!::GetNamedPipeHandleState(m_hPipe, NULL, NULL, NULL, &dwCollectDataTimeout, NULL, 0))
    ThrowNamedPipeException();
                                      
  return dwCollectDataTimeout;
}

DWORD CNamedPipe::GetOutboundBufferSize() const
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwOutBufferSize = 0;
  if (!::GetNamedPipeInfo(m_hPipe, NULL, &dwOutBufferSize, NULL, NULL))
    ThrowNamedPipeException();
                                      
  return dwOutBufferSize;
}

DWORD CNamedPipe::GetInboundBufferSize() const
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwInBufferSize = 0;
  if (!::GetNamedPipeInfo(m_hPipe, NULL, NULL, &dwInBufferSize, NULL))
    ThrowNamedPipeException();
                                      
  return dwInBufferSize;
}

CString CNamedPipe::GetClientUserName() const
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open
  assert(IsServerPipe()); //Must be called from the server side

  TCHAR pszUserName[_MAX_PATH];
  if (!::GetNamedPipeHandleState(m_hPipe, NULL, NULL, NULL, NULL, pszUserName, _MAX_PATH))
    ThrowNamedPipeException();
  
  return pszUserName;
}

DWORD CNamedPipe::GetMaxInstances() const
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwMaxInstances = 0;
  if (!::GetNamedPipeInfo(m_hPipe, NULL, NULL, NULL, &dwMaxInstances))
    ThrowNamedPipeException();
                                      
  return dwMaxInstances;
}

void CNamedPipe::SetMode(BOOL bByteMode, BOOL bBlockingMode)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open

  DWORD dwMode;
  if (bByteMode)
  {
    if (bBlockingMode)
      dwMode = PIPE_READMODE_BYTE | PIPE_WAIT;
    else
      dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
  }
  else
  {
    if (bBlockingMode)
      dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
    else
      dwMode = PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
  }

  if (!::SetNamedPipeHandleState(m_hPipe, &dwMode, NULL, NULL))
    ThrowNamedPipeException();
}

void CNamedPipe::SetMaxCollectionCount(DWORD dwCollectionCount)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open
  assert(IsClientPipe()); //Must be called from the client side

  if (!::SetNamedPipeHandleState(m_hPipe, NULL, &dwCollectionCount, NULL))
    ThrowNamedPipeException();
}

void CNamedPipe::SetCollectionTimeout(DWORD dwDataTimeout)
{
  //Validate our parameters
  assert(IsOpen()); //Pipe must be open
  assert(IsClientPipe()); //Must be called from the client side

  if (!::SetNamedPipeHandleState(m_hPipe, NULL, NULL, &dwDataTimeout))
    ThrowNamedPipeException();
}

DWORD CNamedPipe::Call(LPCTSTR lpszServerName, LPCTSTR lpszPipeName, LPVOID lpInBuffer, DWORD dwInBufferSize, LPVOID lpOutBuffer, DWORD dwOutBufferSize, DWORD dwTimeOut)
{
  //What will be the return value
  DWORD dwBytesRead = 0;

  //Construct the canonical pipe name
  CString sPipeName;
  sPipeName.Format(_T("\\\\%s\\PIPE\\%s"), lpszServerName, lpszPipeName);

  if (!::CallNamedPipe(sPipeName, lpInBuffer, dwInBufferSize, lpOutBuffer, dwOutBufferSize, &dwBytesRead, dwTimeOut))
    ThrowNamedPipeException();

  return dwBytesRead;
}

void CNamedPipe::ServerAvailable(LPCTSTR lpszServerName, LPCTSTR lpszPipeName, DWORD dwTimeOut)
{
  //Construct the canonical pipe name
  CString sPipeName;
  sPipeName.Format(_T("\\\\%s\\PIPE\\%s"), lpszServerName, lpszPipeName);

  if (!::WaitNamedPipe(sPipeName, dwTimeOut))
    ThrowNamedPipeException();
}
