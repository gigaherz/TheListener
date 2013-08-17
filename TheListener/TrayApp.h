#pragma once

#include "TrayPopup.h"

// Declaration of the CWinApp class
class CTrayApp : public CWinApp
{
public:
    CTrayApp();
    virtual ~CTrayApp() {}
	virtual BOOL InitInstance();
	CTrayPopup& GetView() { return m_View; }
	
	std::vector<CLoggerContext*> contexts;

private:
    CTrayPopup m_View;
};

// returns a reference to the CTrayApp object
inline CTrayApp& GetRunningApp() { return *((CTrayApp*)GetApp()); }
