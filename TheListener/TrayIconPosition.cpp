// TrayIconPosition.cpp: implementation of the CTrayIconPosition class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "TrayIconPosition.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrayIconPosition::CTrayIconPosition()
{
	m_hWndOfIconOwner = NULL;
	m_iTrayIconID = -1;
	m_prDefaultPrecision = High;
	m_iPrecisions[0] = 60; //seconds for low precision mode
	m_iPrecisions[1] = 30; //seconds for medium precision mode
	m_iPrecisions[2] = 10; //seconds for high precision mode

	m_osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx( &m_osVer );
}

CTrayIconPosition::~CTrayIconPosition()
{

}

void CTrayIconPosition::InitializePositionTracking(HWND hwndOfIconOwner, int iIconID)
{
	m_hWndOfIconOwner = hwndOfIconOwner;
	m_iTrayIconID = iIconID;
}

BOOL CTrayIconPosition::GetTrayIconPositionVisualScan(CPoint& a_ptPoint, Precision a_ePrec)
{
	//let's find the time from last tray icon position calculation
	int iTotalSec = (m_tLastUpdate - CTime::GetCurrentTime()).GetTotalSeconds();
	if(iTotalSec < 0)
	{
		iTotalSec = -iTotalSec;
	}

	Precision prec;
	if(a_ePrec == Default)
	{
		prec = m_prDefaultPrecision;
	}
	else
	{
		prec = a_ePrec;
	}

	BOOL bUpdateRequired = FALSE;
	if(prec == Low && iTotalSec > m_iPrecisions[0]	  ||
	   prec == Medium && iTotalSec > m_iPrecisions[1] ||
	   prec == High && iTotalSec > m_iPrecisions[2]	    )
	   {
		bUpdateRequired = TRUE;
	   }

	if(bUpdateRequired)
	{
		m_rtRectangleOfTheTray = GetTrayWndRect();

		HICON icon = (HICON) (::LoadImage (GetModuleHandle(NULL), MAKEINTRESOURCE (IDW_MAIN), IMAGE_ICON,
									::GetSystemMetrics (SM_CXSMICON), ::GetSystemMetrics (SM_CYSMICON), 0));
		
		if(FindOutPositionOfIcon(icon) == TRUE)
		{
			//we were able to find position of icon in tray - that's GREAT!
			m_tLastUpdate = CTime::GetCurrentTime();
			a_ptPoint = m_ptPosition;
			return TRUE;
		}
		else
		{
			//We were unable to locate our icon - this is not a tragedy - usually there is a solution:

			if(m_osVer.dwPlatformId == VER_PLATFORM_WIN32_NT && m_osVer.dwMajorVersion >= 5)
			{
				//We're under Win XP
				//It's quite possible that icon is hidden now let's return Left-Center point of tray - it's most likely WinXP hide-tray-icons button.
				a_ptPoint = m_rtRectangleOfTheTray.CenterPoint();
				a_ptPoint.x = m_rtRectangleOfTheTray.left + 6;
			}
			else
			{
				//we're not under XP - I have no idea why but we were unable to locate our icon - let's retunr center of the tray area
				a_ptPoint = m_rtRectangleOfTheTray.CenterPoint();
			}
			return FALSE;
		}
		
	}
	else
	{
		a_ptPoint = m_ptPosition;
	}

	return TRUE;
}

BOOL CTrayIconPosition::GetTrayIconPosition(CPoint& a_ptPoint, TrackType a_eTrackType, Precision a_ePrec)
{
	if(m_hWndOfIconOwner == NULL || m_iTrayIconID == -1)
	{
		return -1;
	}
	
	//remove it if you need exact point of left/top corner of the icon
	const int iOffsetX = 4;
	const int iOffsetY = 6;

	CRect rcDirectRect;
	
	switch (a_eTrackType) 
	{
	case UseBothTechniquesDirectPrefered:
		if( FindOutPositionOfIconDirectly(rcDirectRect) == FALSE )
		{
			return GetTrayIconPositionVisualScan(a_ptPoint, a_ePrec);
		}
		
		a_ptPoint.x = rcDirectRect.left + iOffsetX;
		a_ptPoint.y = rcDirectRect.top  + iOffsetY;
		return TRUE;

	case UseBothTechniquesVisualScanPrefered:
		if( GetTrayIconPositionVisualScan(a_ptPoint, a_ePrec) == FALSE )
		{
			BOOL bOK = FindOutPositionOfIconDirectly(rcDirectRect);

			a_ptPoint.x = rcDirectRect.left + iOffsetX;
			a_ptPoint.y = rcDirectRect.top  + iOffsetY;
			return bOK;
		}
		return TRUE;

	case UseDirectOnly:
		{
			BOOL bOK = FindOutPositionOfIconDirectly(rcDirectRect);

			a_ptPoint.x = rcDirectRect.left + iOffsetX;
			a_ptPoint.y = rcDirectRect.top  + iOffsetY;
			return bOK;
		}

	case UseVisualScanOnly:
		return GetTrayIconPositionVisualScan(a_ptPoint, a_ePrec);
		
	default:
		//ASSERT(0);
		break;
	}
	

	return FALSE;
}

