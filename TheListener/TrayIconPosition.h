// TrayIconPosition.h: interface for the CTrayIconPosition class.
//
//////////////////////////////////////////////////////////////////////

#ifndef TRAYICONPOSITION_H
#define TRAYICONPOSITION_H

#include "Time.h"

class CTrayIconPosition  
{
public:
	typedef enum Precision { Default=0,Low,Medium,High };

	typedef enum TrackType { UseBothTechniquesDirectPrefered, 
							 UseBothTechniquesVisualScanPrefered,
							 UseDirectOnly,
							 UseVisualScanOnly};
	

	//Second tracking method: it detects a tray icon scaning tray area pixel by pixel
	void InitializePositionTracking(HWND a_hwndOfIconOwner, int a_iIconID);
	BOOL GetTrayIconPosition(CPoint& a_ptPoint, TrackType a_eTrackType = UseBothTechniquesDirectPrefered, Precision a_ePrec = Default);
	void RestoreTrayIcon(HICON a_hIcon);

	//Those functions are for use with second tracking method - will cache tracking results
	void Invalidate();
	void SetPrecisionTimeOuts(int iLowSec, int iMedSec, int iHighSec);
	void SetDefaultPrecision(Precision newPrecision) { m_prDefaultPrecision = newPrecision; };
	Precision GetDefaultPrecision(void) { return m_prDefaultPrecision; };

	CTrayIconPosition();
	virtual ~CTrayIconPosition();

protected:
	HICON		m_hIconTemp;
	CRect		m_rtRectangleOfTheTray;
	Precision	m_prDefaultPrecision;
	CTime		m_tLastUpdate;
	CPoint		m_ptPosition;
	int			m_iTrayIconID;
	HWND		m_hWndOfIconOwner;
	int			m_iPrecisions[3];  //This is table of time outs in seconds of low,med and high precision modes.
	OSVERSIONINFO m_osVer;

	//Tracking method: attaches to Tray process and reads data directly, is fast and reliable but will fail if user uses non standard tray software
	BOOL FindOutPositionOfIconDirectly(CRect& a_rcIcon);
	BOOL FindOutPositionOfIconDirectly7(CRect& a_rcIcon);

	BOOL FindOutPositionOfIcon(HICON icon);
	inline BOOL CheckIfColorIsBlackOrNearBlack(COLORREF crColor);
	BOOL GetTrayIconPositionVisualScan(CPoint& a_ptPoint, Precision a_ePrec);
		
	static CRect GetTrayWndRect();
	static HWND  GetTrayNotifyWnd(BOOL a_bSeekForEmbedToolbar);
	static BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK FindToolBarInTrayWnd(HWND hwnd, LPARAM lParam);
};

#endif
