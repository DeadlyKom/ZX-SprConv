#pragma once

#include <memory>
#include <string>
#include "imgui.h"

class SWindow : public std::enable_shared_from_this<SWindow>
{
public:
	SWindow()
		: TickCounter(0)
		, Parent(nullptr)
	{}

	void NativeInitialize(std::shared_ptr<SWindow> _Parent) 
	{
		Parent = _Parent;
		Initialize();
	}
	virtual void Initialize() {}
	virtual void Render()
	{
		bAppearing = ++TickCounter < 3;
	}
	virtual void Update() {}
	virtual void PreDestroy() {}

	virtual void SetOpen(bool _bOpen) { bOpen = _bOpen; }
	virtual void Open() { bOpen = true; }
	virtual void Close() { bOpen = false; }
	virtual bool IsWindowAppearing() const { return bAppearing; }
	virtual bool IsOpen() const { return bOpen; }
	virtual bool IsIncludeInWindows() const { return bIncludeInWindows; }
	virtual std::string GetName() const { return Name; }

protected:
	bool bAppearing = true;
	bool bOpen = true;
	bool bIncludeInWindows = false;
	
	int32_t TickCounter;

	std::shared_ptr<SWindow> Parent;
	std::string Name;
};
