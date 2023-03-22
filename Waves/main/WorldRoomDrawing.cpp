/*!
 *  @author    Dubsky Tomas
 */
#include <Waves/main/WorldRoom.hpp>

RE::Color convert(const glm::vec4& col) {
    return col * 255.0f;
}

void WorldRoom::renderGUI() {
    auto addSource = [&](int type) {
        m_selectedSourceIndex = m_nextFreeSourceIndex;
        m_uniforms.sources[m_selectedSourceIndex] = glm::vec4{
            m_guiPos.x, m_windowDims.y - m_guiPos.y - 1.0f,
            1.0f,
            -m_simSecondsPassed
        };
        m_uniforms.sourceTypes[m_selectedSourceIndex] = type;
        m_nextFreeSourceIndex++;
        m_guiType = GuiType::None;
    };

    ImGui::PushFont(m_arial);
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
            if (ImGui::Button("Add harmonic source")) { addSource(1); }
            if (ImGui::Button("Add pulse source")) { addSource(2); }
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
                    std::swap(
                        m_uniforms.sourceTypes[m_selectedSourceIndex],
                        m_uniforms.sourceTypes[m_nextFreeSourceIndex]
                    );
                }
                m_uniforms.sourceTypes[m_nextFreeSourceIndex] = 0;
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
                    &m_simSpeed, 4.0f, 256.0f, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoRoundToFormat);
                ImGui::EndTabItem();
                if (ImGui::Button("Remove all sources")) {
                    m_uniforms.sources = {};
                    m_uniforms.sourceTypes = {};
                    m_nextFreeSourceIndex = 0;
                    m_guiType = GuiType::None;
                }
            }
            if (ImGui::BeginTabItem("Visualization")) {
                ImGui::Text("FPS: %u", engine().getFramesPerSecond());
                if (comboSelect(RESOLUTIONS, "Resolution", engine().getWindowDims().x * 0.125f, m_resolution, ivec2ToString)) {
                    m_windowDims = *m_resolution;
                    engine().setWindowDims(m_windowDims, true);
                    m_uniforms.areaDims = glm::vec4{m_windowDims, 0.0f, 0.0f};
                    m_uniforms.interfaceX = m_windowDims.x * 0.5f;
                    m_view = RE::View2D{m_windowDims};
                    m_view.shiftPosition(m_windowDims * 0.5f);
                }
                ImGui::Separator();
                ImGui::Text("Plane colors");
                ImGui::ColorEdit4("Interface", &m_uniforms.interfaceColor.x);
                ImGui::ColorEdit4("Direct", &m_uniforms.directColor.x);
                ImGui::ColorEdit4("Reflected", &m_uniforms.reflectedColor.x);
                ImGui::ColorEdit4("Refracted", &m_uniforms.refractedColor.x);
                if (ImGui::Button("Just amplitude")) {
                    m_uniforms.directColor = {1.0f, 1.0f, 1.0f, 1.0f};
                    m_uniforms.reflectedColor = {1.0f, 1.0f, 1.0f, 1.0f};
                    m_uniforms.refractedColor = {1.0f, 1.0f, 1.0f, 1.0f};
                }
                ImGui::SameLine();
                if (ImGui::Button("By origin")) {
                    m_uniforms.directColor = {1.0f, 0.0f, 0.0f, 1.0f};
                    m_uniforms.reflectedColor = {0.0f, 1.0f, 0.0f, 1.0f};
                    m_uniforms.refractedColor = {0.0f, 0.0f, 1.0f, 1.0f};
                }
                ImGui::Separator();
                if (ImGui::Button(m_uniforms.zeroGray ? "Make zero black" : "Make zero gray")) {
                    m_uniforms.zeroGray = 1 - m_uniforms.zeroGray;
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Exit")) {
                if (ImGui::Button("Exit")) {
                    engine().scheduleExit();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    ImGui::PopFont();
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
                glm::vec2{u.refractionIndexRight, u.refractionIndexLeft};
            glm::vec2 i = {u.interfaceX, (-s.y * pX * n.x + p.y * sX * n.y) / (sX * n.y - pX * n.x)};
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
