/*!
 *  @author    Dubsky Tomas
 */
#pragma once
#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>

#include <RealEngine/graphics/batches/GeometryBatch.hpp>
#include <RealEngine/graphics/cameras/View2D.hpp>
#include <RealEngine/graphics/descriptors/DescriptorSet.hpp>
#include <RealEngine/graphics/pipelines/Pipeline.hpp>
#include <RealEngine/graphics/pipelines/PipelineLayout.hpp>
#include <RealEngine/rooms/Room.hpp>

#include <Waves/misc/combos.hpp>

/**
 * @brief Holds all gameplay-related objects.
 */
class WorldRoom: public re::Room {
public:
    WorldRoom(size_t name);

    void sessionStart(const re::RoomTransitionArguments& args) override;
    void sessionEnd() override;
    void step() override;
    void render(const re::CommandBuffer& commandBuffer, double interpolationFactor) override;

private:
    void renderGUI();
    void renderLines(const re::CommandBuffer& commandBuffer, double interpolationFactor);

    glm::vec2 m_windowDims       = engine().windowDims();
    float     m_simSecondsPassed = 0.0f;
    float     m_simSpeed         = 32.0f;

    struct alignas(256) SimulationUB {
        glm::vec2 areaDims;
        int       zeroGray         = 0;
        int       showPolarization = *POLARIZATIONS.begin();
        float     time             = 0.0f;
        float     interfaceX;
        float     refractionIndexLeft  = 1.0f;
        float     refractionIndexRight = 2.0f;
        glm::vec4 interfaceColor{0.0862f, 0.0862f, 0.0862f, 1.0f};
        glm::vec4 directColor{1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec4 reflectedColor{1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec4 refractedColor{1.0f, 1.0f, 1.0f, 1.0f};
        std::array<glm::vec4, 8> sources{};
        std::array<int, 8>       sourceTypes{};
    };
    int m_nextFreeSourceIndex = 0;

    enum class GuiType { None, Interface, NewSource, EditSource, ShowRays };
    GuiType   m_guiType             = GuiType::None;
    glm::vec2 m_guiPos              = glm::vec2(0.0f, 0.0f);
    bool      m_dragInterface       = false;
    int       m_selectedSourceIndex = 0;

    re::View2D         m_view;
    re::PipelineLayout m_pipelineLayout;
    re::Pipeline       m_wavesPl;
    re::DescriptorSet  m_descriptorSet{m_pipelineLayout.descriptorSetLayout(0)};
    re::BufferMapped<SimulationUB> m_simulationUB;
    SimulationUB                   m_uniforms;
    re::GeometryBatch m_gb{vk::PrimitiveTopology::eLineList, 64u, 5.0f};
    ImFont*           m_arial =
        ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/arial.ttf", 21.0f);
    decltype(RESOLUTIONS)::const_iterator m_resolution;
    decltype(POLARIZATIONS
    )::const_iterator m_showPolarization = POLARIZATIONS.begin();
};
