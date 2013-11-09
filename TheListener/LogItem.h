#pragma once
class CLogItem
{

public:
	CLogItem(LPCTSTR message, LPCTSTR source) { m_Message = message; m_Source = source; }
	~CLogItem(void) {}

	std::wstring m_Message;
	std::wstring m_Source;
};

