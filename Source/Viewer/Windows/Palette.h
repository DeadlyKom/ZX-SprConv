#pragma once

#include "Core\Window.h"

class SPalette : public SWindow
{
public:
	virtual void Initialize() override;
	virtual void Render() override;
};
