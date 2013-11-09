////////////////////////////////////////
// SimpleApp.cpp

#include "stdafx.h"
#include "TrayApp.h"


// Definitions for the CTrayApp class
CTrayApp::CTrayApp()
{
    // Constructor for CTrayApp
}

BOOL CTrayApp::InitInstance()
{
    // This function is called automatically when the application starts
	
	// Fill the context list until the config system is implemented
	CLoggerContext *ctx1 = new CLoggerContext();
	ctx1->SetPipeName(_T("\\\\.\\pipe\\ros_logger"));
	contexts.push_back(ctx1);
/*
	CLoggerContext *ctx2 = new CLoggerContext();
	ctx2->SetPipeName(_T("kos_logger"));
	contexts.push_back(ctx2);
*/
    // Create the Window
	m_View.Create();

	return TRUE;
}


void CTrayApp::ShowLoggerWindow(CLoggerContext *ctx)
{
	CLogWindow *wnd = new CLogWindow(ctx);

	windows[ctx] = wnd;

	wnd->Create();

}