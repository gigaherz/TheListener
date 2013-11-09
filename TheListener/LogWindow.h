#pragma once

#include "LoggerContext.h"

#include "OwnerDrawnListBox.h"

class CLogWindow :
	public CWnd
{
	CLoggerContext *m_Context;

	COwnerDrawnListBox m_ListBox;

	CFont listFont;

public:
	CLogWindow(CLoggerContext *ctx);
	virtual ~CLogWindow(void);

	void ItemAdded();
	
protected:
	virtual void OnCreate();
	virtual void OnDestroy();
	virtual void OnDraw(CDC* pDC);	
	virtual void OnInitialUpdate();
	virtual void OnSize();
	virtual void PreCreate(CREATESTRUCT& cs);
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

