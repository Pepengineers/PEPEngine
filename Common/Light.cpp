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
		assert(TryReadVariable<float>(jType, "x", &x));
		assert(TryReadVariable<float>(jType, "y", &y));
		assert(TryReadVariable<float>(jType, "z", &z));
		Direction = Vector3(x, y, z);


		jType = jPos["Color"];
		assert(TryReadVariable<float>(jType, "x", &x));
		assert(TryReadVariable<float>(jType, "y", &y));
		assert(TryReadVariable<float>(jType, "z", &z));
		assert(TryReadVariable<float>(jType, "w", &w));
		Color = Vector4(x, y, z, w);


		assert(TryReadVariable<float>(jPos, "SpotlightAngle", &SpotlightAngle));
		assert(TryReadVariable<float>(jPos, "Range", &Range));
		assert(TryReadVariable<float>(jPos, "Intensity", &Intensity));
		assert(TryReadVariable<bool>(jPos, "Enabled", &Enabled));
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
