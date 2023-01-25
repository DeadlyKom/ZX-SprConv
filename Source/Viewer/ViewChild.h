#pragma once

#include <CoreMinimal.h>
#include "Viewer.h"

class SViewChild : public SWindow
{
public:
	std::shared_ptr<SViewer> GetParent() const;

	template <typename T>
	std::shared_ptr<T> GetWindow(EWindowsType Type)
	{
		std::shared_ptr<SViewer> Viewer = GetParent();
		return Viewer ? std::dynamic_pointer_cast<T>(Viewer->GetWindow(Type)) : nullptr;
	}
};