/*
	This class performs check of current position of tray icon olny from time to time.
	You can decide if you want low , med or high precision.
	Default values of precisions are:
	Low - 60 seconds
	Med - 30 seconds
	High - 10 secods

	For example Low precision at 60 seconds means that this class make a full calculation
	of tray icon and this value expires in next 60 seconds. If application will request
	position of tray icon in time shorter than 60 seconds - there will be no recalculation
	and previous calculated value will be used.
*/
void CTrayIconPosition::SetPrecisionTimeOuts(int iLowSec, int iMedSec, int iHighSec)
{
	m_iPrecisions[0] = iLowSec;
	m_iPrecisions[1] = iMedSec;
	m_iPrecisions[2] = iHighSec;
}

/*
	This function and GetTrayWndRect() are not written by me - I found this code on codeproject - not remember exacly where.
	I was forced to modify this code a bit since some assumptions taken by author were going too far.
*/
BOOL CALLBACK CTrayIconPosition::FindTrayWnd(HWND hwnd, LPARAM lParam)
{    
	TCHAR szClassName[256];
    GetClassName(hwnd, szClassName, 255);    // Did we find the Main System Tray? If so, then get its size and quit
	if (_tcscmp(szClassName, _T("TrayNotifyWnd")) == 0)    
	{        
		HWND* pWnd = (HWND*)lParam;
		*pWnd = hwnd;
        return FALSE;    
	}    
	
	//Oryginal code I found on Internet were seeking here for system clock and it was assumming that clock is on the right side of tray.
	//After that calculated size of tray was adjusted by removing space occupied by clock.
	//This is not a good idea - some clocks are ABOVE or somewhere else on the screen. I found that is far safer to just ignore clock space.
	
	return TRUE;
}

BOOL CALLBACK CTrayIconPosition::FindToolBarInTrayWnd(HWND hwnd, LPARAM lParam)
{    
	TCHAR szClassName[256];
    GetClassName(hwnd, szClassName, 255);    // Did we find the Main System Tray? If so, then get its size and quit
	if (_tcscmp(szClassName, _T("ToolbarWindow32")) == 0)    
	{        
		HWND* pWnd = (HWND*)lParam;
		*pWnd = hwnd;
        return FALSE;    
	}    
	return TRUE;
}

HWND CTrayIconPosition::GetTrayNotifyWnd(BOOL a_bSeekForEmbedToolbar)
{
	HWND hWndTrayNotifyWnd = NULL;
	
    HWND hWndShellTrayWnd = FindWindow(_T("Shell_TrayWnd"), NULL);
    if (hWndShellTrayWnd)    
	{        
		EnumChildWindows(hWndShellTrayWnd, CTrayIconPosition::FindTrayWnd, (LPARAM)&hWndTrayNotifyWnd);   
		
		if(hWndTrayNotifyWnd && IsWindow(hWndTrayNotifyWnd))
		{
			HWND hWndToolBarWnd = NULL;
			EnumChildWindows(hWndTrayNotifyWnd, CTrayIconPosition::FindToolBarInTrayWnd, (LPARAM)&hWndToolBarWnd);   
			if(hWndToolBarWnd)
			{
				return hWndToolBarWnd;
			}
		}

		return hWndTrayNotifyWnd;
	}  

	return hWndShellTrayWnd;
}

