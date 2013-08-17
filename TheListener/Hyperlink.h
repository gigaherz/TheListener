#pragma once

class CHyperlink : public CWnd
{
public:
	CHyperlink();

	virtual ~CHyperlink() {}

	LPCTSTR GetUrl() { return m_Url.c_str(); }
	void SetUrl(LPCTSTR url) { m_Url = std::wstring(url); }

protected:

	// Overridables
	virtual void PreRegisterClass(WNDCLASS &wc);
	virtual void PreCreate(CREATESTRUCT &cs);
	virtual void OnDraw(CDC* pDC);	
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	std::wstring m_Url;
	
	HCURSOR handcursor;
	CFont defaultFont;
};