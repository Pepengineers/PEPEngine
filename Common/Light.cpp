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
				lightData.PositionWorld = gameObject->GetTransform()->GetWorldPosition();
				lightData.DirectionWorld = DirectionWorld;
				lightData.PositionView = PositionView;
				lightData.DirectionView = DirectionView;
				lightData.Color = Color;
				lightData.SpotlightAngle = SpotlightAngle;
				lightData.Range = Range;
				lightData.Intensity = Intensity;
				lightData.Enabled = Enabled;
				lightData.Selected = Selected;
				lightData.Type = Type;
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
