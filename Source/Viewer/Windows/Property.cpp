#include "Property.h"
#include <stdarg.h>

#include "Core\Utils.h"

#include "Viewer\Viewer.h"

SProperty::SProperty()
	: VisiblePropertyType(EPropertyType::Unknow)
{}

void SProperty::Initialize()
{
	bIncludeInWindows = true;
	Name = "Property";
}

void SProperty::Render()
{
	if (!IsOpen())
	{
		Close();
		return;
	}

	ImGui::Begin("Property", &bOpen);
	switch (VisiblePropertyType)
	{
	case EPropertyType::Unknow:											break;
	case EPropertyType::Sprite:			RenderPropertySprite();			break;
	case EPropertyType::SpriteLayer:	RenderPropertySpriteLayer();	break;
	case EPropertyType::SpriteBlock:	RenderPropertySpriteBlock();	break;
	default:															break;
	}
	ImGui::End();
}

void SProperty::SetProperty(EPropertyType PropertyType, ...)
{
	VisiblePropertyType = PropertyType;

	// reset old values
	CachedSprite.reset();
	CachedSpriteBlock.reset();

	// read args
	va_list Args;
	va_start(Args, PropertyType);
	switch (PropertyType)
	{
	case EPropertyType::Unknow:
		break;
	case EPropertyType::Sprite:
		CachedSprite = va_arg(Args, std::weak_ptr<FSprite>);
		InitPropertySprite();
		break;
	case EPropertyType::SpriteLayer:
		break;
	case EPropertyType::SpriteBlock:
		CachedSprite = va_arg(Args, std::weak_ptr<FSprite>);
		CachedSpriteBlock = va_arg(Args, std::weak_ptr<FSpriteBlock>);
		InitPropertySpriteBlock();
		break;
	default:
		break;
	}
	va_end(Args);
}

void SProperty::RenderPropertySprite()
{
	if (CachedSprite.expired())
	{
		return;
	}

	const float TextWidth = ImGui::CalcTextSize("A").x;
	const float TextHeight = ImGui::GetTextLineHeightWithSpacing();
	std::shared_ptr<FSprite> Sprite = CachedSprite.lock();

	ImGui::Dummy(ImVec2(0.0f, TextHeight * 0.25f));
	ImGui::Text("Sprite");
	ImGui::Dummy(ImVec2(0.0f, TextHeight * 0.5f));
	ImGui::Separator();

	if (ImGui::InputTextEx("Name ", NULL, SpriteNameBuffer, IM_ARRAYSIZE(SpriteNameBuffer), ImVec2(TextWidth * 20.0f, TextHeight), ImGuiInputTextFlags_None))
	{
		Sprite->Name = SpriteNameBuffer;
	}

	ImGui::Dummy(ImVec2(0.0f, TextHeight * 1.0f));
	ImGui::Text("Size :");
	ImGui::Separator();

	const ImGuiInputTextFlags InputNumberTextFlags = ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_CallbackEdit;
	if (ImGui::InputTextEx("Width ", NULL, SpriteWidthBuffer, IM_ARRAYSIZE(SpriteWidthBuffer), ImVec2(TextWidth * 10.0f, TextHeight), InputNumberTextFlags, &SViewer::TextEditNumberCallback, (void*)&Sprite->Size.x))
	{
		Sprite->Size.x = ImFloor(Sprite->Size.x);
	}
	if (ImGui::InputTextEx("Height ", NULL, SpriteHeightBuffer, IM_ARRAYSIZE(SpriteHeightBuffer), ImVec2(TextWidth * 10.0f, TextHeight), InputNumberTextFlags, &SViewer::TextEditNumberCallback, (void*)&Sprite->Size.y))
	{
		Sprite->Size.y = ImFloor(Sprite->Size.y);
	}

	ImGui::Dummy(ImVec2(0.0f, TextHeight * 1.0f));
	ImGui::Text("Pivot :");
	ImGui::Separator();

	ImGui::SliderFloat("X ", &Sprite->Pivot.x, 0, ImClamp(Sprite->Size.x, 0.0f, ImMax(Sprite->Size.x - 1.0f, 0.0f)), "%.0f");
	ImGui::SliderFloat("Y ", &Sprite->Pivot.y, 0, ImClamp(Sprite->Size.y, 0.0f, ImMax(Sprite->Size.y - 1.0f, 0.0f)), "%.0f");
	ImGui::Dummy(ImVec2(0.0f, TextHeight * 1.0f));

	// ToDo добавить выбор формата конверсии
}

void SProperty::RenderPropertySpriteLayer()
{}

void SProperty::RenderPropertySpriteBlock()
{
	if (CachedSprite.expired() || CachedSpriteBlock.expired())
	{
		return;
	}

	const float TextWidth = ImGui::CalcTextSize("A").x;
	const float TextHeight = ImGui::GetTextLineHeightWithSpacing();
	std::shared_ptr<FSprite> Sprite = CachedSprite.lock();
	std::shared_ptr<FSpriteBlock> SpriteBlock = CachedSpriteBlock.lock();

	ImGui::Dummy(ImVec2(0.0f, TextHeight * 0.25f));
	ImGui::Text("Sprite Block");
	ImGui::Dummy(ImVec2(0.0f, TextHeight * 0.5f));
	ImGui::Separator();

	if (ImGui::InputTextEx("Name ", NULL, SpriteBlockNameBuffer, IM_ARRAYSIZE(SpriteBlockNameBuffer), ImVec2(TextWidth * 20.0f, TextHeight), ImGuiInputTextFlags_None))
	{
		SpriteBlock->Name = SpriteBlockNameBuffer;
	}

	ImGui::Dummy(ImVec2(0.0f, TextHeight * 1.0f));
	ImGui::Text("Offset :");
	ImGui::Separator();

	ImVec2 OffsetMin = Sprite->Pivot + SpriteBlock->ImageSprite->Size;
	ImVec2 OffsetMax = Sprite->Size - Sprite->Pivot;

	ImGui::SliderFloat("X ", &SpriteBlock->Offset.x, -OffsetMin.x + 1.0f, OffsetMax.x - 1.0f, "%.0f");
	ImGui::SliderFloat("Y ", &SpriteBlock->Offset.y, -OffsetMin.y + 1.0f, OffsetMax.y - 1.0f, "%.0f");
	ImGui::Dummy(ImVec2(0.0f, TextHeight * 1.0f));
}

void SProperty::InitPropertySprite()
{
	if (CachedSprite.expired())
	{
		return;
	}

	std::shared_ptr<FSprite> Sprite = CachedSprite.lock();
	memcpy(SpriteNameBuffer, Sprite->Name.c_str(), Sprite->Name.size() + 1);
	sprintf(SpriteWidthBuffer, "%i\n", int(Sprite->Size.x));
	sprintf(SpriteHeightBuffer, "%i\n", int(Sprite->Size.y));
}

void SProperty::InitPropertySpriteBlock()
{
	if (CachedSpriteBlock.expired())
	{
		return;
	}

	std::shared_ptr<FSpriteBlock> SpriteBlock = CachedSpriteBlock.lock();
	memcpy(SpriteBlockNameBuffer, SpriteBlock->Name.c_str(), SpriteBlock->Name.size() + 1);
}
