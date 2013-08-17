#include "stdafx.h"
#include "LogWindow.h"

CLogWindow::CLogWindow(void)
{
}

CLogWindow::~CLogWindow(void)
{
}

static LPTSTR ClassName = _T("Log Window");

void CLogWindow::OnCreate()
{
	// OnCreate is called automatically during window creation when a
	// WM_CREATE message received.

	// Tasks such as setting the icon, creating child windows, or anything
	// associated with creating windows are normally performed here.

	TRACE(_T("OnCreate\n"));
}

void CLogWindow::OnDestroy()
{
	// End the application when the window is destroyed
	::PostQuitMessage(0);
}

void CLogWindow::OnDraw(CDC* pDC)
{
	// OnDraw is called automatically whenever a part of the
	// window needs to be repainted.
}

void CLogWindow::OnInitialUpdate()
{
	// OnInitialUpdate is called after the window is created.
	// Tasks which are to be done after the window is created go here.

	TRACE(_T("OnInitialUpdate\n"));
}

void CLogWindow::OnSize()
{
	// Force the window to be repainted during resizing
	Invalidate();	
}

void CLogWindow::PreCreate(CREATESTRUCT& cs)
{
	cs.lpszClass = ClassName;		// Window Class
	cs.cx = 600;
	cs.cy = 800;
}

LRESULT CLogWindow::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// This function is our message procedure. We process the messages for
	// the view window here.  Unprocessed messages are passed on for
	//  default processing.

	// Handle message but do default processing
#	define HANDLE_MSG(msg, fn) case msg: fn(); break

	// Handle message but do default processing, the handler needs wParam and lParam
#	define HANDLE_MSG_WL(msg, fn) case msg: fn(wParam, lParam); break
	
	// Handle message and return 0 (processed)
#	define HANDLE_MSG_R(msg, fn) case msg: fn(); return 0

	// Handle message and return 0 (processed), the handler needs wParam and lParam
#	define HANDLE_MSG_WL_R(msg, fn) case msg: fn(wParam, lParam); return 0

	switch(uMsg)
	{
	HANDLE_MSG_R(WM_DESTROY, OnDestroy);
	HANDLE_MSG(WM_SIZE, OnSize);
	}

	// pass unhandled messages on for default processing
	return WndProcDefault(uMsg, wParam, lParam);
}
