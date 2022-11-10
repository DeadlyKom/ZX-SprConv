#pragma once

#include <string>
#include "imgui.h"

class SWindow
{
public:
	virtual void Initialize() {}
	virtual void Render() {}
	virtual void Update() {}
	virtual void PreDestroy() {}

	virtual void SetOpen(bool _bOpen) { bOpen = _bOpen; }
	virtual void Open() { bOpen = true; }
	virtual void Close() { bOpen = false; }
	virtual bool IsOpen() const { return bOpen; }
	virtual bool IsIncludeInWindows() const { return bIncludeInWindows; }
	virtual std::string GetName() const { return Name; }

protected:
	bool bOpen = true;
	bool bIncludeInWindows = false;
	
	std::string Name;
};
