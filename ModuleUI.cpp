#include "Networks.h"
#include "ModuleUI.h"


extern HWND hwnd;                                // Window handle
extern ID3D11Device        *g_pd3dDevice;        // Direct3d11 device pointer
extern ID3D11DeviceContext *g_pd3dDeviceContext; // Direct3d11 device context pointer

bool ModuleUI::init()
{
	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	// Setup style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);


	return true;
}

bool ModuleUI::preUpdate()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	return true;
}

bool ModuleUI::gui()
{
	ImGui::Begin("Logging / Profiling");

	if (ImGui::BeginTabBar("Tab bar"))
	{
		if (ImGui::BeginTabItem("Log"))
		{
			uint32 logEntryCount = getLogEntryCount();
			for (uint32 entryIndex = 0; entryIndex < logEntryCount; ++entryIndex)
			{
				LogEntry entry = getLogEntry(entryIndex);
				if (entry.type == LOG_TYPE_WARN) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
				}
				else if (entry.type == LOG_TYPE_ERROR) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
				}
				else if (entry.type == LOG_TYPE_DEBUG) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 1.0f, 1.0f));
				}
				ImGui::TextWrapped("%.4f:\t%s", entry.time, entry.message);
				if (entry.type == LOG_TYPE_WARN ||
					entry.type == LOG_TYPE_ERROR ||
					entry.type == LOG_TYPE_DEBUG)
				{
					ImGui::PopStyleColor();
				}
			}

			//ImGui::SetScrollHere(1.0f);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Profiling"))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
			ImGui::Text("DEBUG CYCLE COUNTS");
			ImGui::Separator();
			for (int i = 0; i < DebugCycleCounter_Count; ++i)
			{
				if (DebugCycleCountersFront[i].hitCount > 0)
				{
					ImGui::Text(" - %16s: %9I64u cy | %4d hits | %9I64u cy/hit",
						DebugCycleCountersFront[i].label,
						DebugCycleCountersFront[i].cycleCount,
						DebugCycleCountersFront[i].hitCount,
						DebugCycleCountersFront[i].cycleCount / DebugCycleCountersFront[i].hitCount);
				}
			}
			ImGui::PopStyleColor();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	
	ImGui::End();

	return true;
}

bool ModuleUI::postUpdate()
{
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return true;
}

bool ModuleUI::cleanUp()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	return true;
}

void ModuleUI::setInputsEnabled(bool enabled)
{
	inputsEnabled = enabled;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT ModuleUI::HandleWindowsEvents(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (inputsEnabled)
	{
		return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
	}

	return ERROR_SUCCESS;
}
