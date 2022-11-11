#include "Sprite.h"
#include "Core\Utils.h"
#include "Core\Image.h"
#include "Viewer\Viewer.h"
#include "Viewer\Windows\ImageList.h"

void SSprite::Initialize()
{
	bIncludeInWindows = true;
	Name = "Sprite Editor";

	std::shared_ptr<SViewer> Viewer = std::dynamic_pointer_cast<SViewer>(Parent);
	std::shared_ptr<SImageList> ImageList = Viewer ? std::dynamic_pointer_cast<SImageList>(Viewer->GetWindow(EWindowsType::ImageList)) : nullptr;
	if (ImageList)
	{
		ImageList->OnSelectedImage.AddSP(std::dynamic_pointer_cast<SSprite>(shared_from_this()), &SSprite::OnSelectedFileImage);
	}
}

void SSprite::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Sprite Editor", &bOpen, ImGuiWindowFlags_HorizontalScrollbar);
	if (Image)
	{
		ImGui::Text("pointer = %p", Image->Texture);
		ImGui::Text("size = %d x %d", Image->Width, Image->Height);
		Image->Draw();
	}
	ImGui::End();
}

void SSprite::OnSelectedFileImage(const std::filesystem::directory_entry& Path)
{
	Image = Utils::LoadImage(Path.path().string());
}
