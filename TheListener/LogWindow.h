#pragma once

class CLogWindow :
	public CWnd
{
public:
	CLogWindow(void);
	virtual ~CLogWindow(void);
	
protected:
	virtual void OnCreate();
	virtual void OnDestroy();
	virtual void OnDraw(CDC* pDC);	
	virtual void OnInitialUpdate();
	virtual void OnSize();
	virtual void PreCreate(CREATESTRUCT& cs);
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