CRect CTrayIconPosition::GetTrayWndRect()
{    
	CRect rect(0,0,0,0);
	
	HWND hWndTrayWnd = GetTrayNotifyWnd(FALSE);
	if(hWndTrayWnd)
	{
		GetWindowRect(hWndTrayWnd, &rect);
		return rect;
	}

	int nWidth  = GetSystemMetrics(SM_CXSCREEN);
    int nHeight = GetSystemMetrics(SM_CYSCREEN);
    rect.SetRect(nWidth-40, nHeight-20, nWidth, nHeight);

	return rect;
}

BOOL CTrayIconPosition::FindOutPositionOfIconDirectly7(CRect& a_rcIcon)
{
    RECT rect;
	NOTIFYICONIDENTIFIER id = {0};

	// Requires Windows 7 or Server 2008 R2

	// Will not work on XP or earlier
	if(m_osVer.dwMajorVersion < 6 || m_osVer.dwMajorVersion == 6 && m_osVer.dwMinorVersion < 1)
		return FALSE;
	
    id.cbSize = sizeof(id);
	id.hWnd = m_hWndOfIconOwner;
	id.uID = m_iTrayIconID;
    if(FAILED(::Shell_NotifyIconGetRect(&id, &rect)))
		return FALSE;

	a_rcIcon = CRect(rect);

    return TRUE;
}

//First tracking method: attaches to Tray process and reads data directly, is fast and reliable but will fail if user uses non standard tray software
//It was suggested by Neal Andrews with VB example: http://www.codeproject.com/shell/ctrayiconposition.asp?select=999036&forumid=14631&df=100#xx999036xx
//Ported to C++ by Ireneusz Zielinski
BOOL CTrayIconPosition::FindOutPositionOfIconDirectly(CRect& a_rcIcon)
{
	if(FindOutPositionOfIconDirectly7(a_rcIcon))
		return TRUE;

	//first of all let's find a Tool bar control embed in Tray window
	HWND hWndTray = GetTrayNotifyWnd(TRUE);
    if (hWndTray == NULL)    
	{
		return FALSE;
	}

	//now we have to get an ID of the parent process for system tray
	DWORD dwTrayProcessID = -1;
	GetWindowThreadProcessId(hWndTray, &dwTrayProcessID);
	if(dwTrayProcessID <= 0)
	{
		return FALSE;
	}

	HANDLE hTrayProc = OpenProcess(PROCESS_ALL_ACCESS, 0, dwTrayProcessID);
	if(hTrayProc == NULL)
	{
		return FALSE;
	}
 
	//now we check how many buttons is there - should be more than 0
	int iButtonsCount = SendMessage(hWndTray, TB_BUTTONCOUNT, 0, 0);

	//We want to get data from another process - it's not possible to just send messages like TB_GETBUTTON with a localy
	//allocated buffer for return data. Pointer to localy allocated data has no usefull meaning in a context of another
	//process (since Win95) - so we need to allocate some memory inside Tray process.
	//We allocate sizeof(TBBUTTON) bytes of memory - because TBBUTTON is the biggest structure we will fetch. But this buffer
	//will be also used to get smaller pieces of data like RECT structures.
	LPVOID lpData = VirtualAllocEx(hTrayProc, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);
	if( lpData == NULL || iButtonsCount < 1 )
	{
		CloseHandle(hTrayProc);
		return FALSE;
	}

	BOOL bIconFound = FALSE;

	for(int iButton=0; iButton<iButtonsCount; iButton++)
	{
		//first let's read TBUTTON information about each button in a task bar of tray

		DWORD dwBytesRead = -1;
		TBBUTTON buttonData;
		SendMessage(hWndTray, TB_GETBUTTON, iButton, (LPARAM)lpData);
		ReadProcessMemory(hTrayProc, lpData, &buttonData, sizeof(TBBUTTON), &dwBytesRead);
		if(dwBytesRead < sizeof(TBBUTTON))
		{
			continue;
		}

		//now let's read extra data associated with each button: there will be a HWND of the window that created an icon and icon ID
		DWORD dwExtraData[2] = { 0,0 };
		ReadProcessMemory(hTrayProc, (LPVOID)buttonData.dwData, dwExtraData, sizeof(dwExtraData), &dwBytesRead);
		if(dwBytesRead < sizeof(dwExtraData))
		{
			continue;
		}

		HWND hWndOfIconOwner = (HWND) dwExtraData[0];
		int  iIconId		 = (int)  dwExtraData[1];
		
		if(hWndOfIconOwner != m_hWndOfIconOwner || iIconId != m_iTrayIconID)
		{
			continue;
		}
		
		//we found our icon - in WinXP it could be hidden - let's check it:
		if( buttonData.fsState & TBSTATE_HIDDEN )
		{
			break;
		}

		//now just ask a tool bar of rectangle of our icon
		RECT rcPosition = {0,0};
		SendMessage(hWndTray, TB_GETITEMRECT, iButton, (LPARAM)lpData);
		ReadProcessMemory(hTrayProc, lpData, &rcPosition, sizeof(RECT), &dwBytesRead);

		if(dwBytesRead < sizeof(RECT))
		{
			continue;
		}

		MapWindowPoints(hWndTray, NULL, (LPPOINT)&rcPosition, 2);
		a_rcIcon = rcPosition;
		
		bIconFound = TRUE;
		break;
	}

	if(bIconFound == FALSE)
	{
		a_rcIcon = GetTrayWndRect(); //we failed to detect position of icon - let's return fail safe cooridinates of system tray
	}

	VirtualFreeEx(hTrayProc, lpData, NULL, MEM_RELEASE);
	CloseHandle(hTrayProc);

	return bIconFound;	
}

