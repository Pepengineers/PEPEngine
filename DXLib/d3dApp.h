#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "d3dUtil.h"
#include "GameTimer.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"d3d11.lib")

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "RuntimeObject.lib")

#include "d2d1_3.h"
#include "dwrite.h"
#include <dxgi1_6.h>
#include "d3d11on12.h"
#include "d3d11.h"
#include "DXAllocator.h"
#include "Lazy.h"

using Microsoft::WRL::ComPtr;

namespace DXLib
{
	class Window;
	class GDevice;

	class D3DApp
	{
	protected:

		D3DApp(HINSTANCE hInstance);
		D3DApp(const D3DApp& rhs) = delete;
		D3DApp& operator=(const D3DApp& rhs) = delete;
		virtual ~D3DApp();

	public:

		void Destroy() const;

		bool IsTearingSupported() const;

		std::shared_ptr<Window> CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight,
		                                           bool vSync = true);

		void DestroyWindow(const std::wstring& windowName);

		void DestroyWindow(std::shared_ptr<Window> window);

		std::shared_ptr<Window> GetWindowByName(const std::wstring& windowName);

		static void Quit(int exitCode = 0);

		void Flush();

		UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

		GameTimer* GetTimer();

		static D3DApp& GetApp();

		HINSTANCE AppInst() const;
		std::shared_ptr<Window> MainWnd() const;
		float AspectRatio() const;

		bool Get4xMsaaState() const;
		void Set4xMsaaState(bool value);

		int Run();

		virtual bool Initialize();
		virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		uint64_t GetFrameCount() const
		{
			return frameCount;
		}

		std::shared_ptr<GDevice> GetDevice();

	protected:

		void InitialDevices();

		static bool CheckTearingSupport();

		virtual void OnResize();
		virtual void Update(const GameTimer& gt) = 0;
		virtual void Draw(const GameTimer& gt) = 0;

	protected:
		WNDCLASS windowClass;
		std::shared_ptr<Window> MainWindow;

		Lazy<bool> m_TearingSupported;

		std::wstring mainWindowCaption = L"d3d App";

		static D3DApp* instance;
		HINSTANCE appInstance = nullptr;

		bool isAppPaused = false;
		bool isMinimized = false;
		bool isMaximized = false;
		bool isResizing = false;
		bool isFullscreen = false;

		bool isM4xMsaa = false;
		UINT m4xMsaaQuality = 0;

		uint64_t frameCount = 0;
		GameTimer timer;

		bool InitMainWindow();

		ComPtr<IDXGIFactory4> dxgiFactory;
		std::shared_ptr<GDevice> mainDevice;
		custom_vector<std::shared_ptr<GDevice>> devices = DXAllocator::CreateVector<std::shared_ptr<GDevice>>();

		bool InitDirect3D();

		void CalculateFrameStats() const;

		void LogAdapters() const;
	};
}
