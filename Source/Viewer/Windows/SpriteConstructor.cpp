#include "SpriteConstructor.h"
#include "Core\Image.h"
#include "Core\Utils.h"
#include "Core\AppFramework.h"

#define ATTRIBUTE_GRID          1 << 0
#define GRID					1 << 1
#define PIXEL_GRID              1 << 2
#define FORCE_NEAREST_SAMPLING  1 << 31

namespace
{
	ImVec2 VisibleSizeArray[7] =
	{
		ImVec2(16.0f, 16.0f),
		ImVec2(24.0f, 24.0f),
		ImVec2(32.0f, 32.0f),
		ImVec2(48.0f, 48.0f),
		ImVec2(64.0f, 64.0f),
		ImVec2(96.0f, 96.0f),
		ImVec2(128.0f, 128.0f),
	};
}

namespace Shader
{
	/* Constant buffer used in pixel shader. Size must be multiple of 16 bytes.
	 * Layout must match the layout in the pixel shader. */
	struct PIXEL_CONSTANT_BUFFER
	{
		float GridColor[4];
		float GridWidth[2];
		int	  Flags;
		float TimeCounter;
		float BackgroundColor[3];
		int   Dummy;
		float TextureSize[2];
		float GridSize[2];
	};
}

SSpriteConstructor::SSpriteConstructor()
	: ScaleVisible(2)
{}

void SSpriteConstructor::NativeInitialize(FNativeDataInitialize Data)
{
	SWindow::NativeInitialize(Data);

	// save pointers to DirectX device and context
	Device = Data.Device;
	Device->AddRef();
	DeviceContext = Data.DeviceContext;
	DeviceContext->AddRef();

	//
	ID3DBlob* PixelShaderBlob;
	ID3DBlob* ErrorBlob;

	// 'grid' pixel shader
	{
		const std::string PS = FAppFramework::Get().LoadShaderResource(IDR_PS_GRID);
		if (FAILED(D3DCompile(PS.c_str(), PS.size(), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &PixelShaderBlob, &ErrorBlob)))
		{
			std::string Error((const char*)ErrorBlob->GetBufferPointer());
			IMGUI_DEBUG_LOG("ImGuiTexInspect pixel shader failed. Diagnostic:\n%s\n", Error.c_str());
			ErrorBlob->Release();
			return;
		}

		if (Device->CreatePixelShader(PixelShaderBlob->GetBufferPointer(), PixelShaderBlob->GetBufferSize(), NULL, &PS_Grid) != S_OK)
		{
			PixelShaderBlob->Release();
			return;
		}
		PixelShaderBlob->Release();

		// create pixel shader constant buffer
		{
			D3D11_BUFFER_DESC desc;
			desc.ByteWidth = sizeof(Shader::PIXEL_CONSTANT_BUFFER);
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			Device->CreateBuffer(&desc, NULL, &PCB_Grid);
		}
	}
}

void SSpriteConstructor::Initialize()
{
	bIncludeInWindows = true;
	Name = "Sprite Constructor";

	ImageEmpty = Utils::LoadImageFromResource(IDB_TEMPLATE_GRID, TEXT("PNG"));
}

void SSpriteConstructor::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("SpriteConstructor", &bOpen);

	RenderSpriteList();

	ImGui::End();
}

void SSpriteConstructor::OnDrawCallback(const ImDrawList* ParentList, const ImDrawCmd* CMD)
{
	if (PS_Grid == nullptr || PCB_Grid == nullptr)
	{
		return;
	}
	const ImVec4 GridColor = { 0.025f, 0.025f, 0.15f, 0.0f };
	const ImVec4 BackgroundColor = { 0.0f, 1.0f, 0.0f, 0.0f };

	FImage* Image = static_cast<FImage*>(CMD->UserCallbackData);
	// map the pixel shader constant buffer and fill values
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		if (DeviceContext->Map(PCB_Grid, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource) != S_OK)
		{
			return;
		}
		// transfer shader options from shaderOptions to our backend specific pixel shader constant buffer
		Shader::PIXEL_CONSTANT_BUFFER* ConstantBuffer = (Shader::PIXEL_CONSTANT_BUFFER*)MappedResource.pData;
		memcpy(ConstantBuffer->GridColor, &GridColor, sizeof(GridColor));
		memcpy(ConstantBuffer->BackgroundColor, &BackgroundColor, sizeof(float) * 3);
		memcpy(ConstantBuffer->TextureSize, &Image->Size, sizeof(Image->Size));
		ConstantBuffer->Flags = FORCE_NEAREST_SAMPLING;

		DeviceContext->Unmap(PCB_Grid, 0);
	}

	// activate shader and buffer
	DeviceContext->PSSetShader(PS_Grid, NULL, 0);
	DeviceContext->PSSetConstantBuffers(0, 1, &PCB_Grid);
}

void SSpriteConstructor::RenderSpriteList()
{
	HandleKeyboardInputs();
	ImGui::PushAllowKeyboardFocus(true);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(50.0f, 65.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));

	const ImVec2 VisibleSize = VisibleSizeArray[ScaleVisible];
	const ImVec4 BackgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	const ImVec4 TintColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);

	const ImGuiStyle& Style = ImGui::GetStyle();
	const float WindowVisible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

	std::vector<FSprite>& Sprites = GetParent()->GetSprites();
	const uint32_t SpriteNum = uint32_t(Sprites.size());
	for (uint32_t Index = 0; Index < SpriteNum; ++Index)
	{
		ImGui::PushID(Index);
		FSprite& Sprite = Sprites[Index];
		if (Sprite.IsValid())
		{
			if (Sprite.Draw("Test", ImageEmpty, VisibleSize))
			{
				GetParent()->SetSelectedSprite(Index);
			}
		}
		else
		{
			ImGui::ImageButton(Sprite.Name.c_str(), ImageEmpty->GetShaderResourceView(), VisibleSize, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), BackgroundColor, TintColor);
		}

		const float LastButton_x2 = ImGui::GetItemRectMax().x;
		const float NextButton_x2 = LastButton_x2 + Style.ItemSpacing.x + VisibleSize.x; // Expected position if next button was on same line
		if (Index + 1 < SpriteNum && NextButton_x2 < WindowVisible_x2)
		{
			ImGui::SameLine();
		}
		ImGui::PopID();
	}

	ImGui::PopStyleVar(3);

	ImGui::PopAllowKeyboardFocus();
}

void SSpriteConstructor::HandleKeyboardInputs()
{
	ImGuiIO& IO = ImGui::GetIO();
	const bool bHovered = ImGui::IsWindowHovered();

	const bool Shift = IO.KeyShift;
	const bool Ctrl = IO.ConfigMacOSXBehaviors ? IO.KeySuper : IO.KeyCtrl;
	const bool Alt = IO.ConfigMacOSXBehaviors ? IO.KeyCtrl : IO.KeyAlt;

	if (bHovered && Ctrl && !Shift && !Alt && IO.MouseWheel != 0.0f)
	{
		ScaleVisible = (uint32_t)ImClamp<float>(ScaleVisible + IO.MouseWheel, 0.0f, 6.0f);
	}

	if (!ImGui::IsWindowFocused())
	{
		return;
	}
}