//as mentioned in article Shell_NotifyIcon likes to alter icons while putting them to system tray
//so even black icon can be not in 100 black
BOOL CTrayIconPosition::CheckIfColorIsBlackOrNearBlack(COLORREF crColor)
{
	//code improved by Harald according to
	//http://www.codeproject.com/shell/ctrayiconposition.asp?forumid=14631&select=786426&df=100#xx786426xx

	return !(((DWORD)crColor)&0xFCFCFC);
}

BOOL CTrayIconPosition::FindOutPositionOfIcon(HICON icon)
{
	//First: let's set a BLANK icon in the tray.
	NOTIFYICONDATA nid; 
	nid.cbSize = sizeof(nid); 
	nid.hWnd = m_hWndOfIconOwner; 
	nid.uID = m_iTrayIconID; 
	nid.uFlags = NIF_ICON; 
	nid.hIcon = icon;
	Shell_NotifyIcon(NIM_MODIFY, &nid);
	BOOL bWeFoundIt = FALSE;
	
	HDC hdcScreen = GetDC(NULL);
	
	//since tray window can be strethed verticaly let's scan this area in few steps starting from the bottom to the top.
	for(int iy = m_rtRectangleOfTheTray.bottom-3; iy > m_rtRectangleOfTheTray.top; iy--)
	{
		int iNoOfPixelsInLine=0;
		for(int ix=m_rtRectangleOfTheTray.left;ix<m_rtRectangleOfTheTray.right;ix++)
		{
			COLORREF crPixel = GetPixel(hdcScreen, ix, iy);
			COLORREF crPixel2 = GetPixel(hdcScreen, ix, iy-2);
			COLORREF crPixel3 = GetPixel(hdcScreen, ix, iy+2);
			if(CheckIfColorIsBlackOrNearBlack(crPixel)  && CheckIfColorIsBlackOrNearBlack(crPixel2)  && CheckIfColorIsBlackOrNearBlack(crPixel3))
			{
				iNoOfPixelsInLine++;
			}
			else
			{
				iNoOfPixelsInLine = 0;
			}
			
			if(iNoOfPixelsInLine > 8)
			{
				//well I think we found it!
				m_ptPosition.x = ix-1;
				m_ptPosition.y = iy-6;
				bWeFoundIt = TRUE;
				break;
			}
		}
		
		if(bWeFoundIt)
		{
			break;
		}
	}

	ReleaseDC(NULL, hdcScreen);
	return bWeFoundIt;
}

void CTrayIconPosition::RestoreTrayIcon(HICON icon)
{
	  NOTIFYICONDATA nid; 
	  nid.cbSize = sizeof(nid); 
	  nid.hWnd = m_hWndOfIconOwner; 
	  nid.uID = m_iTrayIconID; 
	  nid.uFlags = NIF_ICON; 
	  nid.hIcon = icon;
	  Shell_NotifyIcon(NIM_MODIFY, &nid);	
}

void CTrayIconPosition::Invalidate()
{
	m_tLastUpdate = CTime();
}
