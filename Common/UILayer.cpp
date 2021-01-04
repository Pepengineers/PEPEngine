#include "UILayer.h"

#include "GDescriptorHeap.h"


#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"


// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


namespace PEPEngine::Common
{
	

	void UILayer::SetupRenderBackend()
	{
		srvMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, globalCountFrameResources);


		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(this->hwnd);
		ImGui_ImplDX12_Init(device->GetDXDevice().Get(), globalCountFrameResources,
		                    DXGI_FORMAT_R8G8B8A8_UNORM, srvMemory.GetDescriptorHeap()->GetDirectxHeap(),
		                    srvMemory.GetCPUHandle(), srvMemory.GetGPUHandle());

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
	}

	void UILayer::Initialize()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		
		//ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		//

		ImGuiIO& io = ImGui::GetIO(); //(void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		
		SetupRenderBackend();
	}

	UILayer::UILayer(float width, float height, const HWND hwnd) : RenderPass(width, height), hwnd(hwnd)
	{
		Initialize();
		CreateDeviceObject();
	}

	UILayer::~UILayer()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void UILayer::Render(std::shared_ptr<GCommandList> cmdList)
	{
		cmdList->SetDescriptorsHeap(&srvMemory);

		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();


		
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList->GetGraphicsCommandList().Get());
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		
	}

	void UILayer::Update()
	{
	}

	void UILayer::ChangeRenderTargetSize(float newWidth, float newHeight)
	{
		width = newWidth;
		height = newHeight;
	}

	void UILayer::CreateDeviceObject()
	{
		ImGui_ImplDX12_CreateDeviceObjects();
	}

	void UILayer::Invalidate()
	{
		ImGui_ImplDX12_InvalidateDeviceObjects();
	}

	
	LRESULT UILayer::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
			return true;
	}
}
