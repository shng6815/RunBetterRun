#pragma once
#include "AItem.h"

class Key: public AItem
{
	virtual BOOL Action(void) override;

public:
	Key(FPOINT pos) { Init(pos); }
	virtual HRESULT Init(FPOINT pos) override;
};

