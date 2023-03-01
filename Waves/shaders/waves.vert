/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <Waves/shaders/SimulationUB.glsl>

layout(location = 0) out vec2 o_pos;

const vec2 c_pos[3] = {{-1.0, -1.0}, {3.0, -1.0}, {-1.0, 3.0}};

void main() {
    gl_Position = vec4(c_pos[gl_VertexIndex], 0.0, 1.0);
    o_pos = (c_pos[gl_VertexIndex] + 1.0) * 0.5 * u_areaDims.xy;
}
