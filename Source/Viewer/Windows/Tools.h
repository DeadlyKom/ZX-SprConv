#pragma once

#include <CoreMinimal.h>

class STools : public SWindow
{
public:
	virtual void Initialize() override;
	virtual void Render() override;

private:
	std::shared_ptr<FImage> ImageMarquee;
};
