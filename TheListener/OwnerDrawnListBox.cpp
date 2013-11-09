#include "stdafx.h"
#include "OwnerDrawnListBox.h"

void COwnerDrawnListBox::PreCreate(CREATESTRUCT& cs)
{
	CListBox::PreCreate(cs);
	
	cs.style |= LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_NODATA | LBS_DISABLENOSCROLL /*| LBS_EXTENDEDSEL*/ | WS_VSCROLL | WS_CHILD | WS_VISIBLE;
	cs.dwExStyle |= WS_EX_CLIENTEDGE;
}

void COwnerDrawnListBox::SetCount(int count)
{
	SendMessage(LB_SETCOUNT, count, 0);
}