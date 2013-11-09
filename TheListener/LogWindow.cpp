#include "stdafx.h"
#include "SysFont.h"
#include "LogWindow.h"

static LPTSTR ClassName = _T("Log Window");

CLogWindow::CLogWindow(CLoggerContext *ctx)
	: m_Context(ctx)
{
}

CLogWindow::~CLogWindow(void)
{
}

void CLogWindow::ItemAdded()
{
	SendMessage(WM_USER+1);
}

void CLogWindow::OnCreate()
{
	// OnCreate is called automatically during window creation when a
	// WM_CREATE message received.

	// Tasks such as setting the icon, creating child windows, or anything
	// associated with creating windows are normally performed here.
	
	listFont = CSysFont::Instance.GetDefaultDialogFont();
	
	CRect rect = GetClientRect();
	
	m_ListBox.Create(this);
	m_ListBox.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), 0);
	m_ListBox.SetFont(&listFont);	
	m_ListBox.SetCount(m_Context->m_Items.size());
		
	m_Context->m_LogWindow = this;
}

void CLogWindow::OnDestroy()
{
	m_Context->m_LogWindow = NULL;
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
}

void CLogWindow::OnSize()
{
	CRect rect = GetClientRect();

	m_ListBox.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), 0);

	// Force the window to be repainted during resizing
	Invalidate();
    m_ListBox.Invalidate();
}

void CLogWindow::PreCreate(CREATESTRUCT& cs)
{
	cs.lpszClass = ClassName;		// Window Class
	cs.x = 0;
	cs.y = 0;
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

	case WM_USER+1:
		{
			m_ListBox.SetCount(m_Context->m_Items.size());
			m_ListBox.SetCurSel(m_Context->m_Items.size()-1);
		}
		break;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT mi = (LPMEASUREITEMSTRUCT)lParam;

			CDC *dc = GetDC();
			
			CFont def = CSysFont::Instance.GetDefaultDialogFont();

			dc->SelectObject(&def);
			
			TEXTMETRIC tm; 
            dc->GetTextMetrics(tm); 

			mi->itemHeight = tm.tmHeight + 4;
	
			ReleaseDC(dc);
		}
		return TRUE;
		
	case WM_DRAWITEM:
		{
			int margin;
            DWORD fg;

			CBrush *brush;

			CSize size = CSize();
			CRect rcMessage;
			CRect rcSource;

			LPDRAWITEMSTRUCT di = (LPDRAWITEMSTRUCT) lParam; 

			CRect rcItem = di->rcItem;
			CDC *dc = FromHandle(di->hDC);

			CLogItem *item = m_Context->m_Items.at(di->itemID);

            // If there are no list box items, skip this message. 
            if (di->itemID == -1) 
            { 
                break; 
            } 
 
            // Draw the bitmap and text for the list box item. Draw a 
            // rectangle around the bitmap if it is selected. 
            switch (di->itemAction) 
            {
                case ODA_FOCUS:
                case ODA_SELECT: 
                case ODA_DRAWENTIRE: 
					
                    if (di->itemState & ODS_SELECTED) 
						brush = FromHandle((HBRUSH) (COLOR_HIGHLIGHT+1));
					else
						brush = FromHandle((HBRUSH) (COLOR_WINDOW+1));

                    if (di->itemState & ODS_SELECTED)
                        fg = GetSysColor(COLOR_HIGHLIGHTTEXT);
                    else
                        fg = GetSysColor(COLOR_WINDOWTEXT);

					dc->FillRect(rcItem, brush);
					
					TEXTMETRIC tm;
                    dc->GetTextMetrics(tm); 
					
                    margin = (rcItem.bottom - rcItem.top - tm.tmHeight) / 2;
  			
					if(item->m_Source.size() > 0)
					{
						size = dc->GetTextExtentPoint32W(item->m_Source.c_str(), item->m_Source.size());
					}

                    dc->SetTextColor(0x00000000);
                    dc->SetBkMode(TRANSPARENT);

                    rcMessage = CRect(rcItem.left + margin, rcItem.top + margin, rcItem.right - margin, rcItem.bottom - margin);
					rcMessage.right -= min(size.cx, rcItem.Width() / 4);
					dc->DrawText(item->m_Message.c_str(), item->m_Message.size(), rcMessage, DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS);

					if (item->m_Source.size() > 0)
					{
                        rcSource = CRect(rcMessage.left, rcMessage.top, rcItem.right - margin, rcMessage.bottom);
						dc->DrawText(item->m_Source.c_str(), item->m_Source.size(), rcSource, DT_SINGLELINE | DT_RIGHT | DT_PATH_ELLIPSIS);
					}

                    // Is the item selected? 
                    if (di->itemState & ODS_SELECTED) 
                    {
                        dc->DrawFocusRect(rcItem); 
                    } 
                    break;
            }
		}
		return TRUE;
	}

	// pass unhandled messages on for default processing
	return WndProcDefault(uMsg, wParam, lParam);
}
