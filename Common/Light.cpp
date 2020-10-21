#include "pch.h"
#include "Light.h"
#include "GameObject.h"
#include "Transform.h"

namespace PEPEngine
{
	namespace Common
	{
		void Light::Update()
		{
			if(gameObject->GetTransform()->IsDirty())
			{
				NumFramesDirty = globalCountFrameResources;
			}
			
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

		LightData& Light::GetData()
		{
			return lightData;
		}	
	}
}
