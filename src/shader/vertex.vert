#version 450

layout (location = 0) out vec3 fragColor; // output location for frag shader...frag shader will take input from here
// triangle vertex position
vec3 position[3] = vec3[](
    vec3(-0.5,-0.5,0.0),
    vec3(0.5,-0.5,0.0),
    vec3(0.0,0.5,0.0)
);
// triangle vertex colors
vec3 colors[3] = vec3[](
    vec3(0.0,0.15,1.0),
    vec3(0.1,0.85,0.25),
    vec3(1.0,0.5,0.0)
);

void main(){
    gl_Position = vec4(position[gl_VertexIndex],1.0);
    fragColor = colors[gl_VertexIndex];
}