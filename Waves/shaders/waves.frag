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

float wave(int type, float phase){
    switch (type){
    case 1:
        return sin(max(phase * 0.1, 0.0));
    case 2:
        float s = sin(max(phase * 0.05, 0.0));
        return max(s * 128.0 - 127.0, 0.0);
    }
    return 0.0;//Should not ever get here
}

float reflectance(vec2 n, vec2 si, float si_l){
    float sinI = si.y / si_l;
    if ((n.x > n.y) && (abs(sinI) >= (n.y / n.x))){
        return 1.0;
    } else {
        float I = asin(sinI);
        float T = asin(n.x / n.y * sinI);
        float cosI = cos(I);
        float cosT = cos(T);
        float rs = (n.x*cosI - n.y*cosT) / (n.x*cosI + n.y*cosT);
        //float rp = (n.x*cosT - n.y*cosI) / (n.x*cosT + n.y*cosI);
        return rs * rs;
    }
}

void main() {
    vec2 p = i_pos;

    vec3 color = vec3(0.0);

    for (int i = 0; i < u_sources.length(); i++){
        int type = u_sourceTypes[i];
        if (type == 0) continue;
        vec4 s = u_sources[i];
        float sX = (s.x - u_interfaceX);
        float pX = (p.x - u_interfaceX);
        vec2 n = orientedRefractionIndices(s);
        if (sign(sX) == sign(pX)){
            //Direct wave
            float d = distance(p, s.xy) * n.x;
            color += (s.z * wave(type, u_time - d + s.w)) * u_directColor.rgb * u_directColor.a;
            //Reflected wave
            vec2 i = vec2(u_interfaceX, (s.y * pX + p.y * sX) / (sX + pX));
            vec2 si = i - s.xy;
            float si_l = length(si);
            float r = reflectance(n, si, si_l);
            d = (si_l + distance(p, i)) * n.x;
            color += (s.z * wave(type, u_time - d + s.w)) * u_reflectedColor.rgb * u_reflectedColor.a * r;
        } else {
            //Refracted wave
            vec2 i = vec2(u_interfaceX, (-s.y * pX * n.x + p.y * sX * n.y) / (sX * n.y - pX * n.x));
            vec2 si = i - s.xy;
            float si_l = length(si);
            float r = reflectance(n, si, si_l);
            float d = si_l * n.x + distance(p, i) * n.y;
            color += (s.z * wave(type, u_time - d + s.w)) * u_refractedColor.rgb * u_refractedColor.a * (1.0 - r);
        }
    }
    if (u_zeroGray != 0){
        color = color * 0.5 + 0.5;
    }
    o_color = vec4(color, 1.0);
}
