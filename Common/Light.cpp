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

	void Light::Serialize(json& j)
	{
		j["Type"] = ComponentID;

		auto jPos = json();
		jPos["Type"] = Type;

		auto jType = json();
		jType["x"] = Direction.x;
		jType["y"] = Direction.y;
		jType["z"] = Direction.z;

		jPos["Direction"] = jType;

		jType = json();
		jType["x"] = Color.x;
		jType["y"] = Color.y;
		jType["z"] = Color.z;
		jType["w"] = Color.w;
		jPos["Color"] = jType;

		jPos["SpotlightAngle"] = SpotlightAngle;
		jPos["Range"] = Range;
		jPos["Intensity"] = Intensity;
		jPos["Enabled"] = Enabled;

		j["LightData"] = jPos;
	}

	void Light::Deserialize(json& j)
	{
		auto jPos = j["LightData"];
		Type = jPos["Type"];

		float x, y, z, w;

		auto jType = jPos["Direction"];
		(TryReadVariable<float>(jType, "x", &x));
		(TryReadVariable<float>(jType, "y", &y));
		(TryReadVariable<float>(jType, "z", &z));
		Direction = Vector3(x, y, z);


		jType = jPos["Color"];
		(TryReadVariable<float>(jType, "x", &x));
		(TryReadVariable<float>(jType, "y", &y));
		(TryReadVariable<float>(jType, "z", &z));
		(TryReadVariable<float>(jType, "w", &w));
		Color = Vector4(x, y, z, w);


		(TryReadVariable<float>(jPos, "SpotlightAngle", &SpotlightAngle));
		(TryReadVariable<float>(jPos, "Range", &Range));
		(TryReadVariable<float>(jPos, "Intensity", &Intensity));
		(TryReadVariable<bool>(jPos, "Enabled", &Enabled));
	}

	Light::Light(): Type(Spot)
	                , Direction(0, 0, -1)
	                , Color(1, 1, 1, 1)
	                , SpotlightAngle(45.0f)
	                , Range(5)
	                , Intensity(1.0f)
	                , Enabled(true)
	                , Selected(false)
	{
	}


	Matrix Light::GetViewMatrix() const
	{
		const auto focusPosition = gameObject->GetTransform()->GetForwardVector() + gameObject->GetTransform()->GetWorldPosition();
		
		return Matrix(XMMatrixLookAtLH(gameObject->GetTransform()->GetWorldPosition(), focusPosition, gameObject->GetTransform()->GetUpVector()));
	}


	LightData& Light::GetData()
	{
		return lightData;
	}
}
