/*!
 *  @author     Dubsky Tomas
 */
#extension GL_EXT_scalar_block_layout : require
layout(set = 0, binding = 0, std430) restrict uniform SimulationUB {
    vec2    u_areaDims;
    int     u_zeroGray;
    int     u_padding;
    float   u_time;
    float   u_interfaceX;
    float   u_refractionIndexLeft;
    float   u_refractionIndexRight;
    vec4    u_interfaceColor;
    vec4    u_directColor;
    vec4    u_reflectedColor;
    vec4    u_refractedColor;
    vec4    u_sources[8];   //x,y = position, z = amplitude, w = phase shift
    int     u_sourceTypes[8];
};
