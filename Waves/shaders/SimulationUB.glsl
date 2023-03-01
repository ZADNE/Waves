/*!
 *  @author     Dubsky Tomas
 */
layout(set = 0, binding = 0, std140) restrict uniform SimulationUB {
    vec4    u_areaDims;
    float   u_time;
    float   u_interfaceX;
    float   u_refractionIndexLeft;
    float   u_refractionIndexRight;
    vec4    u_sources[8];   //x,y = position, z = amplitude, w = phase shift
};
