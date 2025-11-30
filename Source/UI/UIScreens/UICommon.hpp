#pragma once
#include <flecs.h>
#include <string>
#include <imgui.h>

namespace UICommon {
    // Função auxiliar para obter entidade por nome
    static flecs::entity GetEntityByName(const flecs::world& ecs, const std::string& name) {
        return ecs.lookup(name.c_str());
    }
    
    // Função auxiliar para botão horizontal
    static bool HorizontalButton(const char* label, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0)) {
        return ImGui::Button(label, size_arg);
    }
}