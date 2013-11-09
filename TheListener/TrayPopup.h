/////////////////////////////////
// View.h

#ifndef TRAYPOPUP_H
#define TRAYPOPUP_H

#include "TrayIconPosition.h"
#include "Hyperlink.h"
#include "LoggerContext.h"

class CTrayPopup : public CWnd
{
public:
	CTrayPopup() : m_IsMinimized(FALSE) {}
	virtual ~CTrayPopup() {}
	void Minimize();
	void Restore();
	
	void CreateTrayIcon();
	void RemoveTrayIcon();
	
	virtual HWND Create(CWnd* pParent = NULL);

protected:
	virtual void OnAbout();
	virtual void OnCreate();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnDestroy();
	virtual void OnDraw(CDC* pDC);	
	virtual void OnInitialUpdate();
	virtual void OnSize();
	virtual void OnTrayIcon(WPARAM wParam, LPARAM lParam);
	virtual void PreCreate(CREATESTRUCT& cs);
	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void OnVScrollBar(WPARAM wParam, LPARAM lParam);

private:
	BOOL m_IsMinimized;

	CTrayIconPosition m_PositionFinder;

	std::map<CLoggerContext*, CButton*> buttons;
	
	CScrollBar scrollBar;

	CFont defFont;

	int lastVScroll;
};

#endif
