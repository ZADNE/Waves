/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>

#include <RealEngine/rooms/Room.hpp>
#include <RealEngine/rendering/pipelines/Pipeline.hpp>
#include <RealEngine/rendering/pipelines/PipelineLayout.hpp>
#include <RealEngine/rendering/descriptors/DescriptorSet.hpp>

 /**
  * @brief Holds all gameplay-related objects.
 */
class WorldRoom: public RE::Room {
public:

    WorldRoom(size_t name);

    void sessionStart(const RE::RoomTransitionArguments& args) override;
    void sessionEnd() override;
    void step() override;
    void render(const vk::CommandBuffer& commandBuffer, double interpolationFactor) override;

private:

    float stepsPassed = 0.0f;

    struct alignas(256) SimulationUB {
        float time;
        float reflX;
    };

    RE::PipelineLayout m_pipelineLayout;
    RE::Pipeline m_pipeline;
    RE::DescriptorSet m_descriptorSet;
    RE::Buffer m_simulationUB;
    SimulationUB* m_simulationUBMapped = m_simulationUB.map<SimulationUB>(0ull, VK_WHOLE_SIZE);
};