#pragma once
#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>

namespace DXLib
{
	using namespace Microsoft::WRL;

	class GCommandQueue;

	class GDevice
	{
		std::shared_ptr<GCommandQueue> directCommandQueue;
		std::shared_ptr<GCommandQueue> computeCommandQueue;
		std::shared_ptr<GCommandQueue> copyCommandQueue;
	public:
		GDevice(IDXGIAdapter* adapter);
		~GDevice();;
		ComPtr<ID3D12Device2> dxDevice;

		std::shared_ptr<GCommandQueue> GetCommandQueue(
			D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;

		UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;


		void Flush() const;
	};
}
