#ifndef SYSFONT_H
#define SYSFONT_H

class CSysFont
{
	NONCLIENTMETRICS metrics;

	CSysFont()
	{
		metrics.cbSize = sizeof(NONCLIENTMETRICS);

		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0);
	}
	
public:

	CFont GetDefaultDialogFont() {
		CFont font;
		font.CreateFontIndirectW(&(metrics.lfMessageFont));
		return font;
	}

	static CSysFont Instance;
};

#endif