/*!
 *  @author     Dubsky Tomas
 */
#version 460
layout(location = 0) out vec4   o_color;

layout(set = 0, binding = 0, std140) restrict uniform SimulationUB {
    float u_time;
    float u_reflX;
};

const float PI = 3.14159265359;
const float AREA_WIDTH = 1000.;

const vec3 SOURCES[] = vec3[](
    vec3(500., 100., 0.25*PI)
    //vec3(500., 200., 0.0*PI)
);

float pulse(float phase){
    phase /= 10.0;
    float c = cos(phase);
    return (c >= 0.5) ? (-0.5 + c * 1.5f) : 0.0f;
}

void main() {
    vec2 p = gl_FragCoord.xy;

    vec3 color = vec3(0.5);
    for (int i = 0; i < SOURCES.length(); i++){
        //Direct wave
        vec3 s = SOURCES[i];
        float d = distance(p, s.xy);
        color.r += (0.5 * pulse(u_time * 2.0 - d + s.z));

        //Reflected wave
        if (p.x < u_reflX){
            float sX = (s.x - u_reflX);
            float pX = (p.x - u_reflX);
            float rY = (s.y * pX + p.y * sX) / (sX + pX);
            d = distance(s.xy, vec2(u_reflX, rY)) + distance(p, vec2(u_reflX, rY));
            color.g += (0.5 * pulse(u_time * 2.0 - d + s.z));
        }
    }

    o_color = vec4(color, 1.0);
}
