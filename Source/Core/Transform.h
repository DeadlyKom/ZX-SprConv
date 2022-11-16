#pragma once

struct Transform2D
{
	ImVec2 Scale;
	ImVec2 Translate;

	/* Transform a vector by this transform.  Scale is applied first */
	ImVec2 operator*(const ImVec2& rhs) const
	{
		return ImVec2(Scale.x * rhs.x + Translate.x, Scale.y * rhs.y + Translate.y);
	}

	/* Return an inverse transform such that transform.Inverse() * transform * vector == vector*/
	Transform2D Inverse() const
	{
		ImVec2 inverseScale(1.0f / Scale.x, 1.0f / Scale.y);
		return { inverseScale, ImVec2(-inverseScale.x * Translate.x, -inverseScale.y * Translate.y) };
	}
};
