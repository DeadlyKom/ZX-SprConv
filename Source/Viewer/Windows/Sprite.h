#pragma once

#include "Core\Window.h"

class SSprite : public SWindow
{
public:
	virtual void Initialize() override;
	virtual void Render() override;
};
