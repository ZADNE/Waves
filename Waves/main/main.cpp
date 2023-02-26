/*! 
 *  @author    Dubsky Tomas
 */
#include <RealEngine/program/MainProgram.hpp>

#include <Waves/main/WorldRoom.hpp>

constexpr size_t WORLD_ROOM_NAME = 1;

int main(int argc, char* argv[]) {
    RE::MainProgram::initialize();

    RE::MainProgram::addRoom<WorldRoom>(WORLD_ROOM_NAME);

    return RE::MainProgram::run(WORLD_ROOM_NAME, {});
}