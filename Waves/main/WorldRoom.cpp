/*!
 *  @author    Dubsky Tomas
 */
#include <WAves/main/WorldRoom.hpp>

#include <Waves/shaders/AllShaders.hpp>

constexpr float STEPS_PER_SECOND = 64.0f;
constexpr float SECONDS_PER_STEP = 1.0f / STEPS_PER_SECOND;

using enum vk::BufferUsageFlagBits;
using enum vk::MemoryPropertyFlagBits;
using enum vk::ShaderStageFlagBits;

constexpr RE::RoomDisplaySettings INITIAL_SETTINGS{
    .stepsPerSecond = static_cast<unsigned int>(STEPS_PER_SECOND),
    .framesPerSecondLimit = 150u,
    .usingImGui = true
};


WorldRoom::WorldRoom(size_t name):
    Room(name, INITIAL_SETTINGS),
    m_view(m_windowDims),
    m_pipelineLayout(
        RE::PipelineLayoutCreateInfo{},
        RE::PipelineLayoutDescription{
            .bindings = {{vk::DescriptorSetLayoutBinding{
                0u,
                vk::DescriptorType::eUniformBufferDynamic,
                1u,
                eVertex | eFragment
            }}}
        }
    ),
    m_wavesPl(
        RE::PipelineGraphicsCreateInfo{
            .pipelineLayout = *m_pipelineLayout
        },
        RE::PipelineGraphicsSources{
            .vert = waves_vert,
            .frag = waves_frag
        }
    ),
    m_simulationUB(
        sizeof(SimulationUB)* RE::FRAMES_IN_FLIGHT,
        eUniformBuffer,
        eHostVisible | eHostCoherent
    ),
    m_uniforms(SimulationUB{
        .areaDims = glm::vec4{m_windowDims, 0.0f, 0.0f},
        .time = 0.0f,
        .interfaceX = m_windowDims.x * 0.5f,
        .refractionIndexLeft = 1.0f,
        .refractionIndexRight = 2.0f,
        .sources = {}
    }) {
    m_descriptorSet.write(vk::DescriptorType::eUniformBufferDynamic, 0u, 0u, m_simulationUB, 0ull, sizeof(SimulationUB));
    engine().setWindowTitle("Waves");
    m_view.shiftPosition(m_windowDims * 0.5f);
}

void WorldRoom::sessionStart(const RE::RoomTransitionArguments& args) {

}

void WorldRoom::sessionEnd() {

}

void WorldRoom::step() {
    m_simSecondsPassed += SECONDS_PER_STEP * m_simSpeed;
    glm::vec2 window = engine().getWindowDims();
    glm::vec2 cursor = engine().getCursorAbs();
    bool onInterface = std::abs(cursor.x - m_uniforms.interfaceX) <= 10.0f;
    if (engine().wasKeyPressed(RE::Key::LMB)) {
        m_guiType = GUIWindow::None;
        if (onInterface) {
            m_dragInterface = true;
        }
    }
    if (engine().wasKeyReleased(RE::Key::LMB)) {
        m_dragInterface = false;
    }
    if (engine().wasKeyPressed(RE::Key::RMB)) {
        bool onSource = false;
        for (int i = 0; i < m_nextFreeSourceIndex; i++) {
            const auto& s = m_uniforms.sources[i];
            if (glm::distance(glm::vec2{s.x, s.y}, cursor) <= 10.0f) {
                m_guiType = GUIWindow::EditSource;
                m_editSourceIndex = i;
                m_guiPos = {s.x, window.y - s.y - 1.0f};
                onSource = true;
                break;
            }
        }
        if (!onSource) {
            if (onInterface) {
                m_guiType = GUIWindow::Interface;
                m_guiPos = {m_uniforms.interfaceX, window.y - cursor.y - 1.0f};
            } else if (m_nextFreeSourceIndex < 8) {
                m_guiType = GUIWindow::NewSource;
                m_guiPos = {cursor.x, window.y - cursor.y - 1.0f};
            }
        }
    }
    if (m_dragInterface) {
        m_uniforms.interfaceX = cursor.x;
    }
}

void WorldRoom::render(const vk::CommandBuffer& commandBuffer, double interpolationFactor) {
    m_simulationUBMapped[RE::CURRENT_FRAME] = m_uniforms;
    m_simulationUBMapped[RE::CURRENT_FRAME].time =
        m_simSecondsPassed + static_cast<float>(interpolationFactor) * SECONDS_PER_STEP * m_simSpeed;

    uint32_t offset = RE::CURRENT_FRAME * sizeof(SimulationUB);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {offset});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_wavesPl);
    commandBuffer.draw(3u, 1u, 0u, 0u);

    m_gb.begin();
    RE::VertexPOCO vertices[2] = {
        { glm::vec2{m_uniforms.interfaceX, 0.0f}, RE::Color{22, 22, 22, 255} },
        { glm::vec2{m_uniforms.interfaceX, m_windowDims.y}, RE::Color{22, 22, 22, 255} }
    };
    m_gb.addVertices(0u, 2u, vertices);
    m_gb.end();
    m_gb.draw(commandBuffer, m_view.getViewMatrix());

    renderGUI();
}

void WorldRoom::renderGUI() {
    switch (m_guiType) {
    case GUIWindow::None: break;
    case GUIWindow::Interface:
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
    case GUIWindow::NewSource:
        ImGui::SetNextWindowPos(m_guiPos, ImGuiCond_Always);
        if (ImGui::Begin("##Add new source", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
            if (ImGui::Button("Add new source")) {
                m_editSourceIndex = m_nextFreeSourceIndex;
                m_uniforms.sources[m_nextFreeSourceIndex++] = glm::vec4{
                    m_guiPos.x, m_windowDims.y - m_guiPos.y - 1.0f,
                    1.0f,
                    -m_simSecondsPassed
                };
                m_guiType = GUIWindow::EditSource;
            }
        }
        ImGui::End();
        break;
    case GUIWindow::EditSource:
        ImGui::SetNextWindowPos(m_guiPos, ImGuiCond_Always);
        if (ImGui::Begin("##Add new source", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
            if (ImGui::Button("Show rays")) {

            }
            if (ImGui::Button("Delete source")) {
                m_nextFreeSourceIndex--;
                if (m_editSourceIndex != m_nextFreeSourceIndex) {
                    std::swap(
                        m_uniforms.sources[m_editSourceIndex],
                        m_uniforms.sources[m_nextFreeSourceIndex]
                    );
                }
                m_uniforms.sources[m_nextFreeSourceIndex] = glm::vec4{0.0f};
                m_guiType = GUIWindow::None;
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
                    m_guiType = GUIWindow::None;
                }
            }
            if (ImGui::BeginTabItem("Visualization")) {
                ImGui::Text("FPS: %u", engine().getFramesPerSecond());
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}
