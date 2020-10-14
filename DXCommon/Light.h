#pragma once
#include "Component.h"
#include "d3dUtil.h"
#include "ShaderBuffersData.h"

namespace DX
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
			Vector4   PositionWorld;
			Vector4   DirectionWorld;
			Vector4   PositionView;
			Vector4   DirectionView;
			Vector4   Color;
			float    SpotlightAngle;
			float    Range;
			float    Intensity;
			bool    Enabled;
			bool    Selected;

			
			LightData GetData() const;			
		};
	}
}
