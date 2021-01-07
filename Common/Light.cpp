#include "pch.h"
#include "Light.h"

#include "Camera.h"
#include "GameObject.h"
#include "Transform.h"

namespace PEPEngine::Common
{
	void Light::Update()
	{
		if (gameObject->GetTransform()->IsDirty())
		{
			NumFramesDirty = globalCountFrameResources;
		}

		if (NumFramesDirty > 0)
		{
			lightData.PositionWorld = gameObject->GetTransform()->GetWorldPosition();
			lightData.DirectionWorld = Direction;
			lightData.Color = Color;
			lightData.SpotlightAngle = SpotlightAngle;
			lightData.Range = Range;
			lightData.Intensity = Intensity;
			lightData.Enabled = Enabled;
			lightData.Selected = Selected;
			lightData.Type = Type;

			if (Type != Point)
			{
				const auto view = GetViewMatrix();
				lightData.PositionView = Vector3::Transform(lightData.PositionWorld, view);
				lightData.DirectionView = Vector3::Transform(lightData.DirectionWorld, view);
			}
			NumFramesDirty--;
		}
	}

	void Light::Render(std::shared_ptr<GCommandList> cmdList)
	{
	}


	Matrix& Light::GetViewMatrix() const
	{
		Vector3 postiton;

		if (Type == Directional)
		{
			postiton = Vector3::One * 150;
		}
		else
		{
			postiton = gameObject->GetTransform()->GetWorldPosition();
		}

		return Matrix(XMMatrixLookAtLH(postiton, Vector3::Zero, Vector3::Up));
	}


	LightData& Light::GetData()
	{
		return lightData;
	}
}
