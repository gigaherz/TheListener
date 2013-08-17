#pragma once

// Crappy replacement for MFC's CTime, needed by CTrayIconPosition

class CTime
{
	DWORD m_ticks;
	
	CTime(DWORD ticks)
	{
		m_ticks = ticks;
	}

public:

	CTime(void)
	{
		m_ticks = ::GetTickCount();
	}

	~CTime(void)
	{
	}
	
	CTime operator -(CTime b)
	{
		return CTime(b.m_ticks - m_ticks);
	}

	int GetTotalSeconds()
	{
		return m_ticks / 1000;
	}

	static CTime GetCurrentTime()
	{
		return CTime();
	}

};

