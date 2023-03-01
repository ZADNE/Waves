/*!
 *  @author    Dubsky Tomas
 */
#include <WAves/main/WorldRoom.hpp>

RE::Color convert(const glm::vec4& col) {
    return col * 255.0f;
}

void WorldRoom::renderGUI() {
    switch (m_guiType) {
    case GuiType::Interface:
        ImGui::SetNextWindowPos(m_guiPos, ImGuiCond_Always, {0.5f, 0.0f});
        if (ImGui::Begin("Interface", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (ImGui::BeginTable("##Table", 2)) {
                ImGui::TableNextRow(); ImGui::TableNextColumn();
                ImGui::Text("Refraction index");
                ImGui::TableNextColumn();
                ImGui::Text("Refraction index");
                ImGui::TableNextRow(); ImGui::TableNextColumn();
                ImGui::SliderFloat(
                    "##refractionIndexLeft",
                    &m_uniforms.refractionIndexLeft, 1.0f, 4.0f, "%.3f", ImGuiSliderFlags_Logarithmic
                );
                ImGui::TableNextColumn();
                ImGui::SliderFloat(
                    "##refractionIndexRight",
                    &m_uniforms.refractionIndexRight, 1.0f, 4.0f, "%.3f", ImGuiSliderFlags_Logarithmic
                );
                ImGui::EndTable();
            }
        }
        ImGui::End();
        break;
    case GuiType::NewSource:
        ImGui::SetNextWindowPos(m_guiPos, ImGuiCond_Always);
        if (ImGui::Begin("##Add new source", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
            if (ImGui::Button("Add new source")) {
                m_selectedSourceIndex = m_nextFreeSourceIndex;
                m_uniforms.sources[m_nextFreeSourceIndex++] = glm::vec4{
                    m_guiPos.x, m_windowDims.y - m_guiPos.y - 1.0f,
                    1.0f,
                    -m_simSecondsPassed
                };
                m_guiType = GuiType::None;
            }
        }
        ImGui::End();
        break;
    case GuiType::EditSource:
        ImGui::SetNextWindowPos(m_guiPos, ImGuiCond_Always);
        if (ImGui::Begin("##Add new source", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
            if (ImGui::Button("Show rays")) {
                m_guiType = GuiType::ShowRays;
            }
            if (ImGui::Button("Delete source")) {
                m_nextFreeSourceIndex--;
                if (m_selectedSourceIndex != m_nextFreeSourceIndex) {
                    std::swap(
                        m_uniforms.sources[m_selectedSourceIndex],
                        m_uniforms.sources[m_nextFreeSourceIndex]
                    );
                }
                m_uniforms.sources[m_nextFreeSourceIndex] = glm::vec4{0.0f};
                m_guiType = GuiType::None;
            }
        }
        ImGui::End();
        break;
    default:
        break;
    }

    //Main menu
    ImGui::SetNextWindowPos({0.0f, 0.0f}, ImGuiCond_Always);
    if (ImGui::Begin("##Main Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        if (ImGui::BeginTabBar("##TabBar")) {
            if (ImGui::BeginTabItem("Simulation")) {
                ImGui::SliderFloat("Simulation speed",
                    &m_simSpeed, 1.0f, 64.0f, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoRoundToFormat);
                ImGui::EndTabItem();
                if (ImGui::Button("Remove all sources")) {
                    m_uniforms.sources = {};
                    m_nextFreeSourceIndex = 0;
                    m_guiType = GuiType::None;
                }
            }
            if (ImGui::BeginTabItem("Visualization")) {
                ImGui::Text("FPS: %u", engine().getFramesPerSecond());
                ImGui::ColorEdit4("Interface", &m_uniforms.interfaceColor.x);
                ImGui::ColorEdit4("Direct", &m_uniforms.directColor.x);
                ImGui::ColorEdit4("Reflected", &m_uniforms.reflectedColor.x);
                ImGui::ColorEdit4("Refracted", &m_uniforms.refractedColor.x);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void WorldRoom::renderLines(const vk::CommandBuffer& commandBuffer, double interpolationFactor) {
    const auto& u = m_uniforms;
    std::array<RE::VertexPOCO, 8> vertices;
    uint32_t j = 0;
    m_gb.begin();

    //Rays
    if (m_guiType == GuiType::ShowRays) {
        glm::vec2 p = engine().getCursorAbs();
        const auto& s = u.sources[m_selectedSourceIndex];
        float sX = (s.x - u.interfaceX);
        float pX = (p.x - u.interfaceX);
        if (glm::sign(sX) == glm::sign(pX)) {
            //Direct ray
            vertices[j++] = {s, convert(u.directColor)};
            vertices[j++] = {p, convert(u.directColor)};
            //Reflected wave
            glm::vec2 i = {u.interfaceX, (s.y * pX + p.y * sX) / (sX + pX)};
            vertices[j++] = {s, convert(u.directColor)};
            vertices[j++] = {i, convert(u.directColor)};
            vertices[j++] = {i, convert(u.reflectedColor)};
            vertices[j++] = {p, convert(u.reflectedColor)};
        } else {
            //Refracted ray
            glm::vec2 n =
                (s.x < u.interfaceX) ?
                glm::vec2{u.refractionIndexLeft, u.refractionIndexRight} :
                glm::vec2{u.refractionIndexLeft, u.refractionIndexRight};
            glm::vec2 i = {u.interfaceX, (-s.y * pX * n.y + p.y * sX * n.x) / (sX * n.x - pX * n.y)};
            vertices[j++] = {s, convert(u.directColor)};
            vertices[j++] = {i, convert(u.directColor)};
            vertices[j++] = {i, convert(u.refractedColor)};
            vertices[j++] = {p, convert(u.refractedColor)};
        }
    }

    //Interface
    vertices[j++] = {glm::vec2{u.interfaceX, 0.0f}, convert(u.interfaceColor)};
    vertices[j++] = {glm::vec2{u.interfaceX, m_windowDims.y}, convert(u.interfaceColor)};
    m_gb.addVertices(0u, j, vertices.data());

    m_gb.end();
    m_gb.draw(commandBuffer, m_view.getViewMatrix());
}
