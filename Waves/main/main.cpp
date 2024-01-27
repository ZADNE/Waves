/*!
 *  @author    Dubsky Tomas
 */
#include <RealEngine/program/MainProgram.hpp>

#include <Waves/main/WorldRoom.hpp>

int main(int argc, char* argv[]) {
    vk::StructureChain chain{vk::PhysicalDeviceFeatures2{
        vk::PhysicalDeviceFeatures{}.setWideLines(true)}};
    re::VulkanInitInfo initInfo{.deviceCreateInfoChain = &chain.get<>()};
    re::MainProgram::initialize(initInfo);

    auto* room = re::MainProgram::addRoom<WorldRoom>(0);

    return re::MainProgram::run(room->name(), {});
}