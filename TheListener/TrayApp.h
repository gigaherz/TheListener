#pragma once

#include "TrayPopup.h"
#include "LogWindow.h"

// Declaration of the CWinApp class
class CTrayApp : public CWinApp
{
public:
    CTrayApp();
    virtual ~CTrayApp() {}
	virtual BOOL InitInstance();
	CTrayPopup& GetView() { return m_View; }
	
	void ShowLoggerWindow(CLoggerContext *ctx);

	std::vector<CLoggerContext*> contexts;
	
	std::map<CLoggerContext*, CLogWindow*> windows;
	
private:
    CTrayPopup m_View;
};

// returns a reference to the CTrayApp object
inline CTrayApp& GetRunningApp() { return *((CTrayApp*)GetApp()); }
