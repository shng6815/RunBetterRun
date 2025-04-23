#pragma once
#include "AItem.h"

class Phone: public AItem
{
	virtual BOOL Action(void) override;

public:
	Phone(FPOINT pos) { Init(pos); }
	virtual HRESULT Init(FPOINT pos) override;
};

