#include "UILayer.h"


#include "d3dUtil.h"
#include "GDescriptorHeap.h"
#include "GDevice.h"


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
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

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

	static void ShowDockingDisabledMessage()
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
		ImGui::Text("Set io.ConfigFlags |= ImGuiConfigFlags_DockingEnable in your code, or ");
		ImGui::SameLine(0.0f, 0.0f);
		if (ImGui::SmallButton("click here"))
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}

	void UILayer::RenderMainWindowAsDockPanel()
	{
		auto& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		}
		else
		{
			ShowDockingDisabledMessage();
		}
	}

	void UILayer::Render(std::shared_ptr<GCommandList> cmdList)
	{
		cmdList->SetDescriptorsHeap(&srvMemory);

		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		RenderMainWindowAsDockPanel();
		//ImGui::Begin("Cross");
	
		ImVec2 s = { ImGui::GetWindowPos().x + width/2 - 64  , ImGui::GetWindowPos().y + height/2 - 64 };

		ImGui::SetWindowPos("Cross",s);
		ImGui::Begin("Cross",nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
		ImGui::Text("+");               // Display some text (you can use a format strings too)
		ImGui::End();
		ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);

		std::string kills = std::string("Kills:") + std::to_string((int)killcount);
		std::string timer = std::string("Time:") + std::to_string( 60 - (int)this->timer);
		ImGui::Text(&kills[0]);
		ImGui::Text(&timer[0]);
		ImGui::End();

		if(this->timer == 60)
		{
			ImVec2 s = { ImGui::GetWindowPos().x + width / 2 - 64  , ImGui::GetWindowPos().y + height / 2 - 64 };

			ImGui::SetWindowPos("End", s);
			ImGui::Begin("End", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration );
			ImGui::Text("THE END");
			std::string kills = std::string("Kills:") + std::to_string((int)killcount);
			ImGui::Text(&kills[0]);
			ImGui::End();
		}
		
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

	void UILayer::setCount(float count, float time)
	{
		killcount = count;
		time > 60 ? this->timer = 60 : this->timer = time;
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
