/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <array>
#include <string>

#include <glm/vec2.hpp>

#include <ImGui/imgui.h>


inline constexpr std::array RESOLUTIONS = {
    glm::ivec2{1280, 1024},
    glm::ivec2{1360, 768},
    glm::ivec2{1366, 768},
    glm::ivec2{1440, 900},
    glm::ivec2{1600, 900},
    glm::ivec2{1680, 1050},
    glm::ivec2{1920, 1080},
    glm::ivec2{2560, 1080},
    glm::ivec2{2560, 1440},
    glm::ivec2{3440, 1440},
    glm::ivec2{3840, 2160}
};

inline std::string ivec2ToString(const glm::ivec2& vec) {
    return std::to_string(vec.x) + "x" + std::to_string(vec.y);
}

template<typename T, size_t N, typename ToStringConvertor>
bool comboSelect(const std::array<T, N>& combos, const char* label, float width, typename std::array<T, N>::const_iterator& selected, ToStringConvertor toString) {
    ImGui::TextUnformatted(label); ImGui::SameLine();
    ImGui::SetNextItemWidth(width);
    bool changedSelection = false;
    std::string hiddenlabel = std::string("##") + label;
    if (ImGui::BeginCombo(hiddenlabel.c_str(), selected != combos.end() ? toString(*selected).c_str() : "")) {
        for (auto it = combos.begin(); it != combos.end(); ++it) {
            if (ImGui::Selectable(toString(*it).c_str(), it == selected)) {
                selected = it;
                changedSelection = true;
            }
        }
        ImGui::EndCombo();
    }
    return changedSelection;
}
