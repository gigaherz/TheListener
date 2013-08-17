#pragma once

class CLoggerContext
{
public:

	CLoggerContext(void)
	{
	}

	virtual ~CLoggerContext(void)
	{
	}

	void SetPipeName(LPCTSTR pipeName) { m_PipeName = std::wstring(pipeName); }
	LPCTSTR getPipeName() { return m_PipeName.c_str(); }

private:
	std::wstring m_PipeName;
};

