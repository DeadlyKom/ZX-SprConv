#pragma once

#include <CoreMinimal.h>
#include "Viewer\ViewChild.h"

class SProperty : public SViewChild
{
public:
	virtual void Initialize() override;
	virtual void Render() override;
};
