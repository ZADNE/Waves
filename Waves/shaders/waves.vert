/*!
 *  @author     Dubsky Tomas
 */
#version 460

const vec2 c_pos[3] = {{-1.0, -1.0}, {3.0, -1.0}, {-1.0, 3.0}};

void main() {
    gl_Position = vec4(c_pos[gl_VertexIndex], 0.0, 1.0);
}
