#include "pch.h"
#include "Light.h"
#include "GameObject.h"
#include "Transform.h"

namespace DX
{
	namespace Common
	{
		void Light::Update()
		{
			if (NumFramesDirty > 0)
			{
				auto pos = gameObject->GetTransform()->GetWorldPosition();
				
				lightData.PositionWorld = Vector4(pos.x, pos.y, pos.z, 1);
				lightData.DirectionWorld = DirectionWorld;
				lightData.PositionView = PositionView;
				lightData.DirectionView = DirectionView;
				lightData.Color = Color;
				lightData.SpotlightAngle = SpotlightAngle;
				lightData.Range = Range;
				lightData.Intensity = Intensity;
				lightData.Enabled = Enabled;
				lightData.Selected = Selected;
				lightData.Type = type;
				NumFramesDirty--;
			}
		}

		void Light::PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList)
		{
		}

		LightData Light::GetData() const
		{
			return lightData;
		}	
	}
}
