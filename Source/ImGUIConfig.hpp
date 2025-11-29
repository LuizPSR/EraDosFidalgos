#pragma once
#include <imgui.h>

inline void SetupImguiFlags(const char *iniPath)
{
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.IniFilename = iniPath;
}

inline void SetupImGuiStyle()
{
	// ayu-dark style by usrnatc from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 5.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(4.0f, 3.0f);
	style.FrameRounding = 5.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 20.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 12.9f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 8.0f;
	style.GrabRounding = 5.0f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 1.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(0.9019608f, 0.7058824f, 0.3137255f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.9019608f, 0.7058824f, 0.3137255f, 0.5019608f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.039215688f, 0.05490196f, 0.078431375f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.078431375f, 0.078431375f, 0.078431375f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.07450981f, 0.09019608f, 0.12941177f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5019608f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.05882353f, 0.07450981f, 0.101960786f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5019608f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.043137256f, 0.05490196f, 0.078431375f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.019607844f, 0.019607844f, 0.019607844f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30980393f, 0.30980393f, 0.30980393f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40784314f, 0.40784314f, 0.40784314f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.24705882f, 0.69803923f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.9019608f, 0.7058824f, 0.3137255f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.56078434f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30980393f, 0.31764707f, 0.3372549f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.30980393f, 0.31764707f, 0.3372549f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.30980393f, 0.31764707f, 0.3372549f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.30980393f, 0.31764707f, 0.3372549f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.24705882f, 0.69803923f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.13333334f, 0.4117647f, 0.54901963f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.13333334f, 0.4117647f, 0.54901963f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.07450981f, 0.09019608f, 0.12941177f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667f, 0.101960786f, 0.14509805f, 0.9724f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13333334f, 0.25882354f, 0.42352942f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.24705882f, 0.69803923f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.24705882f, 0.69803923f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.13333334f, 0.4117647f, 0.54901963f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.039215688f, 0.05490196f, 0.078431375f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.039215688f, 0.05490196f, 0.078431375f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.06666667f, 0.10980392f, 0.16078432f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.24705882f, 0.69803923f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.9764706f, 0.25882354f, 0.25882354f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
}
