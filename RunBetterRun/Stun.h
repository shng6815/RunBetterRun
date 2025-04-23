#pragma once
#include "AItem.h"

#define STUN_TIME 2.5f

class Stun: public AItem
{
	virtual BOOL Action(void) override;

public:
	Stun(FPOINT pos) { Init(pos); }
	virtual HRESULT Init(FPOINT pos) override;
};