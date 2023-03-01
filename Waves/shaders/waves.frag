/*!
 *  @author     Dubsky Tomas
 */
#version 460
#include <Waves/shaders/SimulationUB.glsl>

layout(location = 0) out vec4   o_color;

layout(location = 0) in vec2    i_pos;

const float PI = 3.14159265359;

//x = source plane refraction index
//y = the other plane refraction index
vec2 orientedRefractionIndices(vec4 s){
    if (s.x < u_interfaceX){
        return vec2(u_refractionIndexLeft, u_refractionIndexRight);
    } else {
        return vec2(u_refractionIndexRight, u_refractionIndexLeft);
    }
}

float pulse(float phase){
    phase *= 0.1;
    float s = (phase >= 0.0) ? sin(phase) : 0.0;
    return (s >= 0.5) ? (-0.5 + s * 1.5) : 0.0f;
}

void main() {
    vec2 p = i_pos;

    vec3 color = vec3(0.0);

    for (int i = 0; i < u_sources.length(); i++){
        vec4 s = u_sources[i];
        float sX = (s.x - u_interfaceX);
        float pX = (p.x - u_interfaceX);
        vec2 n = orientedRefractionIndices(s);
        if (sign(sX) == sign(pX)){
            //Direct wave
            float d = distance(p, s.xy) * n.x;
            vec2 i = vec2(u_interfaceX, (s.y * pX + p.y * sX) / (sX + pX));
            color.r += (s.z * pulse(u_time - d + s.w));
            //Reflected wave
            d = (distance(s.xy, i) + distance(p, i)) * n.x;
            color.g += (s.z * pulse(u_time - d + s.w));
        } else {
            //Refracted wave
            vec2 i = vec2(u_interfaceX, (-s.y * pX * n.y + p.y * sX * n.x) / (sX * n.x - pX * n.y));
            float d = distance(s.xy, i) * n.x + distance(p, i) * n.y;
            color.b += (s.z * pulse(u_time - d + s.w));
        }
    }
    color = color * 0.5 + 0.5;

    o_color = vec4(color, 1.0);
}
