/*!
 *  @author    Dubsky Tomas
 */
#include <WAves/main/WorldRoom.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <Waves/shaders/AllShaders.hpp>

#ifdef _DEBUG
constexpr unsigned int FPS_LIMIT = 300u;
#else
constexpr unsigned int FPS_LIMIT = RE::Synchronizer::DO_NOT_LIMIT_FRAMES_PER_SECOND;
#endif // _DEBUG

constexpr float STEPS_PER_SECOND = 64.0f;

using enum vk::BufferUsageFlagBits;
using enum vk::MemoryPropertyFlagBits;
using enum vk::ShaderStageFlagBits;

constexpr RE::RoomDisplaySettings INITIAL_SETTINGS{
    .stepsPerSecond = static_cast<unsigned int>(STEPS_PER_SECOND),
    .framesPerSecondLimit = FPS_LIMIT,
    .usingImGui = true
};


WorldRoom::WorldRoom(size_t name):
    Room(name, INITIAL_SETTINGS),
    m_pipelineLayout(
        RE::PipelineLayoutCreateInfo{},
        RE::PipelineLayoutDescription{
            .bindings = {{vk::DescriptorSetLayoutBinding{
                0u,
                vk::DescriptorType::eUniformBufferDynamic,
                1u,
                eFragment
            }}}
        }
    ),
    m_pipeline(
        RE::PipelineGraphicsCreateInfo{
            .pipelineLayout = *m_pipelineLayout
        },
        RE::PipelineGraphicsSources{
            .vert = waves_vert,
            .frag = waves_frag
        }
    ),
    m_descriptorSet(m_pipelineLayout, 0u),
    m_simulationUB(
        sizeof(SimulationUB) * RE::FRAMES_IN_FLIGHT,
        eUniformBuffer,
        eHostVisible | eHostCoherent
    ){
    m_descriptorSet.write(vk::DescriptorType::eUniformBufferDynamic, 0u, 0u, m_simulationUB, 0ull, sizeof(SimulationUB));
}

void WorldRoom::sessionStart(const RE::RoomTransitionArguments& args) {

}

void WorldRoom::sessionEnd() {

}

void WorldRoom::step() {
    stepsPassed++;
}

void WorldRoom::render(const vk::CommandBuffer& commandBuffer, double interpolationFactor) {
    constexpr static float SECONDS_PER_STEP = 1.0f / STEPS_PER_SECOND;
    m_simulationUBMapped[RE::CURRENT_FRAME].time = (stepsPassed + static_cast<float>(interpolationFactor)) * SECONDS_PER_STEP;
    m_simulationUBMapped[RE::CURRENT_FRAME].reflX = 1000.0f;

    uint32_t offset = RE::CURRENT_FRAME * sizeof(SimulationUB);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0u, *m_descriptorSet, {offset});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
    commandBuffer.draw(3u, 1u, 0u, 0u);
}
