#pragma once
#include "Component.h"
#include "d3dUtil.h"
#include "ShaderBuffersData.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;
		using namespace Utils;


		class Light : public Component
		{
		private:
			
			int NumFramesDirty = globalCountFrameResources;
			LightData lightData{};
			void Update() override;;
			void PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList) override;
			Matrix view = Matrix::Identity;
			Matrix projection = Matrix::Identity;

		public:
			LightType type;
			Vector4   DirectionWorld;
			Vector4   PositionView;
			Vector4   DirectionView;
			Vector4   Color;
			float    SpotlightAngle;
			float    Range;
			float    Intensity;
			bool    Enabled;
			bool    Selected;

			Light()
				:  DirectionWorld(0, 0, -1, 0)
				, PositionView(0, 0, 0, 1)
				, DirectionView(0, 0, 1, 0)
				, Color(1, 1, 1, 1)
				, SpotlightAngle(45.0f)
				, Range(15)
				, Intensity(1.0f)
				, Enabled(true)
				, Selected(false)
				, type(LightType::Point) {  }
			
			LightData& GetData();			
		};
	}
}
