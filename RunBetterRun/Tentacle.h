#pragma once
#include "AMonster.h"

class Tentacle : public AMonster
{
	virtual void Action(void) override;
	virtual void Catch(void) override;

public:
	Tentacle(FPOINT pos) { Init(pos); }
	virtual HRESULT Init(FPOINT pos) override;

};

