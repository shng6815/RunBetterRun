#pragma once
#include "AItem.h"

class Insight: public AItem
{
	virtual BOOL Action(void) override;

public:
	Insight(FPOINT pos) { Init(pos); }
	virtual HRESULT Init(FPOINT pos) override;
};