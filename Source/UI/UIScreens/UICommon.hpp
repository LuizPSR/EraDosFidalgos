#pragma once
#include <flecs.h>
#include <string>
#include <imgui.h>  // Adicionar este include
#include <cfloat>   // Adicionar para FLT_MIN

namespace UICommon {
    // Função auxiliar para botão horizontal
    static bool HorizontalButton(const char* label, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0)) {
        return ImGui::Button(label, size_arg);
    }
}