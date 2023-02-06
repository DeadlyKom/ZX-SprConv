#pragma once

#include <CoreMinimal.h>
#include "Core\Sprite.h"

#define BUFFER_SIZE_INPUT 32

enum class EToolType;

enum class EWindowsType
{
	ImageList = 0,
	Tools,
	SpriteEditor,
	Palette,
	SpriteConstructor,
	Sequencer,
	Property,
};

struct FViewFlags
{
	bool bAttributeGrid = false;
	bool bGrid = false;
	bool bPixelGrid = true;
	int GridSize[2] = { 16, 16 };

	//
	bool bDontAskMeNextTime_Quit = false;
};

struct FRecentFiles
{
	std::string VisibleName;
};

class SImageList;
class SSequencer;
class SSpriteEditor;
class SSpriteConstructor;

class SViewer : public SWindow
{
	friend SImageList;
	friend SSequencer;
	friend SSpriteEditor;
	friend SSpriteConstructor;

public:
	SViewer();

	std::shared_ptr<SWindow> GetWindow(EWindowsType Type);

	virtual void NativeInitialize(FNativeDataInitialize Data) override;
	virtual void Initialize() override;
	virtual void Render() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Destroy() override;

	bool IsHandTool();
	bool IsMarqueeTool();

private:
	// friend functions
	FViewFlags& GetViewFlags() { return ViewFlags; }

	// sprite functions
	void SetSelectedSprite(uint32_t Index);
	FSprite* GetSelectedSprite();
	FSpriteLayer* GetSelectedLayer(const FSprite& Sprite);

	std::vector<FSprite>& GetSprites();

	// file functions
	void AddFilePath(const std::filesystem::directory_entry& FilePath);

	// tool functions
	bool TrySetTool(EToolType NewToolType);
	void ResetTool();

	// input functions
	void OpenFile_Callback(std::filesystem::path FilePath);

	// show functions
	void ShowMenuFile();
	void ShowMenuSprite();
	void ShowMenuLayer();
	void ShowMenuFrame();
	void ShowMenuView();
	void ShowWindows();

	// window functions
	bool WindowQuitModal();
	bool WindowCreateSpriteModal();

	// create functions
	bool AddSprite(FSprite& NewSprite);
	bool AddSpriteBlock(const std::filesystem::directory_entry& FilePath, const ImRect& MarqueeRect);

	// static functions
	static int TextEditNumberCallback(ImGuiInputTextCallbackData* Data);

	// template functions
	template <typename T>
	T* WindowCast(EWindowsType Type)
	{
		return static_cast<T*>(Windows[Type].get());
	}

	// internal
	FViewFlags ViewFlags;
	std::map<EWindowsType, std::shared_ptr<SWindow>> Windows;

	// tool
	bool bToolChangeLock;
	EToolType LastSelectedTool;

	// recent files
	std::vector<FRecentFiles> RecentFiles;

	// open
	DelegateHandle FileDialogHandle;
	std::vector<std::filesystem::directory_entry> Files;

	// create sprite
	bool bCreateSpriteFirstOpen;
	char CreateSpriteNameBuffer[BUFFER_SIZE_INPUT] = "";
	char CreateSpriteWidthBuffer[BUFFER_SIZE_INPUT] = "";
	char CreateSpriteHeightBuffer[BUFFER_SIZE_INPUT] = "";
	ImVec2 CreateSpriteSize;
	ImVec2 CreateSpritePivot;
	EColorMode CreateSpriteColorMode;

	std::shared_ptr<FImage> ImageRGBA;
	std::shared_ptr<FImage> ImageIndexed;
	std::shared_ptr<FImage> ImageZX;

	// sprite
	int32_t SpriteCounter;
	int32_t CurrentSprite;
	std::vector<FSprite> Sprites;

	// layer
	int32_t LayersCounter;
};
