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
        .interfaceX = m_windowDims.x * 0.5f
    }),
    m_resolution(std::find(RESOLUTIONS.begin(), RESOLUTIONS.end(), engine().getWindowDims())) {
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
        m_guiType = GuiType::None;
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
            if (glm::distance(glm::vec2{s.x, s.y}, cursor) <= 20.0f) {
                m_guiType = GuiType::EditSource;
                m_selectedSourceIndex = i;
                m_guiPos = {s.x, window.y - s.y - 1.0f};
                onSource = true;
                break;
            }
        }
        if (!onSource) {
            if (onInterface) {
                m_guiType = GuiType::Interface;
                m_guiPos = {m_uniforms.interfaceX, window.y - cursor.y - 1.0f};
            } else if (m_nextFreeSourceIndex < 8) {
                m_guiType = GuiType::NewSource;
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

    renderLines(commandBuffer, interpolationFactor);
    renderGUI();
}