#pragma once

#include <CoreMinimal.h>
#include "Viewer.h"

class SViewChild : public SWindow
{
public:
	std::shared_ptr<SViewer> GetParent() const;
};