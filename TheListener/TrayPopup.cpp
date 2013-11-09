///////////////////////////////
// View.cpp

#include "stdafx.h"
#include "SysFont.h"
#include "TrayPopup.h"
#include "TrayApp.h"
#include "resource.h"

#define MSG_TRAYICON (WM_USER+0)

static LPTSTR ClassName = _T("Tray Popup");

void CTrayPopup::CreateTrayIcon()
{
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = m_hWnd;
    nid.uID = IDW_MAIN;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = MSG_TRAYICON;
	nid.hIcon = (HICON) (::LoadImage (GetModuleHandle(NULL), MAKEINTRESOURCE (IDW_MAIN), IMAGE_ICON,
		::GetSystemMetrics (SM_CXSMICON), ::GetSystemMetrics (SM_CYSMICON), 0));

	lstrcpy(nid.szTip, _T("The Listener"));
		
	m_PositionFinder.InitializePositionTracking(m_hWnd, IDW_MAIN);

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void CTrayPopup::RemoveTrayIcon()
{
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = m_hWnd;
    nid.uID = IDW_MAIN;

    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void CTrayPopup::Minimize()
{
    ShowWindow(SW_HIDE);
    m_IsMinimized = true;
}

void CTrayPopup::Restore()
{
	CPoint point; 
	
    ShowWindow(SW_SHOW);

	CRect windowRect = GetWindowRect();
	int w = windowRect.Width();
	int h = windowRect.Height();

	int mx = ::GetSystemMetrics(SM_CXMAXIMIZED);
	int my = ::GetSystemMetrics(SM_CYMAXIMIZED);

	int x = mx - w - 20;
	int y = my - h - 20;
	
	if(!m_PositionFinder.GetTrayIconPosition(point))
	{
		x = point.x - w/2;
	}

	SetWindowPos(NULL, x, y, w, h, SWP_NOSIZE); 

    m_IsMinimized = false;
}

void CTrayPopup::OnAbout()
{
	MessageBox(_T("Tray Example: Demonstrates minimizing a window to the tray."), _T("About Tray Example"), MB_OK | MB_ICONINFORMATION);
}

void CTrayPopup::OnCreate()
{
	// OnCreate is called automatically during window creation when a
	// WM_CREATE message received.

	// Tasks such as setting the icon, creating child windows, or anything
	// associated with creating windows are normally performed here.

	CreateTrayIcon();

	CRect cr = GetClientRect();

	defFont = CSysFont::Instance.GetDefaultDialogFont();
	
	int cy = 8;

	const CTrayApp& thisApp = GetRunningApp();

	for(auto it = thisApp.contexts.begin(); it != thisApp.contexts.end(); it++)
	{
		CLoggerContext* ctx = *it;
		CButton* btn = new CButton();
		btn->Create(this);
		btn->SetWindowTextW(ctx->GetPipeName());
		btn->SetWindowPos(NULL, 8, cy, cr.Width()-16, 60, 0);
		btn->SetFont(&defFont);
		btn->SetWindowLongPtr(GWLP_USERDATA, (LONG_PTR)ctx);
		
		buttons[ctx] = btn;
		
		cy += 68;
	}
	
	SCROLLINFO nfo;
	nfo.cbSize = sizeof(SCROLLINFO);
	nfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	nfo.nMin = 0;
	nfo.nMax = cy;
	nfo.nPage = cr.Height();
	nfo.nPos = 0;	
	SetScrollInfo(SB_VERT, nfo, TRUE);

	lastVScroll = 0;
	
	TRACE(_T("OnCreate\n"));
}

BOOL CTrayPopup::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// OnCommand responds to menu and and toolbar input

	UNREFERENCED_PARAMETER(lParam);

	switch(LOWORD(wParam))
	{
	case IDM_MINTOTRAY:
		Minimize();
		return TRUE;
	case IDM_FILE_EXIT:
		// End the application
		PostQuitMessage(0);
		return TRUE;
	case IDM_HELP_ABOUT:
		OnAbout();
		return TRUE;
	}

	return FALSE;
}

void CTrayPopup::OnDestroy()
{
	RemoveTrayIcon();
	
	// End the application when the window is destroyed
	::PostQuitMessage(0);
}

void CTrayPopup::OnDraw(CDC* pDC)
{
	// OnDraw is called automatically whenever a part of the
	// window needs to be repainted.
}

void CTrayPopup::OnInitialUpdate()
{
	// OnInitialUpdate is called after the window is created.
	// Tasks which are to be done after the window is created go here.

	TRACE(_T("OnInitialUpdate\n"));
}

void CTrayPopup::OnSize()
{
	// Force the window to be repainted during resizing
	Invalidate();
	
	CRect cr = GetClientRect();
	
	const CTrayApp& thisApp = GetRunningApp();

	for(auto it = thisApp.contexts.begin(); it != thisApp.contexts.end(); it++)
	{
		CLoggerContext* ctx = *it;

		CButton* btn = buttons[ctx];
		
		btn->SetWindowPos(NULL, 0, 0, cr.Width()-16, 60, SWP_NOMOVE);
	}
		
	SCROLLINFO nfo;
	nfo.cbSize = sizeof(SCROLLINFO);
	nfo.fMask = SIF_RANGE | SIF_PAGE;
	nfo.nMin = 0;
	nfo.nMax = 68+68+68+8;
	nfo.nPage = cr.Height();	
	SetScrollInfo(SB_VERT, nfo, TRUE);

}

void CTrayPopup::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{
	// For a NOTIFYICONDATA with uVersion= 0, wParam and lParam have the following values:
	// The wParam parameter contains the identifier of the taskbar icon in which the event occurred.
	// The lParam parameter holds the mouse or keyboard message associated with the event.
    if (wParam != IDW_MAIN)
		return;

	if (lParam == WM_LBUTTONUP)
    {
        Restore();
    }
    else if (lParam == WM_RBUTTONUP)
    {
		CMenu TopMenu(IDM_MINIMIZED);
		CMenu* pSubMenu = TopMenu.GetSubMenu(0);

        SetForegroundWindow();
		CPoint pt = GetCursorPos();
		UINT uSelected = pSubMenu->TrackPopupMenu(TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, this, NULL);

		switch (uSelected)
		{
		case IDM_MIN_RESTORE:
			Restore();
			break;
		case IDM_MIN_ABOUT:
			OnAbout();
			break;
		case IDM_MIN_EXIT:
			Destroy();
			break;
		}
    }
}

void CTrayPopup::PreCreate(CREATESTRUCT& cs)
{
	cs.lpszClass = ClassName;		// Window Class
	cs.dwExStyle = WS_EX_CLIENTEDGE;		// Extended style
}

HWND CTrayPopup::Create(CWnd* pParent)
{
	DWORD dwStyle = WS_BORDER | WS_POPUP | WS_THICKFRAME | WS_VSCROLL;
	
	int x = 50;	  // top x
	int y = 50;	  // top y
	int cx = 300; // width
	int cy = 100; // height

	return CreateEx(WS_EX_CLIENTEDGE, ClassName, _T(""), dwStyle, x, y, cx, cy, NULL, NULL);
}

LRESULT CTrayPopup::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	HANDLE_MSG_WL(MSG_TRAYICON, OnTrayIcon);
	HANDLE_MSG_WL(WM_VSCROLL, OnVScrollBar);

	case WM_COMMAND:
		if(lParam != NULL)
		{
			switch(HIWORD(wParam))
			{
			case BN_CLICKED:
				
				LONG_PTR ctx = ::GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);

				GetRunningApp().ShowLoggerWindow((CLoggerContext*)ctx);

				break;
			}
		}
		break;

	case WM_SYSCOMMAND:
		switch (wParam)
		{
		HANDLE_MSG(SC_MINIMIZE, Minimize);
		}
		break;

	case WM_QUIT:		
		RemoveTrayIcon();
		break;
	}

	// pass unhandled messages on for default processing
	return WndProcDefault(uMsg, wParam, lParam);
}

void CTrayPopup::OnVScrollBar(WPARAM wParam, LPARAM lParam)
{
	int sp;

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_TRACKPOS | SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(SB_VERT, si);

	sp = si.nPos;

	switch(LOWORD(wParam))
	{
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		sp = si.nTrackPos;
		break;

	case SB_TOP:
		sp = si.nMin;
		break;
			
	case SB_BOTTOM:
		sp = si.nMax;
		break;
			
	case SB_PAGEUP:
		sp -= si.nPage;
		break;
	
	case SB_PAGEDOWN:
		sp += si.nPage;
		break;
		
	case SB_LINEUP:
		sp --;
		break;
		
	case SB_LINEDOWN:
		sp ++;
		break;
	}

	if(sp != si.nPos)
	{
		SetScrollPos(SB_VERT, sp, TRUE);
	}
	
	if(lastVScroll != sp)
	{
		ScrollWindow(0, lastVScroll - sp, NULL, NULL);
		lastVScroll = sp;
	}
}