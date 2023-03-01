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
#include <RealEngine/rendering/batches/GeometryBatch.hpp>
#include <RealEngine/rendering/cameras/View2D.hpp>

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

    void renderGUI();

    glm::vec2 m_windowDims = engine().getWindowDims();
    float m_simSecondsPassed = 0.0f;
    float m_simSpeed = 1.0f;

    struct alignas(256) SimulationUB {
        glm::vec4 areaDims;
        float time;
        float interfaceX;
        float refractionIndexLeft;
        float refractionIndexRight;
        std::array<glm::vec4, 8> sources;
    };
    int m_nextFreeSourceIndex = 0;

    enum class GUIWindow {
        None,
        Interface,
        NewSource,
        EditSource
    };
    GUIWindow m_guiType = GUIWindow::None;
    glm::vec2 m_guiPos = glm::vec2(0.0f, 0.0f);
    bool m_dragInterface = false;
    int m_editSourceIndex = 0;
    RE::View2D m_view;

    RE::PipelineLayout m_pipelineLayout;
    RE::Pipeline m_wavesPl;
    RE::DescriptorSet m_descriptorSet{m_pipelineLayout, 0u};
    RE::Buffer m_simulationUB;
    SimulationUB m_uniforms;
    SimulationUB* m_simulationUBMapped = m_simulationUB.map<SimulationUB>(0ull, VK_WHOLE_SIZE);
    RE::GeometryBatch m_gb{vk::PrimitiveTopology::eLineList, 64u, 3.0f};
};
