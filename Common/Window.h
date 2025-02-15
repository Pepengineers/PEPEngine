#pragma once
#include <Windows.h>
#include <dxgi1_5.h>
#include <memory>
#include <string>
#include <wrl.h>

#include "GDescriptor.h"
#include "GTexture.h"

using namespace Microsoft::WRL;

namespace PEPEngine
{
	namespace Common

	{
		using namespace Allocator;
		using namespace Utils;
		using namespace Graphics;

		class Window
		{
		public:

			HWND GetWindowHandle() const;

			void Destroy();

			const std::wstring& GetWindowName() const;

			int GetClientWidth() const;
			int GetClientHeight() const;


			bool IsVSync() const;
			void SetVSync(bool vSync);


			bool IsFullScreen() const;
			void SetFullscreen(bool fullscreen);

			void Show();

			void Hide();

			UINT GetCurrentBackBufferIndex() const;

			UINT Present();
			void Initialize();

			GRenderTexture* GetCurrentBackBuffer();


			void SetHeight(int height);

			void SetWidth(int width);


			float AspectRatio() const;

			GRenderTexture* GetBackBuffer(UINT i);

			void SetWindowTitle(std::wstring text) const;
		protected:

			friend class D3DApp;

			Window() = delete;
			Window(std::shared_ptr<GDevice> device, WNDCLASS hwnd, const std::wstring& windowName, int clientWidth,
			       int clientHeight, bool vSync);
			virtual ~Window();


			virtual void OnUpdate();
			void CalculateFrameStats();
			virtual void OnRender();
			void ResetTimer();

			virtual void OnResize();

			ComPtr<IDXGISwapChain4> GetSwapChain();


		private:

			int frameCnt = 0;

			HANDLE swapChainEvent;


			Window(const Window& copy) = delete;
			Window& operator=(const Window& other) = delete;

			WNDCLASS windowClass;
			HWND hWnd;
			std::wstring windowName;
			int width;
			int height;
			bool vSync;
			bool fullscreen;


			ComPtr<IDXGISwapChain4> CreateSwapChain();
			ComPtr<IDXGISwapChain4> swapChain;

			std::shared_ptr<GDevice> device;
			std::vector<std::shared_ptr<GTexture>> backBuffers;
			std::vector<std::shared_ptr<GRenderTexture>> renderTargets;
			GDescriptor rtvDescriptors;

			UINT currentBackBufferIndex;

			RECT windowRect;
		};
	}
}
