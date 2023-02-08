#include "Property.h"
#include <stdarg.h>

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

	va_list Args;
	va_start(Args, PropertyType);
	switch (PropertyType)
	{
	case EPropertyType::Unknow:
		break;
	case EPropertyType::Sprite:
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
{}

void SProperty::RenderPropertySpriteLayer()
{}

void SProperty::RenderPropertySpriteBlock()
{
	if (CachedSpriteBlock.expired() || CachedSprite.expired())
	{
		return;
	}

	const float TextWidth = ImGui::CalcTextSize("A").x;
	const float TextHeight = ImGui::GetTextLineHeightWithSpacing();
	std::shared_ptr<FSprite> Sprite = CachedSprite.lock();
	std::shared_ptr<FSpriteBlock> SpriteBlock = CachedSpriteBlock.lock();

	ImGui::Dummy(ImVec2(0.0f, TextHeight * 0.5f));
	ImGui::Text("Size :");
	ImGui::Separator();

	if (ImGui::InputTextEx("Name ", NULL, SpriteBlockNameBuffer, IM_ARRAYSIZE(SpriteBlockNameBuffer), ImVec2(TextWidth * 20.0f, TextHeight), ImGuiInputTextFlags_None))
	{
		SpriteBlock->Name = SpriteBlockNameBuffer;
	}

	ImGui::Dummy(ImVec2(0.0f, TextHeight * 1.0f));
	ImGui::Text("Offset :");
	ImGui::Separator();

	ImVec2 OffsetMin = Sprite->Size - Sprite->Pivot;
	ImVec2 OffsetMax = Sprite->Pivot + SpriteBlock->ImageSprite->Size;

	ImGui::SliderFloat("X ", &SpriteBlock->Offset.x, -OffsetMin.x + 1.0f, OffsetMax.x - 1.0f, "%.0f");
	ImGui::SliderFloat("Y ", &SpriteBlock->Offset.y, -OffsetMin.y + 1.0f, OffsetMax.y - 1.0f, "%.0f");
	ImGui::Dummy(ImVec2(0.0f, TextHeight * 1.0f));
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
