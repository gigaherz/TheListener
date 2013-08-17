#include "stdafx.h"
#include "SysFont.h"
#include "Hyperlink.h"
#include "shellapi.h"

LPCTSTR WindowClassName = _T("STATIC_HYPER");

CHyperlink::CHyperlink()
{	
	handcursor = ::LoadCursor(NULL, IDC_HAND);

	defaultFont = CSysFont::Instance.GetDefaultDialogFont();
}

void CHyperlink::PreCreate(CREATESTRUCT &cs)
{
	cs.lpszClass = WindowClassName;
}

void CHyperlink::PreRegisterClass(WNDCLASS &wc)
{
	wc.lpszClassName = WindowClassName;
	wc.hCursor = handcursor; 
}

void CHyperlink::OnDraw(CDC* pDC)
{
	CRect rect = GetClientRect();

	CFont dfont = CFont(defaultFont);
	LOGFONT lf = dfont.GetLogFont();
	
	lf.lfUnderline = TRUE;

	CFont font;
	font.CreateFontIndirect(&lf);
				
	pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(0,0,200));
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(m_Url.c_str(), m_Url.length(), &rect, DT_VCENTER | DT_CENTER);
	
	font.DeleteObject();
}

LRESULT CHyperlink::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)  
	{
	case WM_LBUTTONDOWN:
		if (((UINT)::ShellExecute(NULL, _T("open"), m_Url.c_str(), NULL, NULL, SW_SHOWNORMAL)) <= 32)
			MessageBeep(0);
		return 0;
	case WM_MOUSEMOVE:
		Invalidate();
		break;
	case WM_SETCURSOR:
		if (handcursor)
			::SetCursor(handcursor);
	}
	
	// pass unhandled messages on for default processing
	return WndProcDefault(uMsg, wParam, lParam);
}
