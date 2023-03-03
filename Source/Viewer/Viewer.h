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
	ImVec2 GridSettingSize = ImVec2(16.0f, 16.0f);
	ImVec2 GridSettingOffset = ImVec2(0.0f, 0.0f);

	//
	bool bDontAskMeNextTime_Quit = false;
};

struct FRecentFiles
{
	std::string VisibleName;
};

class SImageList;
class SProperty;
class SSequencer;
class SSpriteEditor;
class SSpriteConstructor;

class SViewer : public SWindow
{
	friend SImageList;
	friend SProperty;
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
	std::shared_ptr<FSprite> GetSelectedSprite();
	std::shared_ptr<FSpriteLayer> GetSelectedLayer(const std::shared_ptr<FSprite> Sprite);
	std::vector<std::shared_ptr<FSprite>>& GetSprites();

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
	bool WindowgGridSettingsModal();

	// create functions
	bool AddSprite(std::shared_ptr<FSprite> NewSprite);
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

	// grid settings
	bool bGridSettingsFirstOpen;
	char GridSettingsWidthBuffer[BUFFER_SIZE_INPUT] = "";
	char GridSettingsHeightBuffer[BUFFER_SIZE_INPUT] = "";
	char GridSettingsOffsetXBuffer[BUFFER_SIZE_INPUT] = "";
	char GridSettingsOffsetYBuffer[BUFFER_SIZE_INPUT] = "";
	bool bTmpGrid;
	ImVec2 TmpGridSettingSize;
	ImVec2 TmpGridSettingOffset;

	// sprite
	int32_t SpriteCounter;
	int32_t CurrentSprite;
	std::vector<std::shared_ptr<FSprite>> Sprites;

	// layer
	int32_t LayersCounter;
};
