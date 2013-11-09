#pragma once

class COwnerDrawnListBox : public CListBox
{
public:
	COwnerDrawnListBox(void) {}
	~COwnerDrawnListBox(void) {}
	virtual void PreCreate(CREATESTRUCT& cs);

	virtual void SetCount(int count);
};

