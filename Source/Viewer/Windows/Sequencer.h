#pragma once

#include <CoreMinimal.h>

class SSequencer : public SWindow
{
public:
	virtual void Initialize() override;
	virtual void Render() override;
};
