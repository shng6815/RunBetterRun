#pragma once
#include "AItem.h"

class Display: public AItem
{
	virtual BOOL Action(void) override;

public:
	Display(FPOINT pos) {Init(pos);}
	Display(FPOINT pos,LPCWCH texturePath);

	virtual HRESULT Init(FPOINT pos) override;
};