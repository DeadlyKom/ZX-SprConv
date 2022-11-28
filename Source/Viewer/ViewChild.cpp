#include "ViewChild.h"

std::shared_ptr<SViewer> SViewChild::GetParent() const
{
	return std::dynamic_pointer_cast<SViewer>(Parent);
}
