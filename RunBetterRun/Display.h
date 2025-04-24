#pragma once
#include "AItem.h"
#include <map>

class Display: public AItem
{
	static map<DWORD,LPCWCH> ids;
	virtual BOOL Action(void) override;

public:
	Display(FPOINT pos) { Init(pos); }
	Display(FPOINT pos,DWORD id);

	virtual HRESULT Init(FPOINT pos) override;
};